//=============================================================================
//
// タイトル画面処理 [title.cpp]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#include "title.h"
#include "input.h"
#include "fade.h"
#include "sound.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(6)				// テクスチャの数

#define MENU_FIRST					(519)
#define MENU_INTERVAL				(110)
#define START_WAIT					(8)				// 効果音を聞き終わるために少し待つ

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/bg_title1.png",
	"data/TEXTURE/bg_title2.png",
	"data/TEXTURE/bg_title3.png",
	"data/TEXTURE/bg_title_fade.png",
	"data/TEXTURE/text_titleSet.png",
	"data/TEXTURE/cursor_titleMenu.png",
};

TITLE g_title[TITLETEX_ALL] =
{
	{TRUE, XMFLOAT3(0.0f, 0.0f, 0.0f), SCREEN_WIDTH, SCREEN_HEIGHT, 2.5f, 1.0f, 0, -0.0001f, 0},	// use, pos, w, h, loop, opacity, scrlSpeed, texNo
	{TRUE, XMFLOAT3(0.0f, 440.0f, 0.0f), SCREEN_WIDTH, 380, 2.5f, 1.0f, 0, 0.0002f, 1},
	{TRUE, XMFLOAT3(0.0f, 150.0f, 0.0f), SCREEN_WIDTH, SCREEN_HEIGHT - 100, 1.5f, 1.0f, 0, 0.0007f, 2},
	{TRUE, XMFLOAT3(0.0f, 0.0f, 0.0f), SCREEN_WIDTH, SCREEN_HEIGHT - 320, 1.0f, 1.0f, 0, 0, 3},
	{TRUE, XMFLOAT3(0.0f, 0.0f, 0.0f), SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f, 1.0f, 0, 0, 4},
	{TRUE, XMFLOAT3(SCREEN_WIDTH / 2 - 235, 519.0f, 0.0f), 466, 76, 1.0f, 1.0f, 0, 0, 5},
};
static BOOL	g_Load = FALSE;
static int g_menu;
static int g_time = 0;
static BOOL g_click;


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTitle(void)
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

	g_menu = TITLEMENU_GAME;		// 最初はタイトル
	g_time = 0;
	g_click = FALSE;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTitle(void)
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
void UpdateTitle(void)
{
	// モードの切り替え
	{
		// 下に進む
		// キーボード
		if (GetKeyboardTrigger(DIK_DOWN))
		{
			g_menu++;
			PlaySound(SOUND_LABEL_SE_menuSelect);
		}
		// ゲームパッド
		else if (IsButtonTriggered(0, BUTTON_DOWN))
		{
			g_menu++;
			PlaySound(SOUND_LABEL_SE_menuSelect);
		}

		// 上に進む
		// キーボード
		if (GetKeyboardTrigger(DIK_UP))
		{
			g_menu--;
			PlaySound(SOUND_LABEL_SE_menuSelect);
		}
		// ゲームパッド
		else if (IsButtonTriggered(0, BUTTON_UP))
		{
			g_menu--;
			PlaySound(SOUND_LABEL_SE_menuSelect);
		}
	}

	// 次のモードへ
	{
		// キーボード
		if (GetKeyboardTrigger(DIK_RETURN))
		{
			g_click = TRUE;
			PlaySound(SOUND_LABEL_SE_titleClick);
		}

		// ゲームパッドで入力処理
		else if (IsButtonTriggered(0, BUTTON_START))
		{
			g_click = TRUE;
			PlaySound(SOUND_LABEL_SE_titleClick);
		}
		else if (IsButtonTriggered(0, BUTTON_B))
		{
			g_click = TRUE;
			PlaySound(SOUND_LABEL_SE_titleClick);
		}

	 }

	for (int i = 0; i < TITLETEX_ALL; i++)
	{
		if (g_title[i].use == TRUE)
		{
			g_title[i].scrl += g_title[i].scrlSpeed;
		}
	}

	if (g_click)
	{
		g_time++;
		if (g_time > START_WAIT)
		{
			SetFade(FADE_OUT, (abs(g_menu) % TITLEMENU_ALL) + 1);		// 一番最初のモードはタイトルなので+1する
			g_time = 0;
		}
	}

	g_title[TITLETEX_TEXTCURSOR].pos.y = (float)(MENU_FIRST + (MENU_INTERVAL * (abs(g_menu) % TITLEMENU_ALL)));




#ifdef _DEBUG	// デバッグ情報を表示する

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTitle(void)
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

	for (int i = 0; i < TITLETEX_ALL; i++)
	{
		if (g_title[i].use == TRUE)
		{
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_title[i].texNo]);

			float sx = g_title[i].pos.x - g_title[i].scrl;
			float sy = g_title[i].pos.y;
			float sw = g_title[i].w;
			float sh = g_title[i].h;

			float tw = g_title[i].loop;

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLeftTop(g_VertexBuffer, sx, sy, sw, sh, g_title[i].scrl, 0.0f, tw, 1.0f);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}
}





