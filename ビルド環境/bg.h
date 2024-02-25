//=============================================================================
//
// BG���� [bg.h]
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
// #define	SKY_H	(150.0f)	// ��̃|���S���̍���

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
// �\���̒�`
//*****************************************************************************

struct BG
{
	XMFLOAT3	pos;		// �|���S���̍��W
	BOOL		autoScrl;	// true:�����X�N���[��  false:�v���C���[����ɂ��X�N���[��
	int			dir;		// 0:�� 1:�E 2:�� 3:��
	int			scrlCnt;	// �X�N���[�����̃J�E���g

};

struct BGTYPE
{
	BOOL		use;
	XMFLOAT3	pos;		// �|���S���̍��W
	float		w, h;		// ���ƍ���
	int			texNo;		// �g�p���Ă���e�N�X�`���ԍ�
	float		loop;		// ���[�v��
};

struct BGDECO
{
	BOOL		use;
	XMFLOAT3	pos;
	float		w, h;
	float		speed;		// �A�j���[�V������i�߂�X�s�[�h
	int			texNo;
	float		countAnim;
	float		opacity;
	int			x, y;		// �A�j���[�V�����̕�����
	int			checkPt;	// ��������`�F�b�N�|�C���g
	int			num;		// ���܂œ����ɏo����
};

//*****************************************************************************
// �v���g�^�C�v�錾
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
