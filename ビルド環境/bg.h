//=============================================================================
//
// BG処理 [bg.h]
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
// #define	SKY_H	(150.0f)	// 空のポリゴンの高さ

enum SCRL_DIR
{
	SCRL_DIR_UP = 0,
	SCRL_DIR_RIGHT,
	SCRL_DIR_DOWN,
	SCRL_DIR_LEFT,

	CHAR_DIR_MAX
};

enum BG_DECO
{
	BG_DECO_LAVA = 0,
	BG_DECO_LAVA2,
	BG_DECO_CEILING,
	BG_DECO_BOSSCLIMAX,
	BG_DECO_ALL,
};


//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct BG
{
	XMFLOAT3	pos;		// ポリゴンの座標
	BOOL		autoScrl;	// true:自動スクロール  false:プレイヤー操作によるスクロール
	int			dir;		// 0:上 1:右 2:下 3:左
	int			scrlCnt;	// スクロール中のカウント

};

struct BGTYPE
{
	BOOL		use;
	XMFLOAT3	pos;		// ポリゴンの座標
	float		w, h;		// 幅と高さ
	int			texNo;		// 使用しているテクスチャ番号
	float		loop;		// ループ回数
};

struct BGDECO
{
	BOOL		use;
	XMFLOAT3	pos;
	float		w, h;
	float		speed;		// アニメーションを進めるスピード
	int			texNo;
	float		countAnim;
	float		opacity;
	int			x, y;		// アニメーションの分割数
	int			checkPt;	// 所属するチェックポイント
	int			num;		// 何個まで同時に出すか
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitBG(void);
void UninitBG(void);
void UpdateBG(void);
void DrawBG(void);

BG* GetBG(void);
BGTYPE* GetBGType(void);
BGDECO* GetBGDeco(void);

void SetBGScroll(int dir, int num);
void ResetBG(void);
