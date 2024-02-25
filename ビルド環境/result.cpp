//=============================================================================
//
// リザルト画面処理 [result.cpp]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#include "result.h"
#include "input.h"
#include "status.h"
#include "fade.h"
#include "field.h"
#include "bg.h"
#include "guide.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)						// 
#define TEXTURE_MAX					(2)									// テクスチャの数
#define STATUS_POINT_DIGIT			(4)									// ボーナスポイントの桁数
#define PENALTY_PER_RETRY			(100)								// リトライ1回ごとのペナルティ
#define BONUS_PER_TIME				(1000)								// タイムボーナスの追加基準値
#define STANDARD_TIME				(5)									// ボーナスの基準

//*****************************************************************************
// グローバル変数
//*****************************************************************************
RESULT g_result[RESULTTYPE_ALL]
{
	{XMFLOAT3(810.0f, 318, 0.0f), 67, 106, 0},		// pos, w, h, num
	{XMFLOAT3(860.0f, SCREEN_HEIGHT - 276, 0.0f), 28.8f, 45.6f, 0},
	{XMFLOAT3(924.0f, SCREEN_HEIGHT - 175, 0.0f), 28.8f, 45.6f, 0},
	{XMFLOAT3(1316.0f, SCREEN_HEIGHT - 358, 0.0f), 24, 38, 0},
	{XMFLOAT3(1325.0f, SCREEN_HEIGHT - 352, 0.0f), 24, 38, 0},
	{XMFLOAT3(1350.0f, SCREEN_HEIGHT - 202, 0.0f), 24, 38, 0},
};

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/board_result.png",
	"data/TEXTURE/fontDegital_white.png",
};

static BOOL						g_Load = FALSE;
static int						g_time = 0;
static int						g_score = 0;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitResult(void)
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

	STATUS* status = GetStatusOBJ();
	g_score = status[STATUS_SCORE].num;
	g_time = status[STATUS_TIME].num;
	g_result[RESULTTYPE_RETRY].num = status[STATUS_RETRY].num;
	g_result[RESULTTYPE_BONUS].num = 0;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitResult(void)
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
void UpdateResult(void)
{
	if (GetKeyboardTrigger(DIK_RETURN))
	{// Enter押したら、ステージを切り替える
		SetFade(FADE_OUT, MODE_TITLE);
		ResetGame();
	}
	// ゲームパッドで入力処理
	else if (IsButtonTriggered(0, BUTTON_START))
	{
		SetFade(FADE_OUT, MODE_TITLE);
		ResetGame();
	}
	else if (IsButtonTriggered(0, BUTTON_B))
	{
		SetFade(FADE_OUT, MODE_TITLE);
		ResetGame();
	}

	g_result[RESULTTYPE_PENALTY].num = g_result[RESULTTYPE_RETRY].num * PENALTY_PER_RETRY;
	g_result[RESULTTYPE_SCORE].num = g_score + g_result[RESULTTYPE_BONUS].num - g_result[RESULTTYPE_PENALTY].num;

	// windowsから取得した時間を分数へと変換
	int min = g_time / 10 / 6000;
	if (min < STANDARD_TIME)
	{
		g_result[RESULTTYPE_BONUS].num = BONUS_PER_TIME;
	}
	else if (min < STANDARD_TIME * 2)
	{
		g_result[RESULTTYPE_BONUS].num = BONUS_PER_TIME / 2;
	}

#ifdef _DEBUG	// デバッグ情報を表示する
	
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawResult(void)
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

	// リザルトの背景を描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLeftTop(g_VertexBuffer, 0.0f, 0.0f, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}



	// いろいろな数値表示
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		int numTime = g_time / 10;
		int sec = (numTime / 100) % 60;
		int min = numTime / 6000;

		// スコアの位置やテクスチャー座標を反映
		float sy = g_result[RESULTTYPE_TIME].pos.y;									// スコアの表示位置Y
		float sw = g_result[RESULTTYPE_TIME_MILLI].w;										// スコアの表示幅
		float sh = g_result[RESULTTYPE_TIME_MILLI].h;										// スコアの表示高さ

		float tw = 1.0f / 10;		// テクスチャの幅
		float th = 1.0f / 1;		// テクスチャの高さ
		float ty = 0.0f;			// テクスチャの左上Y座標

		// ミリ秒部分
		for (int i = 0; i < STATUS_TIME_DIGIT; i++)
		{
			// 今回表示する桁の数字
			float x = (float)(numTime % 10);

			float sx = g_result[RESULTTYPE_TIME_MILLI].pos.x - g_result[RESULTTYPE_TIME_MILLI].w * i;		// スコアの表示位置X
			float sw = g_result[RESULTTYPE_TIME_MILLI].w;										// スコアの表示幅
			float sh = g_result[RESULTTYPE_TIME_MILLI].h;										// スコアの表示高さ
			float tx = x * tw;																// テクスチャの左上X座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, sx - 2 * i, sy, sw, sh, tx, ty, tw, th,
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

			float sx = g_result[RESULTTYPE_TIME].pos.x - g_result[RESULTTYPE_TIME].w * i;		// スコアの表示位置X
			float tx = x * tw;															// テクスチャの左上X座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, sx - TIME_INTERVAL1 - 2 * i, sy, sw, sh, tx, ty, tw, th,
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

			float sx = g_result[RESULTTYPE_TIME].pos.x - g_result[RESULTTYPE_TIME].w * i;		// スコアの表示位置X
			float tx = x * tw;															// テクスチャの左上X座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, sx - TIME_INTERVAL2 - 2 * i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

			// 次の桁へ
			min /= 10;
		}

		// タイムボーナスを表示
		int bonus = g_result[RESULTTYPE_BONUS].num;
		for (int i = 0; i < STATUS_POINT_DIGIT; i++)
		{
			// 今回表示する桁の数字
			float x = (float)(bonus % 10);

			float sx = g_result[RESULTTYPE_BONUS].pos.x - g_result[RESULTTYPE_BONUS].w * i;		// スコアの表示位置X
			float sy = g_result[RESULTTYPE_BONUS].pos.y;										// スコアの表示位置Y
			float sw = g_result[RESULTTYPE_BONUS].w;											// スコアの表示幅
			float sh = g_result[RESULTTYPE_BONUS].h;											// スコアの表示高さ
			float tx = x * tw;																	// テクスチャの左上X座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, sx - 2 * i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

			// 次の桁へ
			bonus /= 10;
		}

		// トータルスコアを表示
		int total = g_result[RESULTTYPE_SCORE].num;
		for (int i = 0; i < STATUS_SCORE_DIGIT; i++)
		{
			// 今回表示する桁の数字
			float x = (float)(total % 10);

			float sx = g_result[RESULTTYPE_SCORE].pos.x - g_result[RESULTTYPE_SCORE].w * i;		// スコアの表示位置X
			float sy = g_result[RESULTTYPE_SCORE].pos.y;										// スコアの表示位置Y
			float sw = g_result[RESULTTYPE_SCORE].w;											// スコアの表示幅
			float sh = g_result[RESULTTYPE_SCORE].h;											// スコアの表示高さ
			float tx = x * tw;																	// テクスチャの左上X座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, sx - 2 * i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

			// 次の桁へ
			total /= 10;
		}

		// リトライに関するスコア
		{
			int numRetry = g_result[RESULTTYPE_RETRY].num;
			for (int i = 0; i < STATUS_RETRY_DIGIT; i++)
			{
				// 今回表示する桁の数字
				float x = (float)(numRetry % 10);

				// スコアの位置やテクスチャー座標を反映
				float sx = g_result[RESULTTYPE_RETRY].pos.x - g_result[RESULTTYPE_RETRY].w * i;		// スコアの表示位置X
				float sy = g_result[RESULTTYPE_RETRY].pos.y;										// スコアの表示位置Y
				float sw = g_result[RESULTTYPE_RETRY].w;											// スコアの表示幅
				float sh = g_result[RESULTTYPE_RETRY].h;											// スコアの表示高さ
				float tx = x * tw;																	// テクスチャの左上X座標

				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSpriteColor(g_VertexBuffer, sx - 2 * i, sy, sw, sh, tx, ty, tw, th,
					XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);

				// 次の桁へ
				numRetry /= 10;
			}

			// ペナルティの値を表示
			int penalty = g_result[RESULTTYPE_PENALTY].num;
			for (int i = 0; i < STATUS_POINT_DIGIT; i++)
			{
				// 今回表示する桁の数字
				float x = (float)(penalty % 10);

				float sx = g_result[RESULTTYPE_PENALTY].pos.x - g_result[RESULTTYPE_PENALTY].w * i;		// スコアの表示位置X
				float sy = g_result[RESULTTYPE_PENALTY].pos.y;											// スコアの表示位置Y
				float sw = g_result[RESULTTYPE_PENALTY].w;												// スコアの表示幅
				float sh = g_result[RESULTTYPE_PENALTY].h;												// スコアの表示高さ
				float tx = x * tw;																		// テクスチャの左上X座標

				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSpriteColor(g_VertexBuffer, sx - 2 * i, sy, sw, sh, tx, ty, tw, th,
					XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);

				// 次の桁へ
				penalty /= 10;
			}
		}

	}
}



//=============================================================================
// ゲームのリセット
//=============================================================================
void ResetGame(void)
{
	STATUS* status = GetStatusOBJ();
	status[STATUS_SCORE].num = 0;
	status[STATUS_RETRY].num = 0;

	FIELD* field = GetField();
	field->checkPtCnt = 0;

	ResetGuideCount();
	ResetBG();
}