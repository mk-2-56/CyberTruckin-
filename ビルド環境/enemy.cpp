//=============================================================================
//
// エネミー処理 [enemy.cpp]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#include "enemy.h"
#include "bg.h"
#include "player.h"
#include "fade.h"
#include "collision.h"
#include "enemyBullet.h"
#include "sound.h"
#include "effect.h"
#include "field.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX					(4)		// テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
#define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値

#define JUMP_COUNT_MAX				(50.0f)	// 最大ジャンプフレーム
#define JUMP_POWER					(240.0f)	// ジャンプ力
#define JUMP_INTERVAL				(50.0f)	// 
#define WALK_SPEED					(-5.0f)
#define COUNT_BOMB					(15.0f)
#define COUNT_SHOT					(200)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/enemyYellow.png",
	"data/TEXTURE/timerBomb.png",
	"data/TEXTURE/enemyBlue.png",
};

static BOOL				g_Load = FALSE;			// 初期化を行ったかのフラグ
static ENEMY_PATTERN	g_EnemyPatt[ENEMY_PATT_ALL] =
{
	{TRUE, 100.0f, 100.0f, 0, 30, {0, 1, 1, 1.0f, 0, 1, 1, 1.0f}},				// damage, w, h, texNo, hpMax, anime
	{FALSE, 136.5f, 136.0f, 1, 1, {0, 21, 1, 0.18f, 0, 1, 1, 1.0f}},
	{TRUE, 100.0f, 100.0f, 0, 30, {0, 1, 1, 1.0f, 0, 1, 1, 1.0f}},
	{TRUE, 120.0f, 120.0f, 2, 30, {0, 1, 1, 1.0f, 0, 1, 1, 1.0f}},
};

static ENEMY			g_Enemy[ENEMY_MAX] =
{
	{FALSE, XMFLOAT3(2700.0f, 1380.0f, 0.0f), ENEMY_PATT_SHOT},
	{FALSE, XMFLOAT3(3300.0f, 1510.0f, 0.0f), ENEMY_PATT_SHOT_SINGLE},
	{FALSE, XMFLOAT3(3420.0f, 1135.0f + SCREEN_HEIGHT, 0.0f), ENEMY_PATT_BOMB},
	{FALSE, XMFLOAT3(3100.0f, 1430.0f + SCREEN_HEIGHT, 0.0f), ENEMY_PATT_BOMB},
	{FALSE, XMFLOAT3(3650.0f, 1660.0f + SCREEN_HEIGHT, 0.0f), ENEMY_PATT_BOMB},
	{FALSE, XMFLOAT3(6100.0f, 1660.0f, 0.0f), ENEMY_PATT_BOMB},
	{FALSE, XMFLOAT3(6300.0f, 1660.0f, 0.0f), ENEMY_PATT_BOMB},
	{FALSE, XMFLOAT3(6550.0f, 1660.0f, 0.0f), ENEMY_PATT_BOMB},
	{FALSE, XMFLOAT3(7000.0f, 600.0f, 0.0f), ENEMY_PATT_SHOT_SINGLE},
	{FALSE, XMFLOAT3(7700.0f, 600.0f, 0.0f), ENEMY_PATT_SHOT_SINGLE},

};

static int g_jumpCnt;


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
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

	// エネミー構造体の初期化
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		g_Enemy[i].use = ENEMY_USE_FALSE;
		g_Enemy[i].jump = FALSE;
		g_Enemy[i].wall  = FALSE;
		g_Enemy[i].rot  = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].move = XMFLOAT3(WALK_SPEED, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].countAnim = 0.0f;
		g_Enemy[i].time = 0.0f;
		g_Enemy[i].w = g_EnemyPatt[g_Enemy[i].type].w;
		g_Enemy[i].h = g_EnemyPatt[g_Enemy[i].type].h;
		g_Enemy[i].jumpY = 0.0f;
		g_Enemy[i].texNo = g_EnemyPatt[g_Enemy[i].type].texNo;
		g_Enemy[i].patternAnim = 0;
		g_Enemy[i].hp = g_EnemyPatt[g_Enemy[i].type].hpMax;
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
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

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateEnemy(void)
{
	FIELD* field = GetField();
	CHECKPT* checkPt = GetCheckPt();

	int enemyNum = 0;
	for (int i = 0; i < field->checkPtCnt; i++)
	{	// 現在のチェックポイントまでに登場の敵
		enemyNum += checkPt[i].enemyMax;
	}

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_Enemy[i].use == ENEMY_USE_TRUE)	// このエネミーが使われている？
		{										// Yes
			// 地形との当たり判定用に座標のバックアップを取っておく
			XMFLOAT3 pos_old = g_Enemy[i].pos;

			// アニメーション  
			g_Enemy[i].countAnim += g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].speed;
			if (g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].loop)
			{	// ループするアニメーションなら
				if (g_Enemy[i].countAnim > g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].x)
				{
					g_Enemy[i].countAnim = 0.0f;
				}
			}
			else
			{	// ループしないアニメーションなら
				if (g_Enemy[i].countAnim > (g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].x - 1.0f))
				{
					// 最後の絵にしておく
					g_Enemy[i].countAnim = (float)(g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].x) - 1.0f;

				}
			}


			// 敵のタイプに応じて行動パターンが違う
			switch (g_Enemy[i].type)
			{
			case ENEMY_PATT_SHOT:
				enemyShot(g_Enemy[i].pos, g_Enemy[i].time, BULLET_PATT_SPREAD);
				break;

			case ENEMY_PATT_BOMB:

				if(g_Enemy[i].time == COUNT_BOMB) PlaySound(SOUND_LABEL_SE_bombTimer);

				// アニメーションが終わったら
				if (g_Enemy[i].countAnim == (float)(g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].x) - 1.0f)
				{
					SetEffect(g_Enemy[i].pos, 0, EFFECT_EXPLOSION);				// 爆発エフェクトをつける
					PlaySound(SOUND_LABEL_SE_bomb001);
					g_Enemy[i].use = ENEMY_USE_DONE;
				}
				break;

			case ENEMY_PATT_BACKANDFORTH:
				g_Enemy[i].pos.x += g_Enemy[i].move.x;
				enemyShot(g_Enemy[i].pos, g_Enemy[i].time, BULLET_PATT_SINGLE);

				// 壁に当たったら向きを変える
				if ((GetMap(g_Enemy[i].pos.x, g_Enemy[i].pos.y) > 0) || (g_Enemy[i].pos.x > checkPt[field->checkPtCnt].x) || (g_Enemy[i].pos.x < checkPt[field->checkPtCnt].prevX))
				{
					g_Enemy[i].move.x = 0 - g_Enemy[i].move.x;
				}
				break;

			case ENEMY_PATT_SHOT_SINGLE:
				enemyShot(g_Enemy[i].pos, g_Enemy[i].time, BULLET_PATT_SINGLE);
				break;
			}

			// 移動が終わったらプレイヤーとの当たり判定
			if(g_EnemyPatt[g_Enemy[i].type].damage)
			{
				PLAYER* player = GetPlayer();
				for (int j = 0; j < PLAYER_MAX; j++)
				{
					// 生きてるエネミーと当たり判定をする
					if (player[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Enemy[i].pos, g_Enemy[i].w, g_Enemy[i].h,
							player[j].pos, player[j].w, player[j].h);

						// 当たっている？
						if (ans == TRUE)
						{
							// 当たった時の処理
							DamagePlayer(PLAYER_DAMAGE_MIN);
						}
					}
				}
			}

			if (i < enemyNum)
			{
				// 通過したチェックポイントの敵はもうFALSEにしておく
				g_Enemy[i].use = FALSE;
			}

			g_Enemy[i].time += 1.0f;
		}
	}

	PLAYER* player = GetPlayer();

	if ((GetMap(player->pos.x, player->pos.y) >= 0) && (GetMap(player->pos.x, player->pos.y) <= (checkPt[field->checkPtCnt].enemyMax - 1)))
	{
		int enemyNum = 0;

		for (int i = 0; i < field->checkPtCnt; i++)
		{
			enemyNum += checkPt[i].enemyMax;
		}
		// 対応するNo.の敵を出現させる
		SetEnemy(enemyNum + GetMap(player->pos.x, player->pos.y));
	}
	


#ifdef _DEBUG	// デバッグ情報を表示する


#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
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

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_Enemy[i].use == ENEMY_USE_TRUE)			// このエネミーが使われている？
		{										// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Enemy[i].texNo]);

			//エネミーの位置やテクスチャー座標を反映
			float ex = g_Enemy[i].pos.x - bg->pos.x + OUTSIDE_WIDTH;	// エネミーの表示位置X
			float ey = g_Enemy[i].pos.y - bg->pos.y;	// エネミーの表示位置Y
			float ew = g_Enemy[i].w;		// エネミーの表示幅
			float eh = g_Enemy[i].h;		// エネミーの表示高さ

			// アニメーション用
			float tw = 1.0f / g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].x;	// テクスチャの幅
			float th = 1.0f / g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].y;	// テクスチャの高さ
			float tx = (float)((int)g_Enemy[i].countAnim % g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].x) * tw;	// テクスチャの左上X座標
			float ty = 0;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, ex, ey, ew, eh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Enemy[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

		}
	}

}


//=============================================================================
// Enemy構造体の先頭アドレスを取得
//=============================================================================
ENEMY* GetEnemy(void)
{
	return &g_Enemy[0];
}

ENEMY_PATTERN* GetEnemyPatt(void)
{
	return &g_EnemyPatt[0];
}


//=============================================================================
// Enemyに対するアクション
//=============================================================================
// 敵を出現させる
void SetEnemy(int label)
{
	if (g_Enemy[label].use != ENEMY_USE_DONE)
	{
		g_Enemy[label].use = ENEMY_USE_TRUE;
		g_Enemy[label].hp = g_EnemyPatt[g_Enemy[label].type].hpMax;		// HP満タンにし直す
	}
}

void DamageEnemy(int num, int damage)			// どのタイプのどの敵に何ダメージ与えるか
{
	if (!g_EnemyPatt[g_Enemy[num].type].damage)
	{
		return;
	}

	g_Enemy[num].hp -= damage;
	SetEffect(g_Enemy[num].pos, 0, EFFECT_ENEMYHIT);
	PlaySound(SOUND_LABEL_SE_attack00);			// 当たった時の音

	if (g_Enemy[num].hp <= 0)
	{
		// HPがなくなったら、復活する敵以外は使用済みに
		g_Enemy[num].use = (g_Enemy[num].loop) ? ENEMY_USE_REUSE : ENEMY_USE_DONE;
		SetEffect(g_Enemy[num].pos, 0, EFFECT_ENEMYDEAD);								// 倒したときのエフェクト
	}
}


//=============================================================================
// Enemyの行動パターン
//=============================================================================
// 一定間隔でショットを打つタイプ
void enemyShot(XMFLOAT3 pos, float time, int type)
{
	if((int)(time) % COUNT_SHOT == 0) SetEnemyBullet(pos, type);
}