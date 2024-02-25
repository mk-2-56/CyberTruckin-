//=============================================================================
//
// 敵からのバレット処理 [enemyBullet.cpp]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#include "enemyBullet.h"
#include "enemy.h"
#include "collision.h"
#include "status.h"
#include "bg.h"
#include "effect.h"
#include "player.h"
#include "field.h"
#include "sound.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX					(3)		// テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
#define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値
#define BULLET_HEIGHT_INTERVAL		(60)	// バレットの高さ間隔
#define BOSS_CLIMAX_NUM				(4)	// ボスが最終攻撃で出す弾数


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[] = {
	"data/TEXTURE/enemyBullet.png",
	"data/TEXTURE/bossBullet.png",
	"data/TEXTURE/bossBullet2.png",
};

static BOOL		g_Load = FALSE;			// 初期化を行ったかのフラグ
static BULLET_PATTERN	g_BulletPatt[BULLET_PATT_ALL] = {
	{35.0f, 35.0f, 10.0f, 4.0f, 8, 0, 1, 1},						// w, h, damage, speed, max, texNo, x, y
	{100.0f, 100.0f, 10.0f, 12.0f, 2, 1, 5, 2},
	{68.0f, 38.0f, 10.0f, 6.5f, 10, 2, 4, 1},
	{40.0f, 40.0f, 10.0f, 5.0f, 1, 0, 1, 1},
};
static BULLET g_Bullet[BULLET_MAX];
static float g_leftEnd;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemyBullet(void)
{
	ID3D11Device *pDevice = GetDevice();

	//テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// バレット構造体の初期化
	for (int i = 0; i < BULLET_MAX; i++)
	{
		g_Bullet[i].use = FALSE;
		g_Bullet[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Bullet[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Bullet[i].move = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Bullet[i].w = 0.0f;
		g_Bullet[i].h = 0.0f;
		g_Bullet[i].damage = 0.0f;
		g_Bullet[i].speed = 0.0f;
		g_Bullet[i].texNo = 0;
		g_Bullet[i].cntAnime = 0;
		g_Bullet[i].type = 0;
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemyBullet(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateEnemyBullet(void)
{
	int bulletCount = 0;				// 処理したバレットの数
	FIELD* field = GetField();
	CHECKPT* checkpt = GetCheckPt();
	g_leftEnd = checkpt[field->checkPtCnt].prevX;	// ボスの行動座標の左端をセット

	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Bullet[i].use == TRUE)	// このバレットが使われてる？
		{								// Yes

			// バレットの移動処理
			XMVECTOR pos = XMLoadFloat3(&g_Bullet[i].pos);
			XMVECTOR move = XMLoadFloat3(&g_Bullet[i].move);
			pos += move;
			XMStoreFloat3(&g_Bullet[i].pos, pos);

			// アニメーション
			g_Bullet[i].cntAnime++;
			if (g_Bullet[i].cntAnime > g_BulletPatt[g_Bullet[i].type].x * g_BulletPatt[g_Bullet[i].type].y)
			{
				g_Bullet[i].cntAnime = 0;								// ループさせる
			}


			// 当たり判定処理
			{
				PLAYER* player = GetPlayer();

				// 生きてるプレイヤーと当たり判定をする
				if (player[0].use == TRUE)
				{
					XMFLOAT3 playerPos = player[0].pos;
					if (player[i].sliding)
					{
						playerPos = XMFLOAT3(player[0].pos.x, player[0].pos.y + LOSE_COLLISIONH_POS, 0.0f);
					}

					BOOL ans = CollisionBB(g_Bullet[i].pos, g_Bullet[i].w, g_Bullet[i].h,
						player[0].pos, player[0].collisionW, player[0].collisionH);
					// 当たっている？
					if (ans == TRUE)
					{
						// 当たった時の処理
						DamagePlayer(g_Bullet[i].damage);
						g_Bullet[i].use = FALSE;
					}
				}

				if ((GetMap(g_Bullet[i].pos.x, g_Bullet[i].pos.y) > MAPCHIP_NOCOLLISION) ||
					((g_Bullet[i].pos.x > checkpt[field->checkPtCnt].x) || (g_Bullet[i].pos.x < checkpt[field->checkPtCnt].prevX)) || 
					((g_Bullet[i].pos.y > checkpt[field->checkPtCnt].y) || (g_Bullet[i].pos.y < checkpt[field->checkPtCnt].prevY)))
				{	// 壁に当たるもしくはチェックポイントを超えると消える
					g_Bullet[i].use = FALSE;
				}
			}
			bulletCount++;
		}
		else
		{
			g_Bullet[i].move.x = 0;
			g_Bullet[i].move.y = 0;
		}
		
	}


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemyBullet(void)
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	BG* bg = GetBG();


	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Bullet[i].use == TRUE)	// このバレットが使われてる？
		{								// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Bullet[i].texNo]);

			//バレットの位置やテクスチャー座標を反映
			float px = g_Bullet[i].pos.x - bg->pos.x + OUTSIDE_WIDTH;	// バレットの表示位置X
			float py = g_Bullet[i].pos.y - bg->pos.y;	// バレットの表示位置Y
			float pw = g_Bullet[i].w;		// バレットの表示幅
			float ph = g_Bullet[i].h;		// バレットの表示高さ

			float tw = 1.0f / g_BulletPatt[g_Bullet[i].type].x;	// テクスチャの幅
			float th = 1.0f / g_BulletPatt[g_Bullet[i].type].y;	// テクスチャの高さ
			float tx = (float)(g_Bullet[i].cntAnime % g_BulletPatt[g_Bullet[i].type].x) * tw;	// テクスチャの左上X座標
			float ty = (float)(g_Bullet[i].cntAnime / g_BulletPatt[g_Bullet[i].type].x) * th;	// テクスチャの左上Y座標


			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer,
				px, py, pw, ph,
				tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Bullet[i].rot.z, g_Bullet[i].left);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}
}


//=============================================================================
// バレットの発射設定
//=============================================================================
void SetEnemyBullet(XMFLOAT3 pos, int type)
{
	int counter = 0;
	int num1 = 0;
	int num2 = 0;
	int fin = rand() % 2;					// 今回出す弾が一度に2発か1発か、2分の1の確率
	int currentPos;
	PLAYER* player = GetPlayer();

	// もし未使用の弾が無かったら発射しない( =これ以上撃てないって事 )
	for (int i = 0; i < g_BulletPatt[type].max; i++)
	{
		if (g_Bullet[i].use == FALSE)		// 未使用状態のバレットを見つける
		{
			g_Bullet[i].use = TRUE;			// 使用状態へ変更する
			g_Bullet[i].pos = pos;			// 座標をセット

			// バレットのタイプごとの属性をセット
			g_Bullet[i].w = g_BulletPatt[type].w;
			g_Bullet[i].h = g_BulletPatt[type].h;
			g_Bullet[i].damage = g_BulletPatt[type].damage;
			g_Bullet[i].speed = g_BulletPatt[type].speed;
			g_Bullet[i].texNo = g_BulletPatt[type].texNo;
			g_Bullet[i].type = type;

			switch (type)
			{ // タイプごとに動きを変える
			case BULLET_PATT_SPREAD:

				g_Bullet[i].move.x = sinf(XM_PI / 4 * i) * g_Bullet[i].speed;	// 方向をセット
				g_Bullet[i].move.y = cosf(XM_PI / 4 * i) * g_Bullet[i].speed;	// 方向をセット
				break;

			case BULLET_PATT_HORIZONTAL:
				g_Bullet[i].move.x = i == 0 ? g_Bullet[i].speed : g_Bullet[i].speed * -1;
				break;

			case BULLET_PATT_BOSSCLIMAX:
				currentPos = i % BOSS_CLIMAX_NUM;							// 今回出す位置

				if (pos.x == g_leftEnd)
				{
					g_Bullet[i].move.x = g_Bullet[i].speed;
					g_Bullet[i].left = FALSE;
				}
				else
				{
					g_Bullet[i].move.x = g_Bullet[i].speed * -1;				// 逆向き
					g_Bullet[i].left = TRUE;
				}
				g_Bullet[i].pos.y = pos.y + (BULLET_HEIGHT_INTERVAL * currentPos);

				if (fin == 0) return;

				if (counter == 0)
				{
					num1 = currentPos;

					// もし最上段の弾だったら
					if (num1 == 0)
					{
						// 次の弾は絶対に次の段にする(そうでないと避けられない)
						g_Bullet[i + 1].pos.y = pos.y + (BULLET_HEIGHT_INTERVAL * (currentPos + 1));
						return;
					}
					counter++;
				}
				else
				{	// 2発目に関して
					num2 = currentPos;
					
					// もし最上段の弾だったら
					if (num2 == 0)
					{	// 1発目は次の段でないといけないのでそれ以外はFALSEにする
						if (num1 != num2 + 1) g_Bullet[i].use = FALSE;
					}
					return;
				}
				break;

			case BULLET_PATT_SINGLE:
				BOOL left = pos.x > player[0].pos.x;
				g_Bullet[i].move.x = (left) ? g_Bullet[i].speed * -1 : g_Bullet[i].speed;
				break;
			}
		}
	}
}

