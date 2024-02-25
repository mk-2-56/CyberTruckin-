//=============================================================================
//
// フィールド処理 [field.h]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAP_WIDTH				(10650)				// 背景サイズ
#define MAPCHIP_WIDTH			(75)			// マップチップのサイズ
#define MAPCHIP_HEIGHT			(75)			// 
#define CHECKPOINT_MAX			(7)				// チェックポイント数
#define MAPCHIP_NOCOLLISION		(35)			// これ以下は当たり判定のないマップチップ
#define	MAPCHIP_DEATH			(36)			// 即死マス
#define MAPCHIP_UPDOWN			(191)			// はしごのマス

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct FIELD
{
	XMFLOAT3	pos;		// ポリゴンの座標
	int			checkPtCnt;	// チェックポイントを何回超えたか
	float		w, h;		// 幅と高さ
	int			texNo;		// 使用しているテクスチャ番号
};

struct CHECKPT
{
	BOOL		save;				// 中間地点として使えるか
	float		prevX, prevY;		// x,y軸方向のmin
	float		x, y;				// x,y軸方向のmax
	int			dir;				// どの向きにスクロールするか 0:上 1:右 2:下 3:左
	int			enemyMax;			// 最大何体エネミーを出すか
};
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitField(void);
void UninitField(void);
void UpdateField(void);
void DrawField(void);

FIELD* GetField(void);
CHECKPT* GetCheckPt(void);

int GetMap(float posX, float posY);