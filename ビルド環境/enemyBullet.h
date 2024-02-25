//=============================================================================
//
// �G�l�~�[����̃o���b�g���� [enemyBullet.h]
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
#define BULLET_MAX				(100)	// ��ʂɕ\������ő吔

enum bulletType
{
	BULLET_PATT_SPREAD = 0,		// 360�x�ɍL�����Ă����O��
	BULLET_PATT_HORIZONTAL,		// ���E�ɍL����O��
	BULLET_PATT_BOSSCLIMAX,		// �{�X�̃N���C�}�b�N�X
	BULLET_PATT_SINGLE,		// �V���O���V���b�g
	BULLET_PATT_ALL,
};


//*****************************************************************************
// �\���̒�`
//*****************************************************************************


// �o���b�g����
struct BULLET
{
	BOOL				left;				// �摜���]�t���O
	BOOL				use;				// true:�g���Ă���  false:���g�p
	XMFLOAT3			pos;				// �o���b�g�̍��W
	XMFLOAT3			rot;				// �o���b�g�̉�]��
	XMFLOAT3			move;				// �o���b�g�̈ړ���
	float				w, h;				// ���ƍ���
	float				damage;				// �v���C���[�ɗ^����_���[�W��
	float				speed;				// �X�s�[�h
	int					texNo;				// �ǂ̃e�N�X�`�����g����
	int					cntAnime;			// �A�j���[�V�����J�E���g
	int					type;				// �ǂ̃^�C�v��Bullet��
};

// �o���b�g�̃^�C�v
struct BULLET_PATTERN
{
	float				w, h;				// ���ƍ���
	float				damage;				// �v���C���[�ɗ^����_���[�W��
	float				speed;				// �X�s�[�h
	int					max;				// �ő�e��
	int					texNo;				// �ǂ̃e�N�X�`�����g����
	int					x,y;				// �A�j���[�V�����p�^�[����
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitEnemyBullet(void);
void UninitEnemyBullet(void);
void UpdateEnemyBullet(void);
void DrawEnemyBullet(void);

// BULLET *GetBullet(void);

void SetEnemyBullet(XMFLOAT3 pos, int type);
