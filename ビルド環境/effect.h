//=============================================================================
//
// �G�t�F�N�g���� [effect.h]
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

#define EFFECT_NUM_PARTS 30		//�G�t�F�N�g���p�[�e�B�N���ő吔
#define EFFECT_NUM_EFFECTS 10	//�G�t�F�N�g�ő吔

enum effectType
{
	EFFECT_ENEMYDEAD = 0,	// �G�l�~�[�����񂾂Ƃ�
	EFFECT_ENEMYHIT,		// �G�l�~�[�Ƀq�b�g
	EFFECT_EXPLOSION,		// ����
	EFFECT_BOSS_FOLLOW,		// �{�X�_�b�V���̒Ǐ]�G�t�F�N�g
	EFFECT_HIT_WALL,		// �v���C���[�̒e���ǂɓ������ď�����
	EFFECT_PLAYERDEAD,		// �v���C���[������
	EFFECT_PLAYERATTACK,	// �v���C���[�U�����̃G�t�F�N�g
	EFFECT_BOSSDEAD,		// �{�X������
	EFFECT_FIREGROUND,		// �n�ʂ��R���Ă�
	EFFECT_SLIDING,		// �X���C�f�B���O
	EFFECT_ALL,
};


//*****************************************************************************
// �\���̐錾
//*****************************************************************************

typedef struct	// �G�t�F�N�g�\����
{
	BOOL			reverse;					// ���]�t���O
	XMFLOAT3		pos;						// �|���S���̈ړ���
	XMFLOAT3		move;						// �ړ���
	float			time;						// �o�ߎ���
	float			opacity;					// ���ߗ�
	int				PatternAnim;				// �A�j���[�V�����p�^�[���i���o�[
	int				CountAnim;					// �A�j���[�V�����J�E���g

	int				liveTime;

	bool			isFinish;
}PARTICLE;

typedef struct	// �G�t�F�N�g�\����
{
	BOOL			loop;						// ���[�v
	BOOL			use;
	int				isEnding;
	bool			isRemoveOnFinish;

	XMFLOAT3		pos;						// �|���S���̈ړ���

	int				duration;
	int				elapsed;
	int				numFinish;

	int				effectCount;
	int				emitCounter;

	int				numParts;					// ���̃G�t�F�N�g�̃p�[�e�B�N����

	float			w,h;
	float			collisionW, collisionH;
	int				x,y;						// �A�j���[�V����������
	int				texNo;
	int				timeAnim;					// �؂�ւ��^�C����
	int				effectType;					// �ǂ̃G�t�F�N�g��

	PARTICLE		pParticle[EFFECT_NUM_PARTS];

}EFFECT;

typedef struct	// �G�t�F�N�g�̎�ނ̍\����
{
	BOOL			loop;						// ���[�v���邩
	BOOL			synthesis;					// true:���Z���� false:���Z����
	float			w,h;
	int				x, y;
	float			opacity;
	int				texNo;
	int				numParts;					// ���̃G�t�F�N�g�̃p�[�e�B�N����
	int				timeAnim;					// �؂�ւ��^�C����

}EFFECT_TYPE;


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitEffect(void);
void UninitEffect(void);
void UpdateEffect(void);
void DrawEffect(void);

EFFECT* GetEffect(void);
void SetEffect(XMFLOAT3 pos, int duration, int label);
