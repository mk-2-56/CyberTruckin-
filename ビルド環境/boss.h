//=============================================================================
//
// �{�X���� [boss.h]
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
#define BOSS_MAX		(1)		// �{�X�̍ő吔
#define BOSS_HP_MAX		(300)	// �{�X��HP�����l

enum BossAction
{
	ACTION_NONE = 0,
	ACTION_DASH,
	ACTION_JUMP,
	ACTION_CLIMAX,
	ACTION_ALL,
};

enum ActionPhase
{
	PHASE_FIRST = 0,
	PHASE_SECOND,
	PHASE_THIRD,
	PHASE_FOURTH,
	PHASE_FIFTH,
	PHASE_ALL,
};

enum BossAnime
{
	BOSS_ANIM_NORMAL = 0,
	BOSS_ANIM_DASH,
	BOSS_ANIM_JUMP,
	BOSS_ANIM_WALL,
	BOSS_ANIM_DEATH,
	BOSS_ANIM_ALL,
};

//*****************************************************************************
// �\���̒�`
//*****************************************************************************

// �{�X�̍\����
struct BOSS
{
	BOOL				damage;				// ���G���Ԃ��̃t���O
	BOOL				use;				// true:�g���Ă���  false:���g�p
	BOOL				jump;				// �W�����v���t���O
	BOOL				left;				// �ǂ���������Ă邩
	int					action;				// �U���p�^�[�����t���O
	int					phase;				// �U���p�^�[���̒i�K
	XMFLOAT3			pos;				// �|���S���̍��W
	XMFLOAT3			energy;				// �{�X�ɂ�����G�l���M�[�̑���
	XMFLOAT3			rot;				// �|���S���̉�]��
	XMFLOAT3			scl;				// �|���S���̊g��k��
	float				countAnim;			// �A�j���[�V�����J�E���g
	float				time;				// �o�����Ă���̎���
	float				w, h;				// ���ƍ���
	float				jumpY;				// �W�����v��
	float				gravityY;			// �W�����v��
	float				speed;				// �X�s�[�h
	float				hp;					// HP
	float				opacity;			// ����
	int					texNo;				// �e�N�X�`���ԍ�
	int					patternAnim;		// �A�j���[�V�����p�^�[���i���o�[
};

struct BOSSANIM
{
	BOOL		loop;
	float		cntSpeed;		// �A�j���[�V�����J�E���g��i�߂�X�s�[�h
	int			texNo;
	int			xMax;
};

struct BOSSACTION
{
	float		cntMax[PHASE_ALL];
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitBoss(void);
void UninitBoss(void);
void UpdateBoss(void);
void DrawBoss(void);

BOSS* GetBoss(void);

void DamageBoss(float damage);

void StartAction(int pattern);

void SetJump(XMFLOAT3 dirPos, XMFLOAT3 pos, BOOL stay, float power, int cntMax);

void SetIdle(void);