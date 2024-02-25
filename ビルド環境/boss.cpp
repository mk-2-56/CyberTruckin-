//=============================================================================
//
// ボス処理 [boss.cpp]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#include "boss.h"
#include "bg.h"
#include "player.h"
#include "collision.h"
#include "enemyBullet.h"
#include "playerBullet.h"
#include "sound.h"
#include "effect.h"
#include "field.h"
#include "main.h"
#include "status.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(201)													// テクスチャの数
#define TEXTURE_HEIGHT				(252)													// テクスチャの数
#define TEXTURE_MAX					(5)														// テクスチャの数

#define GROUND_POS					(750)
#define BOSS_Y						(GROUND_POS - TEXTURE_HEIGHT / 2)

#define TEXTURE_PATTERN_DIVIDE_X	(1)														// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)														// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)		// アニメーションパターン数
#define ANIM_WAIT					(4)														// アニメーションの切り替わるWait値

#define JUMP_COUNT_MIN				(10)													// 小ジャンプフレーム数
#define JUMP_COUNT_MID				(30)													// 中ジャンプのフレーム数
#define JUMP_COUNT_MAX				(60)													// 大ジャンプのフレーム数
#define JUMP_POWER					(300.0f)												// ジャンプ力
#define JUMP_POWER_MID				(370.0f)												// 中ジャンプ力
#define JUMP_POWER_LARGE			(450.0f)												// 大きいジャンプ力
#define DUSH_SPEED					(15.0f)													// ダッシュスピード

#define	BOSS_GRAVITY				(2.4f)													// 重力
#define	BOSS_GRAVITY_MAX			(10.0f)													// 最大降下力

#define FOLLOW_EFFECT_MAX			(6)														// 追従エフェクトいくつ出すか
#define BULLET_HEIGHT				(550)													// バレットの基準高さ
#define BULLET_INTERVAL				(75)													// バレットの間隔
#define CLIMAX_END					(950)													// 最大攻撃をやめるタイミング
#define BOSS_DECO_OPACTY			(0.7f)													// ボスのオーラのアルファ値
#define BOSS_DOWN					(2.0f)													// ボスがだんだん降りる
#define BG_OPACITY					(0.02f)
#define	BOSS_TIME1					(30)
#define	BOSS_TIME2					(50)
#define	BOSS_TIME3					(300)
#define	BOSS_TIME4					(135)
#define	BOSS_TIME5					(600)
#define	BOSS_SCORE					(10000)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/normalBoss.png",
	"data/TEXTURE/wallBoss.png",
	"data/TEXTURE/deathBoss.png",
};

static BOOL				g_Load = FALSE;			// 初期化を行ったかのフラグ
static BOSS				g_Boss[BOSS_MAX];

static BOOL g_dir;								// 0:右 1:左
static int g_jumpCnt;
static int g_effectCnt;
static int g_patternOld;						// 前のアニメーションパターン
static int g_phaseOld;							// 前の行動段階
static float g_leftEnd;							// 左端の座標
static int g_climaxCnt = 0;							// クライマックス技を何回やったか

static XMFLOAT3 g_startPos;						// アクションスタート時の位置
static XMFLOAT3 g_targetPos;					// プレイヤーの位置を保存する用
static XMFLOAT3 g_effectPos;					// エフェクトを発生させる位置

static BOSSANIM g_BossAnime[BOSS_ANIM_ALL] =
{
	{1, 0.2f, 0, 8},								// loop, cntSpeed, texNo, xMax,
	{1, 0.2f, 0, 8},
	{1, 0.2f, 0, 8},
	{1, 0.2f, 1, 3},
	{0, 0.0f, 2, 1},
};

static BOSSACTION g_BossAction[ACTION_ALL] =
{
	{0},
	{40, 0, 40, 0},
	{40, 0, 15, 10},
	{50, 0, 20, 15},
};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitBoss(void)
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
	for (int i = 0; i < BOSS_MAX; i++)
	{
		g_Boss[i].damage = FALSE;
		g_Boss[i].use = FALSE;
		g_Boss[i].jump = FALSE;
		g_Boss[i].left = TRUE;									// 最初は左向き
		g_Boss[i].action = ACTION_NONE;
		g_Boss[i].phase  = PHASE_FIRST;
		g_Boss[i].pos = XMFLOAT3(10400.0f, 0.0f, 0.0f);
		g_Boss[i].energy = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Boss[i].rot  = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Boss[i].scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Boss[i].countAnim = 0.0f;
		g_Boss[i].time = 0.0f;
		g_Boss[i].w = TEXTURE_WIDTH;
		g_Boss[i].h = TEXTURE_HEIGHT;
		g_Boss[i].jumpY = 0.0f;
		g_Boss[i].gravityY = 0.0f;
		g_Boss[i].speed = 0.0f;
		g_Boss[i].texNo = 0;
		g_Boss[i].patternAnim = BOSS_ANIM_NORMAL;				// 最初のアニメーション
		g_Boss[i].hp = BOSS_HP_MAX;
		g_Boss[i].opacity = 0.0f;
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitBoss(void)
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
void UpdateBoss(void)
{
	FIELD* field = GetField();
	CHECKPT* checkPt = GetCheckPt();
	BGDECO* bgDeco = GetBGDeco();
	g_leftEnd = checkPt[field->checkPtCnt].prevX;	// ボスの行動座標の左端をセット


	if (g_Boss[0].use == TRUE)										// 生きている？
	{
		if (g_Boss[0].hp > 0)										// さらにhpがまだある？
		{
			PLAYER* player = GetPlayer();

			XMVECTOR epos = XMLoadFloat3(&g_Boss[0].pos);			// 前の座標のバックアップ
			g_patternOld = g_Boss[0].patternAnim;					// 前のアニメーションのバックアップ
			g_phaseOld = g_Boss[0].phase;							// 前の行動フェースのバックアップ

			// ボスにかかる力を戻す
			g_Boss[0].energy.y = 0;

			// アニメーション  
			g_Boss[0].countAnim += g_BossAnime[g_Boss[0].patternAnim].cntSpeed;
			if (g_BossAnime[g_Boss[0].patternAnim].loop)
			{	// ループするアニメーションなら
				if (g_Boss[0].countAnim > g_BossAnime[g_Boss[0].patternAnim].xMax)
				{
					g_Boss[0].countAnim = 0.0f;
				}
			}
			else
			{	// ループしないアニメーションなら
				if (g_Boss[0].countAnim > (g_BossAnime[g_Boss[0].patternAnim].xMax - 1.0f))
				{
					// 最後の絵にしておく
					g_Boss[0].countAnim = (float)(g_BossAnime[g_Boss[0].patternAnim].xMax) - 1.0f;
				}
			}



			// 今のアクションによって処理を切り替え
			switch (g_Boss[0].action)
			{
			case ACTION_NONE:
				g_Boss[0].pos.y += BOSS_DOWN;				// 登場時はだんだん降りる

				if (g_Boss[0].pos.y > BOSS_Y)
				{
					StartAction(ACTION_DASH);
					player[0].movable = TRUE;
					g_Boss[0].damage = TRUE;
				}
				break;

			case ACTION_DASH:

				switch (g_Boss[0].phase)
				{
				case PHASE_FIRST:								// まず移動方向を向く
					g_Boss[0].left = g_dir;
					g_Boss[0].patternAnim = BOSS_ANIM_NORMAL;

					// 待機行動
					SetIdle();
					break;

				case PHASE_SECOND:								// ジャンプして両端(スタート地点)に行く

					XMFLOAT3 dirPos;
					g_dir ? dirPos = XMFLOAT3(g_Boss[0].w / 2 + g_leftEnd, BOSS_Y, 0.0f) : dirPos = XMFLOAT3(MAP_WIDTH -  g_Boss[0].w / 2, BOSS_Y, 0.0f);

					SetJump(dirPos, g_Boss[0].pos, FALSE, JUMP_POWER_LARGE, JUMP_COUNT_MAX);
					break;

				case PHASE_THIRD:								// 次の進行方向を向く

					g_Boss[0].left = (g_dir) ? 0 : 1;			// 逆を向く
					g_Boss[0].patternAnim = BOSS_ANIM_NORMAL;

					SetIdle();
					break;

				case PHASE_FOURTH:								// 突進する

					g_Boss[0].damage = FALSE;
					XMFLOAT3 reversePos;						// 逆の端に向かって突進する
					g_dir ? reversePos = XMFLOAT3(MAP_WIDTH - g_Boss[0].w / 2, BOSS_Y, 0.0f) : reversePos = XMFLOAT3(g_Boss[0].w / 2 + g_leftEnd, BOSS_Y, 0.0f);
					XMVECTOR reverseVec = XMLoadFloat3(&reversePos) - epos;

					float angle = atan2f(reverseVec.m128_f32[1], reverseVec.m128_f32[0]);
					float speed = DUSH_SPEED;
					float move = cosf(angle) * fabsf(speed);

					g_Boss[0].patternAnim = BOSS_ANIM_NORMAL;
					g_Boss[0].pos.x += move;

					// 追従エフェクトをセット
					if ((fabsf(g_Boss[0].time * move) > g_Boss[0].w / 1.5) && (g_effectCnt < FOLLOW_EFFECT_MAX))
					{
						g_Boss[0].time = 0.0f;					// カウントを一旦戻す
						g_effectPos = XMFLOAT3(g_Boss[0].pos.x + (g_Boss[0].time * move), BOSS_Y + 45, 0.0f);
						SetEffect(g_effectPos, 0, EFFECT_BOSS_FOLLOW);

						g_effectCnt++;
					}

					// 端に到着したら
					if (fabsf(g_Boss[0].pos.x - reversePos.x) < 8.0f)
					{
						int next = rand() % 3;
						if (g_Boss[0].hp <= BOSS_HP_MAX / 2)			// HPが半分以下になったら
						{
							if (g_climaxCnt == 0)
							{
								StartAction(ACTION_CLIMAX);
							}
							else if (g_Boss[0].hp <= BOSS_HP_MAX / 4)
							{
								StartAction(ACTION_CLIMAX);
							}
							else
							{
								next == 0 ? StartAction(ACTION_JUMP) : StartAction(ACTION_DASH);
								g_Boss[0].damage = TRUE;
							}
						}
						else
						{
							next == 0 ? StartAction(ACTION_JUMP) : StartAction(ACTION_DASH);
							g_Boss[0].damage = TRUE;
						}
					}
					break;
				}

				break;


			case ACTION_JUMP:

				switch (g_Boss[0].phase)
				{
				case PHASE_FIRST:								// 待機行動

					SetIdle();
					break;

				case PHASE_SECOND:								// 近くの壁にジャンプ

					XMFLOAT3 dirPos;
					dirPos = g_Boss[0].pos.x > (SCREEN_WIDTH / 2 - OUTSIDE_WIDTH + g_leftEnd) ? XMFLOAT3(MAP_WIDTH - g_Boss[0].w / 2, 200.0f, 0.0f) : XMFLOAT3(g_Boss[0].w / 2 + g_leftEnd, 200.0f, 0.0f);
					SetJump(dirPos, g_Boss[0].pos, TRUE, JUMP_POWER_LARGE, JUMP_COUNT_MID);
					break;

				case PHASE_THIRD:								// 待機行動

					g_Boss[0].patternAnim = BOSS_ANIM_WALL;
					g_Boss[0].pos.x > (SCREEN_WIDTH / 2 - OUTSIDE_WIDTH + g_leftEnd) ? g_Boss[0].left = TRUE : g_Boss[0].left = FALSE;
					SetIdle();
					break;

				case PHASE_FOURTH:								// プレイヤーに向かって飛ぶ

					// 少しだけ上に上がってほぼ落ちるジャンプ
					if (g_Boss[0].jump == TRUE)
					{
						g_targetPos = player[0].pos;			// 少し前のフレームの座標を目標地にして避ける余地を残す
						XMVECTOR dirVec = XMLoadFloat3(&g_targetPos) - XMLoadFloat3(&g_Boss[0].pos);

						float angle = atan2f(dirVec.m128_f32[1], dirVec.m128_f32[0]);
						float speed = 18.0f;

						float jumpAngle = (XM_PI / JUMP_COUNT_MIN) * g_jumpCnt;
						float y = 5 * cosf(XM_PI / 2 + jumpAngle);

						g_Boss[0].jumpY = y;
						g_Boss[0].pos.x += cosf(angle) * fabsf(speed);
						g_Boss[0].patternAnim = BOSS_ANIM_NORMAL;

						g_jumpCnt++;

						g_Boss[0].energy.y += g_Boss[0].jumpY;


						if (g_jumpCnt > JUMP_COUNT_MIN)
						{
							// ジャンプは一旦ストップ
							g_jumpCnt = (int)(JUMP_COUNT_MIN)+1;

							if (g_Boss[0].pos.y > BOSS_Y)
							{
								g_Boss[0].gravityY = 0;
								g_Boss[0].pos.y = BOSS_Y;
								g_Boss[0].jump = FALSE;
								SetEnemyBullet(XMFLOAT3(g_Boss[0].pos.x, g_Boss[0].pos.y + g_Boss[0].h / 4, 0.0f), BULLET_PATT_HORIZONTAL);
								StartAction(ACTION_DASH);
							}
							else
							{
								g_Boss[0].gravityY += BOSS_GRAVITY;

								if (g_Boss[0].gravityY > BOSS_GRAVITY_MAX)
								{
									g_Boss[0].gravityY = BOSS_GRAVITY_MAX;
								}
								g_Boss[0].energy.y += g_Boss[0].gravityY;
							}
						}
					}
					// ジャンプする？
					else
					{
						g_Boss[0].jump = TRUE;
						g_jumpCnt = 0;
						g_Boss[0].jumpY = 0.0f;
					}

					break;
				}

				break;

			case ACTION_CLIMAX:									// 奥義
				switch (g_Boss[0].phase)
				{
				case PHASE_FIRST:								// 待機行動
					SetIdle();
					break;

				case PHASE_SECOND:								// 中央に向かってジャンプ
					XMFLOAT3 dirPos;
					dirPos = XMFLOAT3((g_leftEnd + MAP_WIDTH) / 2, 300.0f, 0.0f);
					SetJump(dirPos, g_Boss[0].pos, TRUE, JUMP_POWER_MID, JUMP_COUNT_MID);
					break;

				case PHASE_THIRD:
					SetIdle();
					break;

				case PHASE_FOURTH:
					bgDeco[BG_DECO_BOSSCLIMAX].use = TRUE;
					bgDeco[BG_DECO_BOSSCLIMAX].opacity = BOSS_DECO_OPACTY;		// 初期値に戻す
					bgDeco[BG_DECO_BOSSCLIMAX].pos = XMFLOAT3(g_Boss[0].pos.x - bgDeco[BG_DECO_BOSSCLIMAX].w /2, g_Boss[0].pos.y - bgDeco[BG_DECO_BOSSCLIMAX].h / 2.5f, 0.0f);

					if ((int)(g_Boss[0].time) % BULLET_INTERVAL == 0)
					{
						float posX = (g_dir) ? g_leftEnd : MAP_WIDTH;
						SetEnemyBullet(XMFLOAT3(posX, BULLET_HEIGHT, 0.0f), BULLET_PATT_BOSSCLIMAX);
					}
					if (g_Boss[0].time > CLIMAX_END) SetIdle();

					break;


				case PHASE_FIFTH:
					bgDeco[BG_DECO_BOSSCLIMAX].opacity -= BG_OPACITY;
					g_Boss[0].pos.y += BOSS_DOWN;
					g_Boss[0].damage = FALSE;
					if (bgDeco[BG_DECO_BOSSCLIMAX].opacity < 0)
					{
						bgDeco[BG_DECO_BOSSCLIMAX].use = FALSE;
					}

					if (g_Boss[0].pos.y > BOSS_Y)
					{
						StartAction(ACTION_DASH);
						g_climaxCnt++;
						g_Boss[0].damage = TRUE;
					}
					break;
				}
				break;
			}


			// 移動が終わったらプレイヤーとの当たり判定
			{
				for (int j = 0; j < PLAYER_MAX; j++)
				{
					// 生きてるエネミーと当たり判定をする
					if ((player[j].use == TRUE) && (g_Boss[0].action != ACTION_CLIMAX))
					{
						BOOL ans = CollisionBB(XMFLOAT3(g_Boss[0].pos.x, g_Boss[0].pos.y + g_Boss[0].h / 2, 0.0f), g_Boss[0].w, g_Boss[0].h / 2,
							player[j].pos, player[j].w, player[j].h);

						// 当たっている？
						if (ans == TRUE)
						{
							// 当たった時の処理
							DamagePlayer(10.0f);		// とりあえず
						}
					}
				}
			}

			// ボスにかかったエネルギー(重力など)を座標に反映
			XMVECTOR pos = XMLoadFloat3(&g_Boss[0].pos);
			XMVECTOR energy = XMLoadFloat3(&g_Boss[0].energy);
			pos += energy;
			XMStoreFloat3(&g_Boss[0].pos, pos);

			g_Boss[0].time += 1.0f;

			// アニメーションが変わったらカウントを0に戻す
			if (g_patternOld != g_Boss[0].patternAnim)
			{
				g_Boss[0].countAnim = 0;
			}

			// フェーズが変わったらエフェクトカウントを0に戻す
			if (g_phaseOld != g_Boss[0].phase)
			{
				g_Boss[0].time = 0;
				g_effectCnt = 0;
			}
		}

		else
		{	// 死ぬ間際の演出		

			g_Boss[0].time += 1.0f;
			PLAYER* player = GetPlayer();
			player->movable = FALSE;

			if (g_Boss[0].time == BOSS_TIME1)						// 少しだけスローにする
			{
				SetFps(FPS_VALUE);
			}
			if (g_Boss[0].time == BOSS_TIME2)
			{
				StopSound(SOUND_LABEL_BGM_BOSS);
				PlaySound(SOUND_LABEL_SE_bossDead);
				SetEffect(XMFLOAT3(g_Boss[0].pos.x, g_Boss[0].pos.y + g_Boss[0].h / 3, 0.0f), 0, EFFECT_BOSSDEAD);
			}
			if (g_Boss[0].time == BOSS_TIME3)
			{
				g_Boss[0].use = FALSE;
				g_Boss[0].time = 0;
			}
		}
	}
	else if(g_Boss[0].hp <= 0)								// useがfalseだがhpはすでに0→ボスを倒した後の処理
	{
		PLAYER* player = GetPlayer();
		player->movable = FALSE;
		g_Boss[0].time += 1.0f;
		if (g_Boss[0].time == BOSS_TIME4)
		{
			PlaySound(SOUND_LABEL_SE_gameClear);
		}
		if (g_Boss[0].time == BOSS_TIME5)
		{
			SetStatus(STATUSGAME_CLEAR);
			AddScore(BOSS_SCORE);
			PlaySound(SOUND_LABEL_SE_gameClear2);
		}
	}

#ifdef _DEBUG	// デバッグ情報を表示する


#endif
}


//=============================================================================
// 描画処理
//=============================================================================
void DrawBoss(void)
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

	for (int i = 0; i < BOSS_MAX; i++)
	{
		if (g_Boss[i].use == TRUE)			// このエネミーが使われている？
		{										// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_BossAnime[g_Boss[i].patternAnim].texNo]);

			//エネミーの位置やテクスチャー座標を反映
			float ex = g_Boss[i].pos.x - bg->pos.x + OUTSIDE_WIDTH;	// エネミーの表示位置X
			float ey = g_Boss[i].pos.y - bg->pos.y;	// エネミーの表示位置Y
			float ew = g_Boss[i].w;		// エネミーの表示幅
			float eh = g_Boss[i].h;		// エネミーの表示高さ

			float tw = 1.0f / g_BossAnime[g_Boss[i].patternAnim].xMax;	// テクスチャの幅
			float th = 1.0f;	// テクスチャの高さ
			float tx = (float)((int)g_Boss[i].countAnim % g_BossAnime[g_Boss[i].patternAnim].xMax) * tw;	// テクスチャの左上X座標
			float ty = 0.0f;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, ex, ey, ew, eh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, g_Boss[i].opacity),
				g_Boss[i].rot.z, g_Boss[i].left);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

}


//=============================================================================
// BOSS構造体の先頭アドレスを取得
//=============================================================================
BOSS* GetBoss(void)
{
	return &g_Boss[0];
}


//=============================================================================
// Bossに対するアクション
//=============================================================================
void DamageBoss(float damage)
{
	g_Boss[0].hp -= damage;
	PlaySound(SOUND_LABEL_SE_attack00);			// 当たった時の音

	if (g_Boss[0].hp <= 0)
	{
		PLAYER* player = GetPlayer();
		g_Boss[0].patternAnim = BOSS_ANIM_DEATH;
		player[0].movable = FALSE;
		player[0].damage = FALSE;

		// 当たった方向に少し飛ばされる
		PLAYER_BULLET* bullet = GetPlayerBullet();
		bullet->pos.x > g_Boss[0].pos.x ? g_dir = 1 : g_dir = 0;

		// HPがなくなったら一旦ゲームをゆっくりに
		SetFps(FPS_VALUE / 2);
		g_Boss[0].time = 0.0f;
	}
}

// タイプに応じた攻撃パターンを開始
void StartAction(int pattern)
{
	g_Boss[0].time = 0.0f;						// カウントを戻す
	g_Boss[0].action = pattern;

	if (pattern == ACTION_DASH)
	{
		g_dir = rand() % 2;						// 0:右 1:左

		int skip = rand() % 5;
		if (skip == 0)
		{										// まれに途中のフェーズを飛ばす
			g_Boss[0].phase = PHASE_THIRD;
			return;
		}
	}

	g_Boss[0].phase = PHASE_FIRST;
}

// ジャンプ行動
void SetJump(XMFLOAT3 dirPos, XMFLOAT3 pos, BOOL stay, float power, int cntMax)
{
	// ジャンプ処理中？
	if (g_Boss[0].jump == TRUE)
	{
		XMVECTOR dirVec = XMLoadFloat3(&dirPos) - XMLoadFloat3(&pos);

		float angle = atan2f(dirVec.m128_f32[1], dirVec.m128_f32[0]);
		float speed = (dirPos.x - g_startPos.x) / (cntMax);

		float jumpAngle;
		jumpAngle = stay ? (XM_PI / (cntMax * 2)) * g_jumpCnt : (XM_PI / cntMax) * g_jumpCnt;
		float y = power * cosf(XM_PI / 2 + jumpAngle);

		g_Boss[0].jumpY = y;

		g_Boss[0].pos.x += cosf(angle) * fabsf(speed);

		g_Boss[0].patternAnim = BOSS_ANIM_NORMAL;

		g_jumpCnt++;

		if (g_jumpCnt > cntMax)
		{
			g_Boss[0].jump = FALSE;

			// 初期値に戻す
			g_jumpCnt = 0;
			g_Boss[0].jumpY = 0.0f;

			// 次の行動フェーズへ
			g_Boss[0].phase++;

			if(stay) return;
		}
	}
	// ジャンプする？
	else
	{
		g_Boss[0].jump = TRUE;
		g_jumpCnt = 0;
		g_Boss[0].jumpY = 0.0f;

		g_startPos = pos;
	}

	g_Boss[0].pos.y = g_startPos.y + g_Boss[0].jumpY;
}


// 待機行動
void SetIdle(void)
{
	if (g_Boss[0].time > g_BossAction[g_Boss[0].action].cntMax[g_Boss[0].phase])	// 指定されたカウント数待機
	{
		PLAYER* player = GetPlayer();
		g_targetPos = player[0].pos;		// プレイヤーの位置を保存しておく

		g_Boss[0].phase++;					// 次のアクションフェーズへ
	}
}