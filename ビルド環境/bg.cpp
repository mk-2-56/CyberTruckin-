//=============================================================================
//
// BG処理 [bg.cpp]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#include "bg.h"
#include "player.h"
#include "field.h"
#include "sound.h"
#include "status.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	//
 
#define TEXTURE_MAX					(10)				// テクスチャの数

#define BG_SCRL_CNT_MAX				(60)			// スクロールに使うフレーム数
#define BG_SCRL_X_ERROR				(20)			// スクロールに伴うx軸の誤差
#define BG_SCRL_Y_ERROR				(15)			// スクロールに伴うy軸の誤差
#define BG_DECO_POS					(820)			// 修飾の間隔
#define DOWN_SPEED					(0.1f)			// 天井が降りるスピード
#define DOWN_POS_FIRST				(-575.0f)			// 天井が降りる初期位置

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/bg_Yozora.jpg",
	"data/TEXTURE/bg_underGround.png",
	"data/TEXTURE/bg_underGround2.png",
	"data/TEXTURE/bg_underGround3.png",
	"data/TEXTURE/bg_caution.png",
	"data/TEXTURE/bg_boss.png",
	"data/TEXTURE/lavaFalls2.png",
	"data/TEXTURE/lava.png",
	"data/TEXTURE/ceiling.png",
	"data/TEXTURE/bossBullet.png",
};


static BOOL	g_Load = FALSE;		// 初期化を行ったかのフラグ
static BG	g_BG;
static BGTYPE g_bgType[TEXTURE_MAX] = 
{
	{FALSE, XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f), 0, TEXTURE_HEIGHT, 0, 2.0f},	// use, pos, w, h, texNo, loop, auto, dir, scrlCnt
	{FALSE, XMFLOAT3(0.0f, -SCREEN_HEIGHT, 0.0f), 0, TEXTURE_HEIGHT, 1, 1.0f},
	{FALSE, XMFLOAT3(-GAMESCREEN_WIDTH, 0.0f, 0.0f), 0, TEXTURE_HEIGHT, 2, 1.5f},
	{FALSE, XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f), 0, TEXTURE_HEIGHT, 3, 1.0f},
	{FALSE, XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f), 0, TEXTURE_HEIGHT, 3, 1.0f},
	{FALSE, XMFLOAT3(-GAMESCREEN_WIDTH, 0.0f, 0.0f), 0, TEXTURE_HEIGHT, 4, 1.0f},
	{FALSE, XMFLOAT3(-GAMESCREEN_WIDTH, 0.0f, 0.0f), 0, TEXTURE_HEIGHT, 5, 1.0f},
};

static BGDECO g_bgDeco[BG_DECO_ALL] =
{
	{FALSE, XMFLOAT3(4820.0f, SCREEN_HEIGHT * 2, 0.0f), 128, 960, 0.1f, 6, 0, 1.0f, 3, 1, 2, 3}, // use, pos, w, h, speed, texNo, countAnim, x, y, checkPt, num;
	{FALSE, XMFLOAT3(4700.0f, 1640.0f + SCREEN_HEIGHT, 0.0f), 2368, 96, 0.1f, 7, 0, 1.0f, 3, 1, 2, 1},
	{FALSE, XMFLOAT3(7000.0f - GAMESCREEN_WIDTH, -420.0f, 0.0f), 2460.0f, 1082.0f, 0, 8, 0, 1.0f, 1, 1, 4, 1},
	{FALSE, XMFLOAT3(0.0f, 0.0f, 0.0f), 540.0f, 540.0f, 0, 9, 0, 0.0f, 5, 2, 6, 1},
};
static XMFLOAT3 g_restartPos = XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f);



//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitBG(void)
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

	CHECKPT* checkPt = GetCheckPt();
	for (int i = 0; i < CHECKPOINT_MAX; i++)
	{
		g_bgType[i].w = checkPt[i].x - checkPt[i].prevX + MAPCHIP_WIDTH;	// マップチップ一枚分多く描画しておく
		g_bgType[i].h = checkPt[i].y - checkPt[i].prevY;
	}

	// リトライのたびにトラップは位置を初期化
	g_bgDeco[BG_DECO_CEILING].pos.y = DOWN_POS_FIRST;
	g_bgDeco[BG_DECO_BOSSCLIMAX].opacity = 0.0f;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitBG(void)
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
void UpdateBG(void)
{
	FIELD* field = GetField();
	XMFLOAT3 speed = XMFLOAT3(GAMESCREEN_WIDTH / BG_SCRL_CNT_MAX, SCREEN_HEIGHT / BG_SCRL_CNT_MAX, 0.0f);

	// チェックポイントに合わせて背景を開始
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if ((g_bgType[i].use == TRUE) && (i == field->checkPtCnt))
		{
			g_bgType[i].pos.x = g_BG.pos.x - g_restartPos.x;
			g_bgType[i].pos.y = g_BG.pos.y - g_restartPos.y;
		}
	}

	// チェックポイントに合わせて背景の装飾を開始
	for (int j = 0; j < BG_DECO_ALL; j++)
	{
		if (g_bgDeco[j].use == TRUE)
		{
			g_bgDeco[j].countAnim+= g_bgDeco[j].speed;
			
			if (g_bgDeco[j].countAnim > (g_bgDeco[j].x * g_bgDeco[j].y)) g_bgDeco[j].countAnim = 0;
			
			if (j == BG_DECO_CEILING)
			{
				g_bgDeco[j].pos.y += DOWN_SPEED;			// 天井が迫る

				PLAYER* player = GetPlayer();
				// 生きてるエネミーと当たり判定をする
				if (player[0].use == TRUE)
				{
					BOOL ans = g_bgDeco[j].pos.y + g_bgDeco[j].h > player[0].pos.y - player[0].h / 2;

					// 当たっている？
					if (ans == TRUE)
					{
						DamagePlayer(PLAYER_HP_MAX);		// 即死
					}
				}
			}
		}
	}

	// 自動スクロール中
	if (g_BG.autoScrl)
	{
		PLAYER* player = GetPlayer();

		// スクロール中は操作不可
		player[0].movable = FALSE;

		switch (g_BG.dir)
		{
		case SCRL_DIR_UP:				// 上にスクロール
			g_BG.pos.y -= speed.y;
			g_bgType[field->checkPtCnt+1].pos.y -= speed.y;
			break;

		case SCRL_DIR_RIGHT:			// 右にスクロール
			g_BG.pos.x += speed.x;
			g_bgType[field->checkPtCnt + 1].pos.x += speed.x;
			break;

		case SCRL_DIR_DOWN:				// 下にスクロール
			g_BG.pos.y += speed.y;
			g_bgType[field->checkPtCnt + 1].pos.y += speed.y;
			break;

		case SCRL_DIR_LEFT:				// 左にスクロール
			g_BG.pos.x -= speed.x;
			g_bgType[field->checkPtCnt + 1].pos.x += speed.x;
			break;
		}
		g_BG.scrlCnt++;

		if (g_BG.scrlCnt > BG_SCRL_CNT_MAX)
		{
			g_BG.autoScrl = FALSE;
			g_BG.scrlCnt = 0;
			g_bgType[field->checkPtCnt].use = FALSE;		// 使用済み

			for (int i = 0; i < BG_DECO_ALL; i++)
			{
				if (g_bgDeco[i].checkPt == field->checkPtCnt) g_bgDeco[i].use = FALSE;	// 装飾も使用済み
			}
			field->checkPtCnt++;

			g_restartPos.x = g_BG.pos.x;
			g_restartPos.y = g_BG.pos.y;

			// スクロールの誤差を調整
			switch (g_BG.dir)
			{
			case SCRL_DIR_UP:
				g_restartPos.y = g_BG.pos.y + BG_SCRL_Y_ERROR;
				break;

			case SCRL_DIR_RIGHT:
				g_restartPos.x = g_BG.pos.x - BG_SCRL_X_ERROR;
				break;

			case SCRL_DIR_DOWN:
				g_restartPos.y = g_BG.pos.y - BG_SCRL_Y_ERROR;
				break;

			case SCRL_DIR_LEFT:
				g_restartPos.x = g_BG.pos.x + BG_SCRL_X_ERROR;
				break;
			}

			if (field->checkPtCnt == (CHECKPOINT_MAX - 1))
			{
				STATUS* status = GetStatusOBJ();
				StopSound(SOUND_LABEL_BGM_GAME);
				PlaySound(SOUND_LABEL_SE_warning);
				status[STATUS_WARNING].use = TRUE;
			}

			// 操作可能にする
			player[0].movable = TRUE;
		}
	}




#ifdef _DEBUG	// デバッグ情報を表示する


#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawBG(void)
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

	for (int i = 0; i < CHECKPOINT_MAX; i++)
	{
		if (g_bgType[i].use == TRUE)
		{
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_bgType[i].texNo]);

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLTColor(g_VertexBuffer,
				OUTSIDE_WIDTH - g_bgType[i].pos.x, 0 - g_bgType[i].pos.y, g_bgType[i].w, g_bgType[i].h,
				0.0f, 0.0f, g_bgType[i].loop, 1.0f,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

	for (int j = 0; j < BG_DECO_ALL; j++)
	{
		if (g_bgDeco[j].use == TRUE)
		{
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_bgDeco[j].texNo]);

			for (int k = 0; k < g_bgDeco[j].num; k++)
			{
				float dx = g_bgDeco[j].pos.x + BG_DECO_POS * k;
				float tw = 1.0f / g_bgDeco[j].x;	// テクスチャの幅
				float th = 1.0f / g_bgDeco[j].y;	// テクスチャの高さ
				float tx = (float)((int)g_bgDeco[j].countAnim % g_bgDeco[j].x) * fabsf(tw);	// テクスチャの左上X座標
				float ty = (float)((int)g_bgDeco[j].countAnim / g_bgDeco[j].x) * th;	// テクスチャの左上Y座標


				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSpriteLTColor(g_VertexBuffer,
					dx - g_BG.pos.x + OUTSIDE_WIDTH, g_bgDeco[j].pos.y - g_BG.pos.y, g_bgDeco[j].w, g_bgDeco[j].h,
					tx, ty, tw, th,
					XMFLOAT4(1.0f, 1.0f, 1.0f, g_bgDeco[j].opacity));

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}
}


//=============================================================================
// BG構造体の先頭アドレスを取得
//=============================================================================
BG* GetBG(void)
{
	return &g_BG;
}

//=============================================================================
// BGTYPE構造体の先頭アドレスを取得
//=============================================================================
BGTYPE* GetBGType(void)
{
	return &g_bgType[0];
}

//=============================================================================
// BGDECO構造体の先頭アドレスを取得
//=============================================================================
BGDECO* GetBGDeco(void)
{
	return &g_bgDeco[0];
}



//=============================================================================
// BGの自動スクロールを開始
//=============================================================================
void SetBGScroll(int dir, int num)
{
	if (num == CHECKPOINT_MAX - 1)
	{
		return;
	}

	// 指定した方向に向かってスクロールする
	g_BG.dir = dir;
	g_BG.autoScrl = TRUE;
	g_bgType[num + 1].use = TRUE;
	
	for (int i = 0; i < BG_DECO_ALL; i++)
	{
		if (g_bgDeco[i].checkPt == num + 1) g_bgDeco[i].use = TRUE;
	}
}

//=============================================================================
// BGのリセット
//=============================================================================
void ResetBG(void)
{
	// 2週目のゲームのみBGを最初の位置にリセット
	g_bgType[0].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_bgType[1].pos = XMFLOAT3(0.0f, -SCREEN_HEIGHT, 0.0f);
	g_bgType[2].pos = XMFLOAT3(-GAMESCREEN_WIDTH, 0.0f, 0.0f);
	g_bgType[3].pos = XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f);
	g_bgType[4].pos = XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f);
	g_bgType[5].pos = XMFLOAT3(-GAMESCREEN_WIDTH, 0.0f, 0.0f);
	g_bgType[6].pos = XMFLOAT3(-GAMESCREEN_WIDTH, 0.0f, 0.0f);

	g_restartPos = XMFLOAT3(0.0f, 900.0f, 0.0f);
}

					
					