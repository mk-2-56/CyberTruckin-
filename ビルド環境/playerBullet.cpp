//=============================================================================
//
// プレイヤーからのバレット処理 [playerBullet.cpp]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#include "enemy.h"
#include "collision.h"
#include "playerBullet.h"
#include "bg.h"
#include "effect.h"
#include "player.h"
#include "field.h"
#include "sound.h"
#include "status.h"
#include "boss.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(25)	// バレットサイズ
#define TEXTURE_HEIGHT				(25)	// 
#define TEXTURE_MAX					(1)		// テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
#define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値
#define	RIGHT_ANGLE					(3)		// 跳ね返りの角度
#define	LEFT_ANGLE					(5)
#define	ENEMY_DAMAGE				(10)


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[] = {
	"data/TEXTURE/bullet01.png",
};

static BOOL		g_Load = FALSE;			// 初期化を行ったかのフラグ
static float	g_Speed = 16.0f;
static PLAYER_BULLET	g_Bullet[BULLET_PLAYER_MAX];

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayerBullet(void)
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
	for (int i = 0; i < BULLET_PLAYER_MAX; i++)
	{
		g_Bullet[i].use = FALSE;
		g_Bullet[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Bullet[i].w = TEXTURE_WIDTH;
		g_Bullet[i].h = TEXTURE_HEIGHT;
		g_Bullet[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Bullet[i].move = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Bullet[i].texNo = 0;
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayerBullet(void)
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
void UpdatePlayerBullet(void)
{
	int bulletCount = 0;				// 処理したバレットの数
	FIELD* field = GetField();
	CHECKPT* checkpt = GetCheckPt();

	for (int i = 0; i < BULLET_PLAYER_MAX; i++)
	{
		if (g_Bullet[i].use == TRUE)	// このバレットが使われている？
		{								// Yes

			// バレットの移動処理
			XMVECTOR pos = XMLoadFloat3(&g_Bullet[i].pos);
			XMVECTOR move = XMLoadFloat3(&g_Bullet[i].move);
			pos += move;
			XMStoreFloat3(&g_Bullet[i].pos, pos);

			if (GetMap(g_Bullet[i].pos.x, g_Bullet[i].pos.y) > MAPCHIP_NOCOLLISION)
			{	// 壁に当たるとエフェクト付きで消える
				g_Bullet[i].use = false;
				SetEffect(g_Bullet[i].pos, 0, EFFECT_HIT_WALL);
			}
			else if ((g_Bullet[i].pos.x > checkpt[field->checkPtCnt].x) || (g_Bullet[i].pos.x < checkpt[field->checkPtCnt].prevX))
			{	// チェックポイントを超えるとこっそり消える
				g_Bullet[i].use = false;
			}

			// 当たり判定処理
			{
				ENEMY* enemy = GetEnemy();
				ENEMY_PATTERN* enemyPatt = GetEnemyPatt();
				BOSS* boss = GetBoss();

				// エネミーの数分当たり判定を行う
				for (int j = 0; j < ENEMY_MAX; j++)
				{
					// 生きてるエネミーと当たり判定をする
					if (enemy[j].use == ENEMY_USE_TRUE)
					{
						BOOL ans = CollisionBB(g_Bullet[i].pos, g_Bullet[i].w, g_Bullet[i].h,
							enemy[j].pos, enemy[j].w, enemy[j].h);

						// 当たっている？
						if ((ans == TRUE) && (enemyPatt[enemy[j].type].damage))		// ダメージを与えてくるタイプの敵だったら
						{
							// 当たった時の処理
							DamageEnemy(j, ENEMY_DAMAGE);
							g_Bullet[i].use = FALSE;
							AddScore(100);
						}
					}
				}

				// ボスの数分当たり判定を行う
				for (int k = 0; k < BOSS_MAX; k++)
				{
					// 生きてるボスと当たり判定をする
					if (boss[k].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Bullet[i].pos, g_Bullet[i].w, g_Bullet[i].h,
							boss[k].pos, boss[k].w, boss[k].h);

						// 当たっている？
						if (ans == TRUE)
						{
							if (boss[k].damage == TRUE)
							{
								// 当たった時の処理
								DamageBoss(PLAYER_DAMAGE_MIN);
								SetEffect(g_Bullet[i].pos, 0, EFFECT_HIT_WALL);
								g_Bullet[i].use = FALSE;
							}
							else if(g_Bullet[i].pos.x > boss[k].pos.x)
							{
								g_Bullet[i].move.x = sinf(XM_PIDIV4 * RIGHT_ANGLE) * g_Speed;	// 右にはねかえる
								g_Bullet[i].move.y = cosf(XM_PIDIV4 * RIGHT_ANGLE) * g_Speed;
							}
							else
							{
								g_Bullet[i].move.x = sinf(XM_PIDIV4 * LEFT_ANGLE) * g_Speed;	// 左にはねかえる
								g_Bullet[i].move.y = cosf(XM_PIDIV4 * LEFT_ANGLE) * g_Speed;
							}
						}
					}
				}

			}
			bulletCount++;
		}

		
	}


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayerBullet(void)
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

	for (int i = 0; i < BULLET_PLAYER_MAX; i++)
	{
		if (g_Bullet[i].use == TRUE)		// このバレットが使われている？
		{									// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Bullet[i].texNo]);

			//バレットの位置やテクスチャー座標を反映
			float px = g_Bullet[i].pos.x - bg->pos.x + OUTSIDE_WIDTH;	// バレットの表示位置X
			float py = g_Bullet[i].pos.y - bg->pos.y;	// バレットの表示位置Y
			float pw = g_Bullet[i].w;		// バレットの表示幅
			float ph = g_Bullet[i].h;		// バレットの表示高さ

			float tw = 1.0f;	// テクスチャの幅
			float th = 1.0f;	// テクスチャの高さ
			float tx = 0.0f;	// テクスチャの左上X座標
			float ty = 0.0f;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, 
				px, py, pw, ph, 
				tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Bullet[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

}


//=============================================================================
// バレット構造体の先頭アドレスを取得
//=============================================================================
PLAYER_BULLET *GetPlayerBullet(void)
{
	return &g_Bullet[0];
}


//=============================================================================
// バレットの発射設定
//=============================================================================
void SetPlayerBullet(XMFLOAT3 pos)
{
	// もし未使用の弾が無かったら発射しない( =これ以上撃てないって事 )
	for (int i = 0; i < BULLET_PLAYER_MAX; i++)
	{
		if (g_Bullet[i].use == FALSE)							// 未使用状態のバレットを見つける
		{
			g_Bullet[i].use = TRUE;								// 使用状態へ変更する
			PlaySound(SOUND_LABEL_SE_shot00);
			g_Bullet[i].move.y = 0;

			PLAYER* player = GetPlayer();
			if (player[0].left)
			{
				g_Bullet[i].move.x = 0.0f - g_Speed;
				g_Bullet[i].pos = XMFLOAT3(pos.x - player[0].w / 2, pos.y - 15, 0.0f);
				SetEffect(g_Bullet[i].pos, 0, EFFECT_PLAYERATTACK);
			}
			else
			{
				g_Bullet[i].move.x = g_Speed;
				g_Bullet[i].pos = XMFLOAT3(pos.x + player[0].w / 2, pos.y - 15, 0.0f);
				SetEffect(g_Bullet[i].pos, 0, EFFECT_PLAYERATTACK);
			}
			return;												// 1発セットしたので終了する
		}

	}
}

