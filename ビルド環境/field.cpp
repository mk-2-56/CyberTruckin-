//=============================================================================
//
// フィールド処理 [field.cpp]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#include "field.h"
#include "bg.h"
#include "player.h"
#include "collision.h"
#include "status.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAP_HEIGHT				(3000)				//
#define TEXTURE_MAX				(1)					// テクスチャの数
#define TEXTURE_WIDTH			(3150)				// 画像サイズ
#define TEXTURE_HEIGHT			(375)				// 
#define ADD_COLLISION			(1.2f)				// 当たり判定調整用

#define NDGTS					(100)
#define LBFFR					(NDGTS * 100 + 2)
#define NLINE					(50)
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/map.png",
};


static BOOL	g_Load = FALSE;		// 初期化を行ったかのフラグ
static FIELD g_field;
static CHECKPT g_checkpt[CHECKPOINT_MAX] = {
	// save, prevX, prevY, x, y, dir, enemyMax,
	{1, 0.0f, SCREEN_HEIGHT, 3975.0f, SCREEN_HEIGHT * 2, 2, 2},
	{1, 3975 - GAMESCREEN_WIDTH, SCREEN_HEIGHT * 2, 3975.0f, SCREEN_HEIGHT * 3, 1, 3},
	{1, 3975, SCREEN_HEIGHT * 2, 7050.0f, SCREEN_HEIGHT * 3, 0, 0},
	{1, 7050 - GAMESCREEN_WIDTH, SCREEN_HEIGHT, 7050.0f, SCREEN_HEIGHT * 2, 0, 3},
	{1, 7050 - GAMESCREEN_WIDTH, 0.0f, 8250.0f, SCREEN_HEIGHT, 1, 2},
	{1, 8250, 0.0f, 9450.0f, SCREEN_HEIGHT, 1, 0},
	{1, 9450, 0.0f, 10650.0f, SCREEN_HEIGHT, 0, 0},
};

static int g_map[MAP_HEIGHT / MAPCHIP_HEIGHT][MAP_WIDTH / MAPCHIP_WIDTH];
// x:16 y:12

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitField(void)
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


	// 変数の初期化
	g_field.w     = MAP_WIDTH;
	g_field.h     = MAP_HEIGHT;
	g_field.pos   = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_field.texNo = 0;


	const char* fnm = "data/INFO/map.csv";
	FILE* fp;
	char bffr[LBFFR];
	char* p;
	int col;
	int In = 0;

	if ((fp = fopen(fnm, "r")) == NULL)
	{
		printf("ファイルエラー！");
	}
	else
	{
		while (fgets(bffr, LBFFR, fp) != NULL)
		{
			p = bffr;
			col = 0;
			while ((p = strtok(p, "\n")) != NULL)
			{
				if (col >= 250)
				{
					printf("データ系列が多すぎます\n");
					continue;
				}
				g_map[In][col++] = int(atof(strtok(p, ",\n")));
				p = NULL;
			}
			In++;
		}

		// 終了
		fclose(fp);
		printf("終了");
	}

	g_Load = TRUE;
	return S_OK;

}

//=============================================================================
// 終了処理
//=============================================================================
void UninitField(void)
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
void UpdateField(void)
{
	// スクロール量を取得
	BG* bg = GetBG();
	float scrollx = bg->pos.x;
	float scrolly = bg->pos.y;

	PLAYER* player = GetPlayer();

	// プレイヤーとの当たり判定

	for (int y = 0; y < SCREEN_HEIGHT / MAPCHIP_HEIGHT + 1; y++)
	{
		for (int x = 0; x < GAMESCREEN_WIDTH / MAPCHIP_WIDTH + 1; x++)
		{
			int chip_x = x + (int)(scrollx / MAPCHIP_WIDTH);
			int chip_y = y + (int)(scrolly / MAPCHIP_HEIGHT);
			int chip_id = g_map[chip_y][chip_x];

			// 当たり判定のいらないマップチップ
			if (chip_id <= MAPCHIP_NOCOLLISION)
			{
				continue;
			}

			// プレイヤーの数分当たり判定を行う
			for (int i = 0; i < PLAYER_MAX; i++)
			{
				// プレイヤーとチップとの当たり判定をする
				if (player[i].use == TRUE)
				{
					XMFLOAT3 mapchip = XMFLOAT3((float)(chip_x * MAPCHIP_WIDTH + MAPCHIP_WIDTH / 2), (float)(chip_y * MAPCHIP_HEIGHT + MAPCHIP_HEIGHT / 2), 0.0f);
					float chipW = MAPCHIP_WIDTH;
					float playerW = player[i].w;
					if (chip_id == MAPCHIP_UPDOWN)
					{
						chipW = MAPCHIP_WIDTH * ADD_COLLISION;
						playerW = player[i].w * ADD_COLLISION;
					}

					// 全身との当たり判定
					BOOL ans = CollisionBB(mapchip, MAPCHIP_WIDTH, MAPCHIP_HEIGHT,
						player[i].pos, player[i].w, player[i].h);
					// 当たっている？
					if (ans == TRUE)
					{
						player[i].ground = TRUE;
						if (chip_id == MAPCHIP_UPDOWN)
						{
							player[i].updown = UPDOWN_DURING;
						}
						else
						{
							player[i].pos = player[i].pos_old;
						}
						return;
					}
				

					// 地面との判定
					XMFLOAT3 playerBtm = XMFLOAT3(player[i].pos.x, player[i].pos.y + player[i].h / 2 - 5.0f, 0.0f);
					BOOL ans2 = CollisionBB(mapchip, chipW, MAPCHIP_HEIGHT,
						playerBtm, playerW, player[i].h / 4);

					// 当たっている？
					if (ans2 == TRUE)
					{
						// 当たった時の処理
						player[i].ground = TRUE;

						if (chip_id == MAPCHIP_UPDOWN)
						{
							player[i].updown = UPDOWN_AVAILABLE;
						}
						else if (chip_id == MAPCHIP_DEATH)
						{
							DamagePlayer(PLAYER_HP_MAX);			// 即死
						}
						return;
					}
				}
			}
		}
	}
	player[0].ground = FALSE;
	player[0].updown = UPDOWN_FALSE;
	return;
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawField(void)
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

	// 背景と一緒にスクロールするため背景のスクロール量を取得
	BG* bg = GetBG();
	float scrollx = bg->pos.x;
	float scrolly = bg->pos.y;

	// テクスチャの中にマップチップが何種類分あるか
	int width_chip_num  = TEXTURE_WIDTH / MAPCHIP_WIDTH;
	int height_chip_num = TEXTURE_HEIGHT / MAPCHIP_HEIGHT;
		
	// フィールドを描画
	for (int y = 0; y < SCREEN_HEIGHT / MAPCHIP_HEIGHT + 2; y++)
	{
		for (int x = 0; x < GAMESCREEN_WIDTH / MAPCHIP_WIDTH + 3; x++)
		{
			int chip_id = g_map[y + (int)(scrolly / MAPCHIP_HEIGHT)][x + (int)(scrollx / MAPCHIP_WIDTH)];
			// int chip_id = g_map[y][x];

			// 無効な値かをチェック
			if (chip_id <= MAPCHIP_NOCOLLISION - 1)
			{
				continue;
			}

			// テクスチャ設定

			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_field.texNo]);

			float mx = (float)(MAPCHIP_WIDTH * x) - ((int)scrollx % MAPCHIP_WIDTH) + OUTSIDE_WIDTH;
			float my = (float)(MAPCHIP_HEIGHT * y) - ((int)scrolly % MAPCHIP_HEIGHT);
			
			// float mx = (float)(x * MAPCHIP_WIDTH);
			// float my = (float)(y * MAPCHIP_HEIGHT);


			float mw = MAPCHIP_WIDTH;
			float mh = MAPCHIP_HEIGHT;

			float tw = 1.0f / width_chip_num;
			float th = 1.0f / height_chip_num;
			float tx = (float)(chip_id % width_chip_num) * tw;
			float ty = (float)(chip_id / width_chip_num) * th;

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLTColor(g_VertexBuffer,
				mx, my, mw, mh,
				tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

		}
	}
}


//=============================================================================
// マップ配列の値を取得
//=============================================================================
// オブジェクトの座標をもらったら該当するマップチップのデータを渡す
int GetMap(float posX, float posY)
{
	int x = (int)(posX / MAPCHIP_WIDTH);
	int y = (int)(posY / MAPCHIP_HEIGHT);

	return g_map[y][x];
}

FIELD* GetField(void)
{
	return &g_field;
}

CHECKPT* GetCheckPt(void)
{
	return &g_checkpt[0];
}