//=============================================================================
//
// �v���C���[���� [player.h]
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
#define PLAYER_MAX			(1)			// �v���C���[��Max�l��
#define	PLAYER_OFFSET_CNT	(16)		// 16���g
#define	PLAYER_HP_MAX		(100.0f)	// �ő�HP
#define	PLAYER_HP_LAW		(20.0f)		// �s���`�t���O
#define	PLAYER_LOSE_HEIGHT	(25.0f)		// �X���C�f�B���O�Ō��鍂��
#define	PLAYER_DAMAGE_MIN	(10.0f)		// �v���C���[�ɗ^������_���[�W�A�ア����
#define LOSE_COLLISIONH_POS	(15.0f)		// �X���C�f�B���O�ŉ����锻��


// �v���C���[�̍s�����J�E���g����p
enum PLAYER_COUNT
{
	PLAYER_COUNT_JUMP = 0,
	PLAYER_COUNT_DAMAGE,
	PLAYER_COUNT_ATTACK,
	PLAYER_COUNT_SLIDING,
	PLAYER_COUNT_DEAD,

	PLAYER_COUNT_ALL,
};

// �A�j���[�V�����̎��
enum ANIME
{
	ANIME_NORMAL = 0,			// �ʏ�
	ANIME_JUMP,					// �W�����v
	ANIME_WALK,					// ����
	ANIME_HASHIGO,				// ����~��
	ANIME_ATTACK,				// �U��
	ANIME_ATTACKRUN,			// �U��(����Ȃ���)
	ANIME_DAMAGE,				// �_���[�W���󂯂�
	ANIME_DOWN,					// ������
	ANIME_CRISIS,				// �s���`(HP�����Ȃ�)
	ANIME_SLIDING,				// �X���C�f�B���O
	ANIME_ALL,
};

enum UPDOWN
{
	UPDOWN_FALSE = 0,
	UPDOWN_AVAILABLE,
	UPDOWN_DURING,
};

//*****************************************************************************
// �\���̒�`
//*****************************************************************************

struct PLAYER
{
	XMFLOAT3	pos;			// �|���S���̍��W
	XMFLOAT3	energy;			// // �ړ����x
	XMFLOAT3	pos_old;		// �ړ��O�̍��W
	XMFLOAT3	rot;			// �|���S���̉�]��
	BOOL		use;			// true:�g���Ă���  false:���g�p
	BOOL		movable;		// true:����\  false:����s��
	float		w, h;			// ���ƍ���
	float		collisionW, collisionH;			// �����蔻��p�̕��ƍ���
	float		hp;
	float		opacity;

	float		countAnim;		// �A�j���[�V�����J�E���g
	int			patternAnim;	// �A�j���[�V�����p�^�[���i���o�[
	int			pattern_old;	// �ЂƂO�̃A�j���[�V�����p�^�[��
	
	BOOL		left;			// �����itrue:��  false:�E�j
	BOOL		moving;			// �L�[���͂����邩�̃t���O

	BOOL		jump;			// �W�����v�t���O
	BOOL		damage;			// �_���[�W�����̃t���O
	BOOL		attack;			// �U���t���O
	BOOL		ground;			// �n�ʃt���O
	BOOL		sliding;		// �X���C�f�B���O���̃t���O

	float		jumpY;			// �W�����v�̍���
	float		gravityY;		// �d��
	int			updown;			// 0:����~��ł��Ȃ� 1:����~��ł��� 2:����~�蒆
};

struct PLAYERANIM
{
	BOOL		loop;
	float		cntSpeed;		// �A�j���[�V�����J�E���g��i�߂�X�s�[�h
	int			texNo;
	int			xMax;
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

PLAYER* GetPlayer(void);

void DamagePlayer(float damage);
