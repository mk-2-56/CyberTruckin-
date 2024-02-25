//=============================================================================
//
// エフェクト処理 [effect.cpp]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#include "effect.h"
#include "bg.h"
#include "player.h"
#include "collision.h"
#include "sound.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define EMISSION_FULL 0			//パーティクル全生成フラグ
#define EMISSION_RATE 5			//パーティクルの生成間隔(duration/EMISSION_RATEの数分エフェクトが出る)

#define EMISSION_WIDTH  50		//パーティクル生成範囲（横幅）
#define EMISSION_HEIGHT 50		//パーティクル生成範囲（高さ）
#define EMISSION_PERTIME 0.01f	//経過時間の基準


#define TEXTURE_MAX					(10)		// テクスチャの数

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void ResetParticle(int i, int n);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char* g_TexturName[] = { 
	"data/TEXTURE/effect_enemyDead.png",
	"data/TEXTURE/effect_enemyHit.png",
	"data/TEXTURE/effect_explosion.png",
	"data/TEXTURE/effect_follow1_red.png",
	"data/TEXTURE/hitEffect01.png",
	"data/TEXTURE/playerDead01.png",
	"data/TEXTURE/attackLights.png",
	"data/TEXTURE/bossExplotion.png",
	"data/TEXTURE/effect_fireGround2.png",
	"data/TEXTURE/effect_sliding.png",
};


static BOOL			g_Load = FALSE;					// 初期化を行ったかのフラグ
static EFFECT		effectWk[EFFECT_NUM_EFFECTS];	// エフェクト構造体
static EFFECT_TYPE	effectType[EFFECT_ALL] ={
	{FALSE, 0, 180, 180, 5, 2, 1.0f, 0, 1, 3},		// loop, 合成タイプ, w, h, x分割数, y分割数, opacity, texNo, numParts, timeAnim
	{FALSE, 0, 150, 150, 1, 1, 1.0f, 1, 1, 5},
	{FALSE, 0, 600, 600, 3, 3, 1.0f, 2, 1, 3},
	{FALSE, 0, 192, 192, 5, 2, 1.0f, 3, 1, 4},
	{FALSE, 0, 86, 86, 5, 1, 0.8f, 4, 1, 4},
	{FALSE, 1, 360, 360, 10, 1, 0.8f, 5, 1, 8},
	{FALSE, 1, 60, 60, 7, 1, 1.0f, 6, 1, 1},
	{FALSE, 0, 500, 500, 2, 13, 1.0f, 7, 1, 14},
	{TRUE, 0, 146, 97, 1, 5, 1.0f, 8, 1, 4},
	{FALSE, 0, 40, 36, 4, 1, 1.0f, 9, 1, 2},
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEffect(void)
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


	// 初期化処理
	for (int i = 0; i < EFFECT_NUM_EFFECTS; i++)
	{
		effectWk[i].use = FALSE;
		effectWk[i].elapsed = 0;

		for (int n = 0; n < EFFECT_NUM_PARTS; n++)
		{
			ResetParticle(i, n);
		}
	}


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// パーティクルのリセット
//=============================================================================
void ResetParticle(int i, int n) 
{
	effectWk[i].pParticle[n].pos = XMFLOAT3(effectWk[i].pos.x, effectWk[i].pos.y, 0.0f);	// 座標データを初期化
	effectWk[i].pParticle[n].move = XMFLOAT3(0.0f, 0.0f, 0.0f);					// 移動量

	effectWk[i].pParticle[n].PatternAnim = 0;									// アニメパターン番号をランダムで初期化
	effectWk[i].pParticle[n].CountAnim = 0;										// アニメカウントを初期化
	effectWk[i].pParticle[n].liveTime = 2000;
	effectWk[i].pParticle[n].isFinish = 0;
	effectWk[i].pParticle[n].time = 0.0f;
	effectWk[i].pParticle[n].reverse = FALSE;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEffect(void)
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
void UpdateEffect(void)
{
	for (int i = 0; i < EFFECT_NUM_EFFECTS; i++)
	{
		if (effectWk[i].use)
		{
			if (effectWk[i].isEnding){
				effectWk[i].use = FALSE;
				continue;
			}

			//エミットが有効であればエフェクト作成処理を行う
			if (effectWk[i].isRemoveOnFinish == FALSE)
			{

				//エフェクト作成レートの増加処理
				if (effectWk[i].effectCount < effectWk[i].numParts)
					effectWk[i].emitCounter++;

				//バッファに空きがあり、追加タイミングが来ていれば新たなエフェクトを追加する
				while ((effectWk[i].effectCount < effectWk[i].numParts) && (effectWk[i].emitCounter > EMISSION_RATE))
				{
					//全体追加フラグがONであれば空き領域全てに新たなエフェクトを追加する
					if (EMISSION_FULL)
						effectWk[i].effectCount = effectWk[i].numParts;
					else
						effectWk[i].effectCount++;

					//エフェクト作成レートの初期化
					effectWk[i].emitCounter = 0;
				}

				effectWk[i].elapsed++;

				//時間超過
				if ((effectWk[i].duration != -1) && (effectWk[i].duration < effectWk[i].elapsed)){
					effectWk[i].isRemoveOnFinish = TRUE;
				}
			}

			int effectIndex = 0;
			int effectPatternNum = effectWk[i].x * effectWk[i].y;

			//エフェクトの更新処理
			while (effectIndex < effectWk[i].effectCount)
			{
				float vP0 = 1.0f;
				float vP1 = 0.4f;					// 制御点
				float vP2 = 0.0f;
				float temp = effectWk[i].pParticle[effectIndex].time;

				PLAYER* player = GetPlayer();

				// エフェクトごとに処理を変える
				switch (effectWk[i].effectType)
				{
				case EFFECT_EXPLOSION:													// 爆発エフェクト
					
					effectWk[i].pParticle[effectIndex].time += EMISSION_PERTIME;

					if (effectWk[i].pParticle[effectIndex].time > 1.0f)
					{
						effectWk[i].pParticle[effectIndex].time = 0.0f;
					}

					// 爆発エフェクトはだんだん透過させて煙を消す
					effectWk[i].pParticle[effectIndex].opacity = QuadraticBezierCurve_single(vP0, vP1, vP2, temp);

					BOOL ans1;
					effectWk[i].collisionW = effectWk[i].w * (effectWk[i].pParticle[effectIndex].time + 0.5f);
					effectWk[i].collisionH = effectWk[i].h * (effectWk[i].pParticle[effectIndex].time + 0.5f);
					ans1 = CollisionBB(XMFLOAT3(effectWk[i].pos.x, effectWk[i].pos.y, 0.0f), effectWk[i].collisionW, effectWk[i].collisionH,
						player[0].pos, player[0].w, player[0].h);

					// 当たっている？
					if (ans1 == TRUE)
					{
						// 当たった時の処理
						DamagePlayer(20.0f);
					}
					break;

				case EFFECT_BOSS_FOLLOW:												// ボスの追従エフェクト

					for (int j = 0; j < PLAYER_MAX; j++)
					{
						if (player[j].use == TRUE)
						{
							float heightAngle = (XM_PI / (effectWk[i].x * effectWk[i].y)) * (effectWk[i].pParticle[effectIndex].PatternAnim + 1);
							float h = effectType[EFFECT_BOSS_FOLLOW].h / 3 * cosf(XM_PI / 2 - heightAngle);
							effectWk[i].collisionH = h;

							BOOL ans2;
							ans2 = CollisionBB(XMFLOAT3(effectWk[i].pos.x, effectWk[i].pos.y + effectWk[i].h / 3, 0.0f), effectWk[i].collisionW, effectWk[i].collisionH,
								player[j].pos, player[j].w, player[j].h);

							// 当たっている？
							if (ans2 == TRUE)
							{
								// 当たった時の処理
								DamagePlayer(10.0f);
							}
						}
					}
					break;

				case EFFECT_PLAYERATTACK:

					// プレイヤーの銃の位置に追従する
					if (player[0].left)
					{
						effectWk[i].pParticle[effectIndex].pos = XMFLOAT3(player[0].pos.x - player[0].w / 2, player[0].pos.y - 15, 0.0f);
					}
					else
					{
						effectWk[i].pParticle[effectIndex].pos = XMFLOAT3(player[0].pos.x + player[0].w / 2, player[0].pos.y - 15, 0.0f);
					}
					break;
				}

				// 経過時間
				if (effectWk[i].pParticle[effectIndex].liveTime > 0)
				{
					//生存フレーム減少
					effectWk[i].pParticle[effectIndex].liveTime--;

					//アニメパターン進行
					if (++effectWk[i].pParticle[effectIndex].CountAnim > effectWk[i].timeAnim) {
						//アニメパターンが最大値に達した場合でも終了
						if (++effectWk[i].pParticle[effectIndex].PatternAnim >= (effectPatternNum - 1))
						{
							if (!effectWk[i].loop)
							{
								effectWk[i].pParticle[effectIndex].PatternAnim = effectPatternNum - 1;
								effectWk[i].pParticle[effectIndex].liveTime = 0;
							}
						}

						effectWk[i].pParticle[effectIndex].CountAnim = 0;
					}

					effectIndex++;

				}
				else{
					if (effectWk[i].isRemoveOnFinish){
						if (effectWk[i].pParticle[effectIndex].isFinish == 0)
						{
							effectWk[i].pParticle[effectIndex].isFinish = 1;
							effectWk[i].numFinish++;
						}
						//終了処理
						if (effectWk[i].numFinish == effectWk[i].effectCount)
						{
							effectWk[i].isEnding = TRUE;
							break;
						}
						effectIndex++;
					}
					else{
						//バッファを初期化する
						ResetParticle(i, effectIndex);

						//末尾でなければインデックスを詰める
						if (effectIndex != (effectWk[i].effectCount - 1))
						{
							effectWk[i].pParticle[effectIndex] = effectWk[i].pParticle[effectWk[i].effectCount - 1];
							ResetParticle(i, (effectWk[i].effectCount - 1));
						}
						effectWk[i].effectCount--;
					}
				}
			}
		}
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawEffect(void)
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
	PLAYER* player = GetPlayer();

	for (int i = 0; i < EFFECT_NUM_EFFECTS; i++)
	{
		if (effectWk[i].use == TRUE)		// このエフェクトが使われている？
		{									// Yes

			// 合成タイプを変える
			if (effectType[effectWk[i].effectType].synthesis) SetBlendState(BLEND_MODE_ADD);

			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[effectWk[i].texNo]);

			for (int n = 0; n < effectWk[i].effectCount; n++)
			{
				if (effectWk[i].pParticle[n].isFinish == 0) {
					//バレットの位置やテクスチャー座標を反映
					float px = effectWk[i].pParticle[n].pos.x - bg->pos.x + OUTSIDE_WIDTH;	// エフェクトの表示位置X
					float py = effectWk[i].pParticle[n].pos.y - bg->pos.y;	// エフェクトの表示位置Y
					float pw = effectWk[i].w;			// エフェクトの表示幅
					float ph = effectWk[i].h;			// エフェクトの表示高さ

					// px -= effectWk[i].w / effectWk[i].x;
					// py -= effectWk[i].h / effectWk[i].y;

					float tw = 1.0f / effectWk[i].x;	// テクスチャの幅
					float th = 1.0f / effectWk[i].y;	// テクスチャの高さ
					float tx = (float)(effectWk[i].pParticle[n].PatternAnim % effectWk[i].x) * tw;	// テクスチャの左上X座標
					float ty = (float)(effectWk[i].pParticle[n].PatternAnim / effectWk[i].x) * th;	// テクスチャの左上Y座標

					// プレイヤーの向きによってテクスチャを反転させるエフェクトの場合
					if ((effectWk[i].effectType == EFFECT_PLAYERATTACK) || effectWk[i].effectType == EFFECT_SLIDING)
					{
						effectWk[i].pParticle[n].reverse = player[0].left;
					}

					// １枚のポリゴンの頂点とテクスチャ座標を設定
					SetSpriteColorRotation(g_VertexBuffer,
						px, py, pw, ph,
						tx, ty, tw, th,
						XMFLOAT4(1.0f, 1.0f, 1.0f, effectWk[i].pParticle[n].opacity), 0.0f, effectWk[i].pParticle[n].reverse);

					// ポリゴン描画
					GetDeviceContext()->Draw(4, 0);
				}
			}

			SetBlendState(BLEND_MODE_ALPHABLEND);	// 合成タイプを元に戻す

		}
	}

}


//=============================================================================
// エフェクト構造体の先頭アドレスを取得
//=============================================================================
EFFECT *GetEffect(void)
{
	return &effectWk[0];
}


//=============================================================================
// エフェクトのセット
// 
// int duration		エフェクト発生源の生存時間
//=============================================================================
void SetEffect(XMFLOAT3 pos, int duration, int label)
{
	// もし未使用のエフェクトが無かったら実行しない( =これ以上表示できないって事 )
	for (int i = 0; i < EFFECT_NUM_EFFECTS; i++)
	{
		if (effectWk[i].use == FALSE)		// 未使用状態のエフェクトを見つける
		{
			effectWk[i].use = TRUE;
			effectWk[i].isEnding = FALSE;
			effectWk[i].isRemoveOnFinish = FALSE;

			effectWk[i].duration = duration;
			effectWk[i].pos = pos;							// 座標をセット

			effectWk[i].effectCount = 0;
			effectWk[i].elapsed = 0;
			effectWk[i].emitCounter = EMISSION_RATE;
			effectWk[i].numFinish = 0;

			effectWk[i].w = effectType[label].w;			// 選んだエフェクトタイプに対する情報をセット
			effectWk[i].h = effectType[label].h;
			effectWk[i].collisionW = (float)(effectType[label].w * 0.7);			// 選んだエフェクトタイプに対する情報をセット
			effectWk[i].collisionH = effectType[label].h;
			effectWk[i].x = effectType[label].x;
			effectWk[i].y = effectType[label].y;
			effectWk[i].texNo = effectType[label].texNo;
			effectWk[i].numParts = effectType[label].numParts;
			effectWk[i].timeAnim = effectType[label].timeAnim;
			effectWk[i].loop = effectType[label].loop;
			effectWk[i].effectType = label;

			//パーティクルの初期化
			for (int n = 0; n < effectWk[i].numParts; n++)
			{
				ResetParticle(i, n);
				effectWk[i].pParticle[n].opacity = effectType[label].opacity;
			}

			return;							// 1個セットしたので終了する
		}
	}
}
