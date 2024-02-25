//=============================================================================
//
// ガイド処理 [guide.h]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
enum GUIDEOBJ
{
	GUIDE_START = 0,
	GUIDE_READY,
	GUIDE_MOVE1,
	GUIDE_MOVE1_2,
	GUIDE_MOVE2,
	GUIDE_MOVE2_2,
	GUIDE_MOVE3,
	GUIDE_MOVE3_2,
	GUIDE_MOVE3_SUB,
	GUIDE_MOVE4,
	GUIDE_ALL,
};




//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct GUIDE
{
	BOOL		use;		// 使用中かどうか
	XMFLOAT3	pos;
	float		w, h;
	float		opacity;	// アルファ値
	int			texNo;
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitGuide(void);
void UninitGuide(void);
void UpdateGuide(void);
void DrawGuide(void);
void ResetGuideCount(void);