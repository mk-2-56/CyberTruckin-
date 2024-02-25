//=============================================================================
//
// リザルト画面処理 [result.h]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
enum RESULTTYPE
{
	RESULTTYPE_SCORE = 0,
	RESULTTYPE_RETRY,
	RESULTTYPE_PENALTY,
	RESULTTYPE_TIME,
	RESULTTYPE_TIME_MILLI,
	RESULTTYPE_BONUS,
	RESULTTYPE_ALL,
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct RESULT
{
	XMFLOAT3	pos;
	float		w, h;
	int			num;
};



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitResult(void);
void UninitResult(void);
void UpdateResult(void);
void DrawResult(void);
void ResetGame(void);


