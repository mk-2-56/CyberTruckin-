//=============================================================================
//
// プレイヤー処理 [player.cpp]
// Author : GP11A132 15 高橋ほの香 
//
//=============================================================================
#include "player.h"
#include "input.h"
#include "bg.h"
#include "playerBullet.h"
#include "enemy.h"
#include "collision.h"
#include "status.h"
#include "field.h"
#include "sound.h"
#include "effect.h"
#include "fade.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(99)	// キャラサイズ
#define TEXTURE_HEIGHT				(121)	// 
#define TEXTURE_MAX					(11)	// テクスチャの数

// #define TEXTURE_PATTERN_DIVIDE_X	(3)		// アニメパターンのテクスチャ内分割数（X)
// #define TEXTURE_PATTERN_DIVIDE_Y	(1)		// アニメパターンのテクスチャ内分割数（Y)
// #define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
// #define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値

// プレイヤーの画面内配置座標
#define PLAYER_DISP_X				(GAMESCREEN_WIDTH/2)
#define PLAYER_DISP_Y				(SCREEN_HEIGHT/2 + TEXTURE_HEIGHT)

#define	PLAYER_DUSH_SPEED			(6.0f)	// 移動スピード
#define	PLAYER_SLIDING_SPEED		(0.8f)	// スライディングスピード
#define	PLAYER_SLIDING_MAX			(3.0f)	// スライディングMAX

// ジャンプ処理
#define	PLAYER_JUMP_CNT_FIRST		(20.0f)	// 上昇するフレーム数の初期値
#define	PLAYER_JUMP_POWER_FIRST		(10.0f)	// ジャンプ力の初期値
#define	PLAYER_JUMP_POWER_MAX		(23.0f)	// ジャンプ力の最大値

#define PLAYER_ATTACK_ACTION		(20.0f) // 攻撃モーションの時間
#define PLAYER_SLIDING_ACTION		(15.0f) // スライディングモーションの時間
#define	PLAYER_GRAVITY				(2.0f)		// 重力
#define	PLAYER_GRAVITY_MAX			(10.0f)		// 最大降下力

#define	PLAYER_COLLISION_DAMAGE		(10.0f)	// 敵とぶつかったときのダメージ
#define	PLAYER_DAMAGE_INTERVAL		(52.0f)	// 無敵時間(一度ダメージをくらってから次にくらうまでの間隔)のフレーム数
#define LOSE_COLLISIONH				(0.7f)	// スライディングで減らす判定


//*****************************************************************************
// 
// プロトタイプ宣言
//*****************************************************************************
// void DrawPlayerOffset(int no);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/normalPlayer.png",
	"data/TEXTURE/jumpPlayer.png",
	"data/TEXTURE/walkPlayer.png",
	"data/TEXTURE/hashigoPlayer.png",
	"data/TEXTURE/attackPlayer.png",
	"data/TEXTURE/attackRunPlayer.png",
	"data/TEXTURE/damagePlayer.png",
	"data/TEXTURE/downPlayer.png",
	"data/TEXTURE/crisisPlayer.png",
	"data/TEXTURE/slidingPlayer.png",
};


static BOOL		g_Load = FALSE;							// 初期化を行ったかのフラグ
static PLAYER	g_Player[PLAYER_MAX];					// プレイヤー構造体

static float    g_jumpCntMax = PLAYER_JUMP_CNT_FIRST;	// ジャンプカウント
static float    g_jumpPower = PLAYER_JUMP_POWER_FIRST;	// ジャンプ力
static float	g_playerCnt[PLAYER_COUNT_ALL];

static PLAYERANIM	g_PlayerAnim[ANIME_ALL] = {
	{0,0.0f,0,1},							// loop, cntSpeed, texNo, xMax
	{0,0.05f,1,2},
	{1,0.1f,2,4},
	{1,0.1f,3,4},
	{0,0.2f,4,2},
	{1,0.1f,5,4},
	{0,0.0f,6,1},
	{0,0.0f,7,1},
	{1,0.05f,8,2},
	{0,0.0f,9,1},
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer(void)
{
	ID3D11Device* pDevice = GetDevice();

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


	// プレイヤー構造体の初期化
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		g_Player[i].use = TRUE;
		g_Player[i].movable = TRUE;

		FIELD* field = GetField();
		// 最後に死んだ場所によって位置を変える
		switch (field->checkPtCnt)
		{
		case 0:				// 1つめ
			g_Player[i].pos = XMFLOAT3(150.0f, 1100.0f, 0.0f);
			break;

		case 1:				// 2つめ
			g_Player[i].pos = XMFLOAT3(3780.0f, 1500.0f, 0.0f);
			break;

		case 2:				// 3つめ
			g_Player[i].pos = XMFLOAT3(4110.0f, 2540.0f, 0.0f);
			break;

		case 3:				// 4つめ
			g_Player[i].pos = XMFLOAT3(7002.0f, 1656.0f, 0.0f);
			break;

		case 4:				// 5つめ
			g_Player[i].pos = XMFLOAT3(5975.0f, 756.0f, 0.0f);
			break;

		case 5:				// 5つめ
			g_Player[i].pos = XMFLOAT3(8370.0f, 680.0f, 0.0f);
			break;

		case 6:				// 6つめ
			g_Player[i].pos = XMFLOAT3(9500.0f, 680.0f, 0.0f);
			break;
		}

		g_Player[i].energy = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Player[i].pos_old = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Player[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Player[i].w = TEXTURE_WIDTH;
		g_Player[i].h = TEXTURE_HEIGHT;
		g_Player[i].collisionW = g_Player[i].w;
		g_Player[i].collisionH = g_Player[i].h;
		g_Player[i].hp = PLAYER_HP_MAX;
		g_Player[i].opacity = 1.0f;
		g_Player[i].countAnim = 0;

		g_Player[i].left = FALSE;							// スタート時は右向き
		// g_Player[i].dir = ANIME_NORMAL;
		g_Player[i].moving = FALSE;							// 移動中フラグ
		g_Player[i].patternAnim = ANIME_NORMAL;				// 最初のアニメーション
		g_Player[i].pattern_old = ANIME_NORMAL;				// 通常にしておく
		// g_Player[i].patternAnim = g_Player[i].dir * TEXTURE_PATTERN_DIVIDE_X;
		// g_Player[i].patternAnim = g_Player[i].patternAnim * g_anime[1][g_Player[i].anim];

		// ジャンプの初期化
		g_Player[i].jump = FALSE;
		g_Player[i].jumpY = 0.0f;

		// 攻撃の初期化
		g_Player[i].attack = FALSE;

		g_Player[i].ground = FALSE;
		g_Player[i].updown = 0;
		g_Player[i].gravityY = 0.0f;						// 重力の初期化

		g_Player[i].damage = FALSE;							// 最初は攻撃に当たっていない
	}


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer(void)
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
void UpdatePlayer(void)
{
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		// 生きているプレイヤーだけ処理をする
		if (g_Player[i].use == TRUE)
		{
			// バックアップを取っておく
			g_Player[i].pos_old = g_Player[i].pos;				// 当たり判定用に移動前の座標のバックアップ
			g_Player[i].pattern_old = g_Player[i].patternAnim;	// 前のアニメーションのバックアップ

			// プレイヤーにかかる力を戻す
			g_Player[i].energy.y = 0;

			FIELD* field = GetField();
			CHECKPT* checkpt = GetCheckPt();


			// アニメーション  
			if ((g_Player[i].patternAnim != ANIME_HASHIGO) && (g_Player[i].movable)) g_Player[i].countAnim += g_PlayerAnim[g_Player[i].patternAnim].cntSpeed;

			if (g_PlayerAnim[g_Player[i].patternAnim].loop)
			{	// ループするアニメーションなら
				if (g_Player[i].countAnim > g_PlayerAnim[g_Player[i].patternAnim].xMax)
				{
					g_Player[i].countAnim = 0.0f;
				}
			}
			else
			{	// ループしないアニメーションなら
				if (g_Player[i].countAnim > (g_PlayerAnim[g_Player[i].patternAnim].xMax - 1.0f))
				{
					// 最後の絵にしておく
					g_Player[i].countAnim = (float)(g_PlayerAnim[g_Player[i].patternAnim].xMax) - 1.0f;

				}
			}
			if ((g_Player[i].moving == FALSE) && (g_Player[i].updown != UPDOWN_DURING)) {
				g_Player[i].hp >= PLAYER_HP_LAW ? g_Player[i].patternAnim = ANIME_NORMAL : g_Player[i].patternAnim = ANIME_CRISIS;
			}

			// ダメージをくらったら
			if (g_Player[i].damage == TRUE)
			{
				g_playerCnt[PLAYER_COUNT_DAMAGE]++;
				g_Player[i].patternAnim = ANIME_DAMAGE;

				if (g_playerCnt[PLAYER_COUNT_DAMAGE] > PLAYER_DAMAGE_INTERVAL / 2)
				{
					// 少し早めに操作可能に戻す
					g_Player[i].movable = TRUE;

					// 無敵時間終了
					if (g_playerCnt[PLAYER_COUNT_DAMAGE] > PLAYER_DAMAGE_INTERVAL)
					{
						g_Player[i].damage = FALSE;
						g_playerCnt[PLAYER_COUNT_DAMAGE] = 0.0f;
					}
				}
			}


			// 操作可能なときキー入力で移動 
			if (g_Player[i].movable == TRUE)
			{
				float speed = PLAYER_DUSH_SPEED;

				g_Player[i].moving = FALSE;


				// 下にハシゴなど昇り降りできるものがあるとき
				if (g_Player[i].updown != UPDOWN_FALSE)
				{
					// キーボードでの処理
					if (GetKeyboardPress(DIK_DOWN))
					{
						g_Player[i].pos.y += speed;
						g_Player[i].countAnim += g_PlayerAnim[g_Player[i].patternAnim].cntSpeed;
						g_Player[i].moving = TRUE;
					}
					else if (GetKeyboardPress(DIK_UP))
					{
						g_Player[i].pos.y -= speed;
						g_Player[i].countAnim += g_PlayerAnim[g_Player[i].patternAnim].cntSpeed;
						g_Player[i].moving = TRUE;
					}

					// ゲームパッドでの処理
					if (IsButtonPressed(0, BUTTON_DOWN))
					{
						g_Player[i].pos.y += speed;
						g_Player[i].countAnim += g_PlayerAnim[g_Player[i].patternAnim].cntSpeed;
						g_Player[i].moving = TRUE;
					}
					else if (IsButtonPressed(0, BUTTON_UP))
					{
						g_Player[i].pos.y -= speed;
						g_Player[i].countAnim += g_PlayerAnim[g_Player[i].patternAnim].cntSpeed;
						g_Player[i].moving = TRUE;
					}

					if (g_Player[i].updown == UPDOWN_DURING)
					{
						g_Player[i].patternAnim = ANIME_HASHIGO;
					}
				}
				
				
				if (g_Player[i].updown != UPDOWN_DURING)
				{
					if (GetKeyboardPress(DIK_RIGHT))
					{
						g_Player[i].pos.x += speed;
						g_Player[i].patternAnim = ANIME_WALK;
						g_Player[i].left = FALSE;
						g_Player[i].moving = TRUE;
					}
					else if (GetKeyboardPress(DIK_LEFT))
					{
						g_Player[i].pos.x -= speed;
						g_Player[i].patternAnim = ANIME_WALK;
						g_Player[i].left = TRUE;
						g_Player[i].moving = TRUE;
					}

					if (IsButtonPressed(0, BUTTON_RIGHT))
					{
						g_Player[i].pos.x += speed;
						g_Player[i].patternAnim = ANIME_WALK;
						g_Player[i].left = FALSE;
						g_Player[i].moving = TRUE;
					}
					else if (IsButtonPressed(0, BUTTON_LEFT))
					{
						g_Player[i].pos.x -= speed;
						g_Player[i].patternAnim = ANIME_WALK;
						g_Player[i].left = TRUE;
						g_Player[i].moving = TRUE;
					}
				}


				// ジャンプ処理中？
				if (g_Player[i].jump == TRUE)
				{
					// 長押しするほどジャンプ力が上がる
					if ((g_jumpPower < PLAYER_JUMP_POWER_MAX) && ((GetKeyboardPress(DIK_J)) || IsButtonPressed(0, BUTTON_A)))
					{
						g_jumpPower += 1.0f;
						g_jumpCntMax += 1.0f;
					}
					float angle = (XM_PI / (g_jumpCntMax * 2)) * g_playerCnt[PLAYER_COUNT_JUMP];
					float y = g_jumpPower * cosf(XM_PI / 2 + angle);
					g_Player[i].jumpY = g_jumpPower + y;
					g_Player[i].moving = TRUE;

					g_Player[i].patternAnim = ANIME_JUMP;

					g_playerCnt[PLAYER_COUNT_JUMP]++;

					if (g_playerCnt[PLAYER_COUNT_JUMP] > g_jumpCntMax)
					{
						g_Player[i].jump = FALSE;

						// 初期値に戻す
						g_playerCnt[PLAYER_COUNT_JUMP] = 0;
						g_Player[i].jumpY = 0.0f;
						g_jumpPower = PLAYER_JUMP_POWER_FIRST;
						g_jumpCntMax = PLAYER_JUMP_CNT_FIRST;
					}
				}
				// ジャンプボタン押した？
				else if ((g_Player[i].jump == FALSE) && ((GetKeyboardTrigger(DIK_J)) || IsButtonTriggered(0, BUTTON_A)))
				{
					if (g_Player[i].ground)
					{
						g_Player[i].jump = TRUE;
						g_playerCnt[PLAYER_COUNT_JUMP] = 0;
						g_Player[i].jumpY = 0.0f;
					}
				}
				else
				{
					// プレイヤーの足元が地面？
					if (g_Player[i].ground)
					{
						g_Player[i].gravityY = 0.0f;
						g_Player[i].energy.y = 0.0f;

						if (((GetKeyboardTrigger(DIK_G)) || (IsButtonTriggered(0, BUTTON_X))) && (g_Player[i].updown != UPDOWN_DURING))
						{
							g_Player[i].sliding = TRUE;
							float addX = (g_Player[i].left) ? g_Player[i].w / 2 : -g_Player[i].w / 2;
							SetEffect(XMFLOAT3(g_Player[i].pos.x + addX, g_Player[i].pos.y + g_Player[i].h / 3, 0.0f), 0, EFFECT_SLIDING);
						}
					}
					else 
					{
						g_Player[i].gravityY += PLAYER_GRAVITY;

						if (g_Player[i].gravityY > PLAYER_GRAVITY_MAX)
						{
							g_Player[i].gravityY = PLAYER_GRAVITY_MAX;
						}
						g_Player[i].energy.y += g_Player[i].gravityY;
					}
				}

				g_Player[i].energy.y -= g_Player[i].jumpY;

				// バレット処理
				if (GetKeyboardTrigger(DIK_SPACE))
				{
					XMFLOAT3 pos = g_Player[i].pos;
					SetPlayerBullet(pos);
					g_Player[i].attack = TRUE;
				}
				if (IsButtonTriggered(0, BUTTON_B))
				{
					XMFLOAT3 pos = g_Player[i].pos;
					SetPlayerBullet(pos);
					g_Player[i].attack = TRUE;
				}

				// MAP外チェック（チェックポイントまではスクロールする）
				BG* bg = GetBG();

				if (g_Player[i].pos.x > checkpt[field->checkPtCnt].x)
				{
					g_Player[i].pos.x = checkpt[field->checkPtCnt].x;
				}

				if (g_Player[i].pos.x < checkpt[field->checkPtCnt].prevX)
				{
					g_Player[i].pos.x = checkpt[field->checkPtCnt].prevX;
				}

				if (g_Player[i].pos.y > checkpt[field->checkPtCnt].y)
				{
					g_Player[i].pos.y = checkpt[field->checkPtCnt].y;
				}

				if (g_Player[i].pos.y < checkpt[field->checkPtCnt].prevY)
				{
					g_Player[i].pos.y = checkpt[field->checkPtCnt].prevY;
				}

				// プレイヤーの立ち位置からMAPのスクロール座標を計算する
				bg->pos.x = g_Player[i].pos.x - PLAYER_DISP_X;

				// ゲーム画面の左右の端はチェックポイントまで
				if (bg->pos.x < checkpt[field->checkPtCnt].prevX) bg->pos.x = checkpt[field->checkPtCnt].prevX;
				if (bg->pos.x > checkpt[field->checkPtCnt].x - GAMESCREEN_WIDTH) bg->pos.x = checkpt[field->checkPtCnt].x - GAMESCREEN_WIDTH;


				bg->pos.y = g_Player[i].pos.y - PLAYER_DISP_Y;

				// ゲーム画面の上下の端はチェックポイントまで
				if (bg->pos.y < checkpt[field->checkPtCnt].prevY) bg->pos.y = checkpt[field->checkPtCnt].prevY;
				if (bg->pos.y > checkpt[field->checkPtCnt].y - SCREEN_HEIGHT) bg->pos.y = checkpt[field->checkPtCnt].y - SCREEN_HEIGHT;

			}


			// 移動が終わったらエネミーとの当たり判定
			{
				ENEMY* enemy = GetEnemy();
				ENEMY_PATTERN* enemyPatt = GetEnemyPatt();

				// エネミーの数分当たり判定を行う
				for (int j = 0; j < ENEMY_MAX; j++)
				{
					// 生きてるエネミーと当たり判定をする
					if ((enemy[j].use == ENEMY_USE_TRUE) && (enemyPatt[enemy[j].type].damage))
					{
						BOOL ans = CollisionBB(g_Player[i].pos, g_Player[i].w, g_Player[i].h,
							enemy[j].pos, enemy[j].w, enemy[j].h);
						// 当たっている？
						if (ans == TRUE)
						{
							// 当たった時の処理
							DamagePlayer(PLAYER_COLLISION_DAMAGE);
						}
					}
				}
			}

			// プレイヤーにかかったエネルギー(重力など)を座標に反映
			XMVECTOR pos = XMLoadFloat3(&g_Player[i].pos);
			XMVECTOR energy = XMLoadFloat3(&g_Player[i].energy);
			pos += energy;
			XMStoreFloat3(&g_Player[i].pos, pos);

			// 前のフレームよりプレイヤーの位置が下がっていたら(落下中？)
			if ((g_Player[i].pos.y > g_Player[i].pos_old.y) && (g_Player[i].updown != UPDOWN_DURING))
			{
				g_Player[i].patternAnim = ANIME_DOWN;
			}

			// もしチェックポイントに到達したら
			if ((g_Player[i].pos.x == checkpt[field->checkPtCnt].x) && (checkpt[field->checkPtCnt].dir == 1))		// 右向きの場合
			{
				SetBGScroll(SCRL_DIR_RIGHT, field->checkPtCnt);
			}
			else if ((g_Player[i].pos.y == checkpt[field->checkPtCnt].y) && (checkpt[field->checkPtCnt].dir == 2))	// 下向きの場合
			{
				SetBGScroll(SCRL_DIR_DOWN, field->checkPtCnt);
			}
			else if ((g_Player[i].pos.y == checkpt[field->checkPtCnt].prevY) && (checkpt[field->checkPtCnt].dir == 0))	// 上向きの場合
			{
				SetBGScroll(SCRL_DIR_UP, field->checkPtCnt);
			}


#ifdef _DEBUG	// デバッグ情報を表示する
			BG* bg = GetBG();

			if (GetKeyboardTrigger(DIK_R))
			{
				g_Player[i].pos = XMFLOAT3(bg->pos.x, bg->pos.y, 0.0f);
			}
#endif
			// 攻撃アクション中？
			if (g_Player[i].attack == TRUE)
			{
				g_playerCnt[PLAYER_COUNT_ATTACK]++;
				g_Player[i].moving = TRUE;

				// 走ってる状態だったら走りながら攻撃の方のアニメーションに
				g_Player[i].patternAnim == ANIME_WALK ? g_Player[i].patternAnim = ANIME_ATTACKRUN : g_Player[i].patternAnim = ANIME_ATTACK;

				if (g_playerCnt[PLAYER_COUNT_ATTACK] > PLAYER_ATTACK_ACTION)
				{
					g_Player[i].attack = FALSE;
					g_playerCnt[PLAYER_COUNT_ATTACK] = 0.0f;
				}
			}

			// スライディング中？
			if (g_Player[i].sliding)
			{
				g_Player[i].moving = TRUE;
				g_Player[i].w = TEXTURE_HEIGHT;
				g_Player[i].collisionH = g_Player[i].h * LOSE_COLLISIONH;
				g_Player[i].patternAnim = ANIME_SLIDING;

				float slide = PLAYER_SLIDING_SPEED;
				g_Player[i].energy.x += (g_Player[i].left) ? slide * -1 : slide;
				g_playerCnt[PLAYER_COUNT_SLIDING]++;

				// speedの限界値
				if (slide > PLAYER_SLIDING_MAX) slide = PLAYER_SLIDING_MAX;

				if (g_playerCnt[PLAYER_COUNT_SLIDING] > PLAYER_SLIDING_ACTION)
				{
					g_Player[i].energy.x = 0.0f;
					g_Player[i].sliding = FALSE;
					g_Player[i].w = TEXTURE_WIDTH;
					g_Player[i].moving = FALSE;
					g_Player[i].collisionH = g_Player[i].h;
					g_playerCnt[PLAYER_COUNT_SLIDING] = 0.0f;
				}
			}


			// アニメーションが変わったらカウントを0に戻す
			if (g_Player[i].patternAnim != g_Player[i].pattern_old)
			{
				g_Player[i].countAnim = 0;
			}

		}

		// プレイヤーが死んだ
		else
		{
			g_playerCnt[PLAYER_COUNT_DEAD]++;
			
			if (g_playerCnt[PLAYER_COUNT_DEAD] > PLAYER_DAMAGE_INTERVAL)
			{
				FIELD* field = GetField();
				AddRetry(1);
				SetFade(FADE_OUT, MODE_GAME);						// 一旦フェードアウト
				g_playerCnt[PLAYER_COUNT_DEAD] = 0.0f;

			}
		}
	}


#ifdef _DEBUG	// デバッグ情報を表示する
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayer(void)
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

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (g_Player[i].use == TRUE)		// このプレイヤーが使われている？
		{									// Yes

			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_PlayerAnim[g_Player[i].patternAnim].texNo]);

			//プレイヤーの位置やテクスチャー座標を反映
			float px = g_Player[i].pos.x - bg->pos.x + OUTSIDE_WIDTH;	// プレイヤーの表示位置X
			float py = g_Player[i].pos.y - bg->pos.y;	// プレイヤーの表示位置Y
			float pw = g_Player[i].w;		// プレイヤーの表示幅
			float ph = g_Player[i].h;		// プレイヤーの表示高さ

			// アニメーション用
			float tw = 1.0f / g_PlayerAnim[g_Player[i].patternAnim].xMax;			// テクスチャの幅
			float th = 1.0f / 1;													// テクスチャの高さ
			float tx = (float)((int)g_Player[i].countAnim % g_PlayerAnim[g_Player[i].patternAnim].xMax) * fabsf(tw);	// テクスチャの左上X座標
			float ty = 0;		// テクスチャの左上Y座標(1枚の画像に全部収めるならxで割る)

			float angle = (XM_PI / PLAYER_ATTACK_ACTION) * g_playerCnt[PLAYER_COUNT_DAMAGE];

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, g_Player[i].opacity * fabsf(sinf(XM_PI / 2 + angle))),	// ダメージを受けたらしばらくチカチカさせる
				g_Player[i].rot.z, g_Player[i].left);								// 左向きの時は画像を反転

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

		}
	}


}


//=============================================================================
// Player構造体の先頭アドレスを取得
//=============================================================================
PLAYER* GetPlayer(void)
{
	return &g_Player[0];
}

//=============================================================================
// プレイヤーのHPを減らす
//=============================================================================
void DamagePlayer(float damage)
{
	if (g_Player[0].damage == FALSE)		// 次のダメージをくらえる状態になっていたら
	{
		g_Player[0].hp -= damage;

		if (g_Player[0].hp <= 0)
		{
			// HPがなくなったらuseをFALSEに
			g_Player[0].use = FALSE;
			SetEffect(g_Player[0].pos, 0, EFFECT_PLAYERDEAD);		// HPがなくなったときのエフェクト
			PlaySound(SOUND_LABEL_SE_playerDead00);
			StopSound(SOUND_LABEL_BGM_GAME);
		}
		else
		{
			g_Player[0].damage = TRUE;
			g_Player[0].movable = FALSE;
			PlaySound(SOUND_LABEL_SE_damage00);			// 当たった時の音
		}
	}
}



