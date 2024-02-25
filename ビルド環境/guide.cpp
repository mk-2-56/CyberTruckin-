//=============================================================================
//
// ガイド処理 [guide.cpp]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "sprite.h"
#include "player.h"
#include "guide.h"
#include "sound.h"
#include "status.h"
#include "field.h"
#include "bg.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX					(10)		// テクスチャの数
#define GUIDE_POINT1				(1700)		// 座標
#define GUIDE_POINT2				(1800)		// 座標
#define GUIDE_POINT3				(2400)		// 座標
#define GUIDE_POINT4				(2460)		// 座標
#define GUIDE_POINT5				(3600)		// 座標
#define GUIDE_POINT6				(3700)		// 座標
#define TIME_COUNT1					(70)		// 切り替え
#define TIME_COUNT2					(200)		// 切り替え
#define TIME_COUNT3					(300)		// 切り替え

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static BOOL						g_Load = FALSE;						// 初期化を行ったかのフラグ
static int						g_count = 0;						// 汎用カウント
static int						g_guideMove;						// どの動作のガイド中か

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/bg_start.jpg",
	"data/TEXTURE/text_ready.png",
	"data/TEXTURE/guide1_1.png",
	"data/TEXTURE/guide1_2.png",
	"data/TEXTURE/guide2_1.png",
	"data/TEXTURE/guide2_2.png",
	"data/TEXTURE/guide3_1.png",
	"data/TEXTURE/guide3_2.png",
	"data/TEXTURE/guide3_sub.png",
	"data/TEXTURE/guide4.png",
};

GUIDE g_guide[GUIDE_ALL] =
{
	{FALSE, XMFLOAT3(0.0f, 0.0f, 0.0f), SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f, 0},
	{FALSE, XMFLOAT3(GAMESCREEN_WIDTH / 2 + 50.0f, 360.0f, 0.0f), 346, 65, 1.0f, 1},
	{FALSE, XMFLOAT3(GAMESCREEN_WIDTH / 2 - 150.0f, 120.0f, 0.0f), 450, 256, 0.8f, 2},
	{FALSE, XMFLOAT3(GAMESCREEN_WIDTH / 2 + 150.0f, 270.0f, 0.0f), 450, 256, 0.8f, 3},
	{FALSE, XMFLOAT3(GAMESCREEN_WIDTH / 2 - 150.0f, 120.0f, 0.0f), 450, 256, 0.8f, 4},
	{FALSE, XMFLOAT3(GAMESCREEN_WIDTH / 2 + 150.0f, 270.0f, 0.0f), 450, 256, 0.8f, 5},
	{FALSE, XMFLOAT3(GAMESCREEN_WIDTH / 2 - 150.0f, 120.0f, 0.0f), 450, 256, 0.8f, 6},
	{FALSE, XMFLOAT3(GAMESCREEN_WIDTH / 2 + 150.0f, 270.0f, 0.0f), 450, 256, 0.8f, 7},
	{FALSE, XMFLOAT3(GAMESCREEN_WIDTH / 2 - 180.0f, 360.0f, 0.0f), 450, 256, 0.8f, 8},
	{FALSE, XMFLOAT3(GAMESCREEN_WIDTH / 2, 270.0f, 0.0f), 450, 256, 0.8f, 9},
};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitGuide(void)
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

	g_guide[GUIDE_READY].opacity = 1.0f;

#ifdef _DEBUG	// デバッグ情報を表示する
	/*SetTimer();
	SetStatus(STATUSGAME_DURING);
	PlaySound(SOUND_LABEL_BGM_GAME);*/

#endif

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitGuide(void)
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
void UpdateGuide(void)
{
	PLAYER* player = GetPlayer();
	FIELD* field = GetField();

	for (int i = 0; i < GUIDE_ALL; i++)
	{
		g_guide[i].use = FALSE;			// 一旦FALSEに
	}

	if (GetMode() == MODE_GAME)
	{
		// スタート描写
		if (g_count < TIME_COUNT2)
		{
			g_guide[GUIDE_START].use = TRUE;
			g_guide[GUIDE_READY].use = TRUE;
			g_guide[GUIDE_READY].opacity = 1.0f;
			player[0].movable = FALSE;
			player[0].opacity = 0.0f;

			if (g_count % TIME_COUNT1 == 0)
			{
				g_guide[GUIDE_READY].opacity = 0.0f;
			}
		}
		else if (g_count == TIME_COUNT2)
		{
			SetTimer();
			SetStatus(STATUSGAME_DURING);
			BGTYPE* bgtype = GetBGType();
			bgtype[field->checkPtCnt].use = TRUE;
		}
		else
		{
			g_guide[GUIDE_START].use = FALSE;
			g_guide[GUIDE_READY].use = FALSE;
			player[0].movable = TRUE;
			player[0].opacity = 1.0f;
		}

		if (g_count == TIME_COUNT3)
		{
			PlaySound(SOUND_LABEL_BGM_GAME);
		}

		if ((g_guide[GUIDE_START].use == FALSE) && (field->checkPtCnt == 0))
		{
			if ((player->pos.x > 100) && (player->pos.x < GUIDE_POINT1))
			{
				g_guide[GUIDE_MOVE1].use = TRUE;
				g_guide[GUIDE_MOVE1_2].use = TRUE;
			}

			else if ((player->pos.x > GUIDE_POINT2) && (player->pos.x < GUIDE_POINT3))
			{
				g_guide[GUIDE_MOVE2].use = TRUE;
				g_guide[GUIDE_MOVE2_2].use = TRUE;
			}

			else if ((player->pos.x > GUIDE_POINT4) && (player->pos.x < GUIDE_POINT5))
			{
				g_guide[GUIDE_MOVE3].use = TRUE;
				g_guide[GUIDE_MOVE3_2].use = TRUE;
				g_guide[GUIDE_MOVE3_SUB].use = TRUE;
			}
			else if ((player->pos.x > GUIDE_POINT6))
			{
				g_guide[GUIDE_MOVE4].use = TRUE;
				if (player->pos.y == SCREEN_HEIGHT * 2)
				{
					g_guide[GUIDE_MOVE4].use = FALSE;
				}
			}
		}
		g_count++;
	}

#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);
	// g_guide[GUIDE_MOVE].use = TRUE;
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawGuide(void)
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

	for (int i = 0; i < GUIDE_ALL; i++)
	{
		if (g_guide[i].use == TRUE)			// このガイドが使われている？
		{
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_guide[i].texNo]);

			float gx = g_guide[i].pos.x;
			float gy = g_guide[i].pos.y;
			float gw = g_guide[i].w;
			float gh = g_guide[i].h;

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLTColor(g_VertexBuffer, gx, gy, gw, gh, 0.0f, 0.0f, 1.0f, 1.0f,
				XMFLOAT4(1.0f, 1.0f, 1.0f, g_guide[i].opacity));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

}

//=============================================================================
// ガイドを始める
//=============================================================================
void ResetGuideCount(void)
{
	g_count = 0;
}
