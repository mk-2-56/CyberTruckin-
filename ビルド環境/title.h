//=============================================================================
//
// タイトル画面処理 [title.h]
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
enum TITLETEX
{
	TITLETEX_BG1 = 0,
	TITLETEX_BG2,
	TITLETEX_BG3,
	TITLETEX_BG4,
	TITLETEX_TEXT,
	TITLETEX_TEXTCURSOR,
	TITLETEX_ALL,
};

enum TITLEMENU
{
	TITLEMENU_GAME = 0,
	TITLEMENU_CREDIT,
	TITLEMENU_ALL,
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct TITLE
{
	BOOL		use;
	XMFLOAT3	pos;
	float		w, h;
	float		loop;
	float		opcaity;
	float		scrl;
	float		scrlSpeed;
	int			texNo;
};



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitTitle(void);
void UninitTitle(void);
void UpdateTitle(void);
void DrawTitle(void);


