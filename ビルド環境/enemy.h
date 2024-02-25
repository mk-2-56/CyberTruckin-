//=============================================================================
//
// �G�l�~�[���� [enemy.h]
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
#define ENEMY_MAX		(10)

enum enemyType
{
	ENEMY_PATT_SHOT,			// �ł��Ă���G
	ENEMY_PATT_BOMB,			// ���e
	ENEMY_PATT_BACKANDFORTH,	// �s�����藈����
	ENEMY_PATT_SHOT_SINGLE,		// �ꔭ�łG
	ENEMY_PATT_ALL,
};

enum enemyAnime
{
	ENEMY_ANIM_NORMAL = 0,
	ENEMY_ANIM_ATTACK,
	ENEMY_ANIM_ALL,
};

enum enemyUse
{
	ENEMY_USE_FALSE = 0,	// �܂��g�p���ĂȂ�
	ENEMY_USE_TRUE,			// �g�p��
	ENEMY_USE_DONE,			// �g�p����(�����g�p���Ȃ�)
	ENEMY_USE_REUSE,		// �g�p�������Ǖ����\
};

//*****************************************************************************
// �\���̒�`
//*****************************************************************************

// �G�l�~�[����
struct ENEMY
{
	BOOL				loop;				// �J��Ԃ��o�ꂷ�邩
	XMFLOAT3			pos;				// �|���S���̍��W
	int					type;				// �G�l�~�[�̃^�C�v


	BOOL				jump;				// �W�����v���t���O
	BOOL				wall;				// �ǂɓ�������
	XMFLOAT3			rot;				// �|���S���̉�]��
	XMFLOAT3			move;				// �|���S���̈ړ���
	XMFLOAT3			scl;				// �|���S���̊g��k��
	float				countAnim;			// �A�j���[�V�����J�E���g
	float				time;				// �o�����Ă���̎���
	float				w, h;				// ���ƍ���
	float				jumpY;				// �W�����v��
	int					use;				// ���g��enemyUse�̓��e
	int					texNo;				// �e�N�X�`���ԍ�
	int					patternAnim;		// �A�j���[�V�����p�^�[���i���o�[
	int					hp;					// HP
};

struct ENEMY_ANIME
{
	BOOL		loop;
	int			x, y;
	float		speed;
};

// �G�l�~�[�̃^�C�v
struct ENEMY_PATTERN
{
	BOOL			damage;						// �_���[�W��^���邩
	float			w, h;						// ���ƍ���
	int				texNo;						// �e�N�X�`���ԍ�
	int				hpMax;						// HP�̏����l(max���)
	ENEMY_ANIME		enemyAnime[ENEMY_ANIM_ALL];
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

ENEMY* GetEnemy(void);
ENEMY_PATTERN* GetEnemyPatt(void);

void SetEnemy(int label);
void DamageEnemy(int num, int damage);

void enemyShot(XMFLOAT3 pos, float time, int type);