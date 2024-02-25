//=============================================================================
//
// ���U���g��ʏ��� [result.h]
// Author : GP11A132 15 �����ق̍�
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "sprite.h"

//*****************************************************************************
// �}�N����`
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
// �\���̒�`
//*****************************************************************************
struct RESULT
{
	XMFLOAT3	pos;
	float		w, h;
	int			num;
};



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitResult(void);
void UninitResult(void);
void UpdateResult(void);
void DrawResult(void);
void ResetGame(void);


