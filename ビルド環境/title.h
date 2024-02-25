//=============================================================================
//
// �^�C�g����ʏ��� [title.h]
// Author : GP11A132 15 �����ق̍�
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"


//*****************************************************************************
// �}�N����`
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
// �\���̒�`
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
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitTitle(void);
void UninitTitle(void);
void UpdateTitle(void);
void DrawTitle(void);


