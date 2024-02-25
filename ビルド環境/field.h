//=============================================================================
//
// �t�B�[���h���� [field.h]
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
#define MAP_WIDTH				(10650)				// �w�i�T�C�Y
#define MAPCHIP_WIDTH			(75)			// �}�b�v�`�b�v�̃T�C�Y
#define MAPCHIP_HEIGHT			(75)			// 
#define CHECKPOINT_MAX			(7)				// �`�F�b�N�|�C���g��
#define MAPCHIP_NOCOLLISION		(35)			// ����ȉ��͓����蔻��̂Ȃ��}�b�v�`�b�v
#define	MAPCHIP_DEATH			(36)			// �����}�X
#define MAPCHIP_UPDOWN			(191)			// �͂����̃}�X

//*****************************************************************************
// �\���̒�`
//*****************************************************************************

struct FIELD
{
	XMFLOAT3	pos;		// �|���S���̍��W
	int			checkPtCnt;	// �`�F�b�N�|�C���g�����񒴂�����
	float		w, h;		// ���ƍ���
	int			texNo;		// �g�p���Ă���e�N�X�`���ԍ�
};

struct CHECKPT
{
	BOOL		save;				// ���Ԓn�_�Ƃ��Ďg���邩
	float		prevX, prevY;		// x,y��������min
	float		x, y;				// x,y��������max
	int			dir;				// �ǂ̌����ɃX�N���[�����邩 0:�� 1:�E 2:�� 3:��
	int			enemyMax;			// �ő剽�̃G�l�~�[���o����
};
//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitField(void);
void UninitField(void);
void UpdateField(void);
void DrawField(void);

FIELD* GetField(void);
CHECKPT* GetCheckPt(void);

int GetMap(float posX, float posY);