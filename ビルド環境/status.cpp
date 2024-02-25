//=============================================================================
//
// ステータス処理 [Status.cpp]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "sprite.h"
#include "status.h"
#include "player.h"
#include "enemy.h"
#include "boss.h"
#include "input.h"
#include "fade.h"
#include "sound.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX					(10)		// テクスチャの数
#define OPACITY_COUNT				(40)		
#define OPACITY_MAX					(0.9f)		// 最大透過率
#define BOSS_OPACITY				(0.05f)		// ボスの出現
#define WARNING_STOP				(140)		// warningをやめるタイミング

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static BOOL						g_Load = FALSE;						// 初期化を行ったかのフラグ
static int						g_GameStatus = STATUSGAME_START;	// 0:ゲーム進行中 1:ゲームスタート時 2:ゲームクリア時
static float					g_opacity;

static char *g_TexturName[] = {
	"data/TEXTURE/fontDegital.png",
	"data/TEXTURE/playerStatus.png",
	"data/TEXTURE/progressbar.png",
	"data/TEXTURE/progressbar.png",
	"data/TEXTURE/bossStatus.png",
	"data/TEXTURE/outside_left.png",
	"data/TEXTURE/outside_right.png",
	"data/TEXTURE/text_gameClear.png",
	"data/TEXTURE/text_warning.png",
	"data/TEXTURE/bar_white.png",
};

// ステータス構造体の配列
static STATUS	g_Status[STATUS_ALL] = {
	{TRUE, XMFLOAT3(SCREEN_WIDTH - 110.0f, 218.0f, 0.0f), 20, 31.6f, 0, 0},
	{TRUE, XMFLOAT3(172.0f, SCREEN_HEIGHT - 202, 0.0f), 24, 38, 0, 0},
	{TRUE, XMFLOAT3(SCREEN_WIDTH - 22.0f, SCREEN_HEIGHT - 120, 0.0f), 20, 32, 0, 0},
	{TRUE, XMFLOAT3(SCREEN_WIDTH - 22.0f, SCREEN_HEIGHT - 100, 0.0f), 16, 26, 0, 0},
	{TRUE, XMFLOAT3(10.0f + OUTSIDE_WIDTH, 10.0f, 0.0f), 397, 107, 1, 0},
	{TRUE, XMFLOAT3(132.0f + OUTSIDE_WIDTH, 45.0f, 0.0f), 270, 26, 2, 0},
	{TRUE, XMFLOAT3(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 75.0f, 0.0f), 708, 100, 4, 0},
	{TRUE, XMFLOAT3(497.0f, SCREEN_HEIGHT - 112.0f, 0.0f), 595, 38, 3, 0},
	{TRUE, XMFLOAT3(0.0f, 0.0f, 0.0f), OUTSIDE_WIDTH, SCREEN_HEIGHT, 5, 0},
	{TRUE, XMFLOAT3((SCREEN_WIDTH - OUTSIDE_WIDTH * 2) / 2, SCREEN_HEIGHT / 2, 0.0f), 338, 184, 7, 0},
	{FALSE, XMFLOAT3((SCREEN_WIDTH - OUTSIDE_WIDTH * 2) / 2, SCREEN_HEIGHT / 2, 0.0f), 500, 218, 8, 0},
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitStatus(void)
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


	// ステータスの初期化
	for (int i = 0; i < STATUS_ALL; i++)
	{
		g_Status[i].pos = g_Status[i].pos;
		g_Status[i].w = g_Status[i].w;
		g_Status[i].h = g_Status[i].h;
		g_Status[i].texNo = g_Status[i].texNo;
		g_Status[i].num = g_Status[i].num;
	}
	g_opacity = 0.0f;

	g_Status[STATUS_WARNING].use = FALSE;

	if (g_GameStatus == STATUSGAME_START)
	{
		g_Status[STATUS_TIME].num = 0;
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitStatus(void)
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
void UpdateStatus(void)
{
	if (g_GameStatus == STATUSGAME_CLEAR)
	{
		if (GetKeyboardTrigger(DIK_RETURN))
		{
			SetFade(FADE_OUT, MODE_RESULT);
			g_GameStatus = STATUSGAME_START;
		}
		if (IsButtonTriggered(0, BUTTON_B))
		{
			SetFade(FADE_OUT, MODE_RESULT);
			g_GameStatus = STATUSGAME_START;
		}
	}

	BOSS* boss = GetBoss();
	PLAYER* player = GetPlayer();
	if ((g_GameStatus == STATUSGAME_DURING) && (boss->hp > 0))
	{
		g_Status[STATUS_TIME].num = GetTimer();
	}

	if (g_Status[STATUS_WARNING].use == TRUE)
	{
		g_opacity++;
		boss->opacity += BOSS_OPACITY;
		boss->use = TRUE;
		player->movable = FALSE;
		
		if (g_opacity > WARNING_STOP)
		{
			g_Status[STATUS_WARNING].use = FALSE;
			g_opacity = 0.0f;
			boss->opacity = 1.0f;
			PlaySound(SOUND_LABEL_BGM_BOSS);
		}
	}


#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
	
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawStatus(void)
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

	// 画面外の情報
	{
		// 左側
		{
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Status[STATUS_OUTSIDE].texNo]);

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLeftTop(g_VertexBuffer, g_Status[STATUS_OUTSIDE].pos.x, g_Status[STATUS_OUTSIDE].pos.y, g_Status[STATUS_OUTSIDE].w, g_Status[STATUS_OUTSIDE].h,
				0.0f, 0.0f, 1.0f, 1.0f);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}

		// 右側
		{
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Status[STATUS_OUTSIDE].texNo + 1]);

			float rx = SCREEN_WIDTH - g_Status[STATUS_OUTSIDE].w;

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLeftTop(g_VertexBuffer, rx, g_Status[STATUS_OUTSIDE].pos.y, g_Status[STATUS_OUTSIDE].w, g_Status[STATUS_OUTSIDE].h,
				0.0f, 0.0f, 1.0f, 1.0f);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// 数字関係の描画
	{
		// スコアの表示

		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Status[STATUS_SCORE].texNo]);

		int numScore = g_Status[STATUS_SCORE].num;
		for (int i = 0; i < STATUS_SCORE_DIGIT; i++)
		{
			// 今回表示する桁の数字
			float x = (float)(numScore % 10);

			// スコアの位置やテクスチャー座標を反映
			float sx = g_Status[STATUS_SCORE].pos.x - g_Status[STATUS_SCORE].w * i;		// スコアの表示位置X
			float sy = g_Status[STATUS_SCORE].pos.y;									// スコアの表示位置Y
			float sw = g_Status[STATUS_SCORE].w;										// スコアの表示幅
			float sh = g_Status[STATUS_SCORE].h;										// スコアの表示高さ

			float tw = 1.0f / 10;		// テクスチャの幅
			float th = 1.0f / 1;		// テクスチャの高さ
			float tx = x * tw;			// テクスチャの左上X座標
			float ty = 0.0f;			// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, sx - 2*i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

			// 次の桁へ
			numScore /= 10;
		}

		// リトライ回数の表示

		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Status[STATUS_RETRY].texNo]);

		int numRetry = g_Status[STATUS_RETRY].num;
		for (int i = 0; i < STATUS_RETRY_DIGIT; i++)
		{
			// 今回表示する桁の数字
			float x = (float)(numRetry % 10);

			// スコアの位置やテクスチャー座標を反映
			float sx = g_Status[STATUS_RETRY].pos.x - g_Status[STATUS_RETRY].w * i;		// スコアの表示位置X
			float sy = g_Status[STATUS_RETRY].pos.y;									// スコアの表示位置Y
			float sw = g_Status[STATUS_RETRY].w;										// スコアの表示幅
			float sh = g_Status[STATUS_RETRY].h;										// スコアの表示高さ

			float tw = 1.0f / 10;		// テクスチャの幅
			float th = 1.0f / 1;		// テクスチャの高さ
			float tx = x * tw;			// テクスチャの左上X座標
			float ty = 0.0f;			// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, sx - 2*i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

			// 次の桁へ
			numRetry /= 10;
		}

		// 経過時間の表示

		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Status[STATUS_TIME].texNo]);

		int numTime = g_Status[STATUS_TIME].num / 10;
		int sec = (numTime / 100) % 60;
		int min = numTime / 6000;

		// スコアの位置やテクスチャー座標を反映
		float sy = g_Status[STATUS_TIME].pos.y;									// スコアの表示位置Y
		float sw = g_Status[STATUS_TIME].w;										// スコアの表示幅
		float sh = g_Status[STATUS_TIME].h;										// スコアの表示高さ

		float tw = 1.0f / 10;		// テクスチャの幅
		float th = 1.0f / 1;		// テクスチャの高さ
		float ty = 0.0f;			// テクスチャの左上Y座標

		// ミリ秒部分
		for (int i = 0; i < STATUS_TIME_DIGIT; i++)
		{
			// 今回表示する桁の数字
			float x = (float)(numTime % 10);

			float sx = g_Status[STATUS_TIME].pos.x - g_Status[STATUS_TIME_MILLI].w * i;		// スコアの表示位置X
			float sw = g_Status[STATUS_TIME_MILLI].w;										// スコアの表示幅
			float sh = g_Status[STATUS_TIME_MILLI].h;										// スコアの表示高さ
			float tx = x * tw;																// テクスチャの左上X座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, sx - 2*i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

			// 次の桁へ
			numTime /= 10;
		}

		// 秒数部分
		for (int i = 0; i < STATUS_TIME_DIGIT; i++)
		{
			// 今回表示する桁の数字
			float x = (float)(sec % 10);

			float sx = g_Status[STATUS_TIME].pos.x - g_Status[STATUS_TIME].w * i;		// スコアの表示位置X
			float tx = x * tw;															// テクスチャの左上X座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, sx - TIME_INTERVAL1 - 2*i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

			// 次の桁へ
			sec /= 10;
		}

		// 分数部分
		for (int i = 0; i < STATUS_TIME_DIGIT; i++)
		{
			// 今回表示する桁の数字
			float x = (float)(min % 10);

			float sx = g_Status[STATUS_TIME].pos.x - g_Status[STATUS_TIME].w * i;		// スコアの表示位置X
			float tx = x * tw;															// テクスチャの左上X座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, sx - TIME_INTERVAL2 - 2*i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

			// 次の桁へ
			min /= 10;
		}
	}

	// プレイヤーのステータスを描画
	{
		PLAYER* player = GetPlayer();

		// HPを描画
		{
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Status[STATUS_PLAYER_HP].texNo]);

			float hx = g_Status[STATUS_PLAYER_HP].pos.x;									// プレイヤーステータスの表示位置X
			float hy = g_Status[STATUS_PLAYER_HP].pos.y;									// プレイヤーステータスの表示位置Y
			float hw = (float)(g_Status[STATUS_PLAYER_HP].w / PLAYER_HP_MAX * player->hp);	// 現在の残りHP分の幅を表示
			float hh = g_Status[STATUS_PLAYER_HP].h;

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLTColor(g_VertexBuffer, hx, hy, hw, hh, 0.0f, 0.0f, 1.0f, 1.0f,
				XMFLOAT4(0.0f, 0.81f, 0.99f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}

		// 枠を描画
		{
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Status[STATUS_PLAYER].texNo]);

			float px = g_Status[STATUS_PLAYER].pos.x;	// プレイヤーステータスの表示位置X
			float py = g_Status[STATUS_PLAYER].pos.y;									// プレイヤーステータスの表示位置Y
			float pw = g_Status[STATUS_PLAYER].w;										// プレイヤーステータスの表示幅
			float ph = g_Status[STATUS_PLAYER].h;

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLeftTop(g_VertexBuffer, px, py, pw, ph, 0.0f, 0.0f, 1.0f, 1.0f);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// ボスのステータスを描画
	{
		BOSS* boss = GetBoss();

		for (int i = 0; i < BOSS_MAX; i++)
		{
			if (boss[i].use == TRUE)			// ボスなら
			{	 
				// 枠を描画
				{
					// テクスチャ設定
					GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Status[STATUS_BOSS].texNo]);

					float ex = g_Status[STATUS_BOSS].pos.x;	// プレイヤーステータスの表示位置X
					float ey = g_Status[STATUS_BOSS].pos.y;									// プレイヤーステータスの表示位置Y
					float ew = g_Status[STATUS_BOSS].w;										// プレイヤーステータスの表示幅
					float eh = g_Status[STATUS_BOSS].h;

					// １枚のポリゴンの頂点とテクスチャ座標を設定
					SetSprite(g_VertexBuffer, ex, ey, ew, eh, 0.0f, 0.0f, 1.0f, 1.0f);

					// ポリゴン描画
					GetDeviceContext()->Draw(4, 0);
				}

				// HPを描画
				{
					// テクスチャ設定
					GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Status[STATUS_BOSS_HP].texNo]);

					float hx = g_Status[STATUS_BOSS_HP].pos.x;									// プレイヤーステータスの表示位置X
					float hy = g_Status[STATUS_BOSS_HP].pos.y;									// プレイヤーステータスの表示位置Y
					float hw = (float)(g_Status[STATUS_BOSS_HP].w / BOSS_HP_MAX * boss[i].hp);	// 現在の残りHP分の幅を表示
					float hh = g_Status[STATUS_BOSS_HP].h;
					float tw = 1.0f / BOSS_HP_MAX * boss[i].hp;

					// １枚のポリゴンの頂点とテクスチャ座標を設定
					SetSpriteLTColor(g_VertexBuffer, hx, hy, hw, hh, 0.0f, 0.0f, tw, 1.0f,
						XMFLOAT4(1.0f, 0.3f, 0.0f, 1.0f));

					// ポリゴン描画
					GetDeviceContext()->Draw(4, 0);
				}
			}
		}
	}

	// ゲームクリア！
	{
		if (g_GameStatus == STATUSGAME_CLEAR)
		{
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Status[STATUS_GAMECLEAR].texNo]);

			float tx = g_Status[STATUS_GAMECLEAR].pos.x;
			float ty = g_Status[STATUS_GAMECLEAR].pos.y;
			float tw = g_Status[STATUS_GAMECLEAR].w;
			float th = g_Status[STATUS_GAMECLEAR].h;

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSprite(g_VertexBuffer, tx + OUTSIDE_WIDTH, ty, tw, th, 0.0f, 0.0f, 1.0f, 1.0f);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

	// warning
	{
		if (g_Status[STATUS_WARNING].use == TRUE)
		{
			// 文字
			{
				GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Status[STATUS_WARNING].texNo]);

				float tx = g_Status[STATUS_WARNING].pos.x;
				float ty = g_Status[STATUS_WARNING].pos.y;
				float tw = g_Status[STATUS_WARNING].w;
				float th = g_Status[STATUS_WARNING].h;

				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSprite(g_VertexBuffer, tx + OUTSIDE_WIDTH, ty, tw, th, 0.0f, 0.0f, 1.0f, 1.0f);

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);
			}

			// 背景
			{
				GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Status[STATUS_WARNING].texNo + 1]);

				float angle = (XM_PI / OPACITY_COUNT) * g_opacity;
				float currentOpacity = 1.0f * fabsf(sinf(XM_PI / 2 + angle));
				if (currentOpacity > OPACITY_MAX)
				{
					currentOpacity = OPACITY_MAX;
				}

				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSpriteLTColor(g_VertexBuffer, OUTSIDE_WIDTH, 0.0f, GAMESCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f,
					XMFLOAT4(1.0f, 0.0f, 0.0f, currentOpacity));

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}
}


//=============================================================================
// スコアを加算する
// 引数:add :追加する点数。マイナスも可能
//=============================================================================
STATUS* GetStatusOBJ(void)
{
	return &g_Status[0];
}

void AddScore(int add)
{
	g_Status[STATUS_SCORE].num += add;
	if (g_Status[STATUS_SCORE].num > STATUS_SCORE_MAX)
	{
		g_Status[STATUS_SCORE].num = STATUS_SCORE_MAX;
	}
}

void AddRetry(int add)
{
	g_Status[STATUS_RETRY].num += add;
	if (g_Status[STATUS_RETRY].num > STATUS_RETRY_MAX)
	{
		g_Status[STATUS_RETRY].num = STATUS_RETRY_MAX;
	}
}

int GetScore(void)
{
	return g_Status[STATUS_SCORE].num;
}

void SetStatus(int status)
{
	g_GameStatus = status;
}

int GetStatus(void)
{
	return g_GameStatus;
}

