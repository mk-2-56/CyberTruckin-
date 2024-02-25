//=============================================================================
//
// �X�e�[�^�X���� [status.h]
// Author : GP11A132 15 �����ق̍�
//
//=============================================================================
#pragma once


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define STATUS_SCORE_MAX			(99999)		// �X�R�A�̍ő�l
#define STATUS_SCORE_DIGIT			(5)			// ����
#define STATUS_RETRY_MAX			(99)		// ���g���C�̍ő�l
#define STATUS_RETRY_DIGIT			(2)			// ����
#define STATUS_TIME_MAX				(995999)	// ���Ԃ̍ő�l
#define STATUS_TIME_DIGIT			(2)			// ����
#define TIME_INTERVAL1				(53)	// �����ƕb���̊Ԃ̊Ԋu
#define TIME_INTERVAL2				(115)	// �����ƕb���̊Ԃ̊Ԋu

// �X�e�[�^�X�̎��
enum STATUSOBJ
{
	STATUS_SCORE = 0,		// �X�R�A
	STATUS_RETRY,			// ���g���C��
	STATUS_TIME,			// �o�ߎ���
	STATUS_TIME_MILLI,		// �o�ߎ���(�~���b)
	STATUS_PLAYER,			// �v���C���[�Ɋւ���X�e�[�^�X
	STATUS_PLAYER_HP,		// �v���C���[��HP�Ɋւ���X�e�[�^�X
	STATUS_BOSS,			// �{�X�Ɋւ���X�e�[�^�X
	STATUS_BOSS_HP,			// �{�X��HP�Ɋւ���X�e�[�^�X
	STATUS_OUTSIDE,			// ��ʊO�̃X�e�[�^�X�p�w�i
	STATUS_GAMECLEAR,		// �Q�[���N���A�I
	STATUS_WARNING,			// warning
	STATUS_ALL,
};

// �Q�[���̐i�s�
enum STATUSGAME
{
	STATUSGAME_DURING = 0,	// �i�s��
	STATUSGAME_START,		// �Q�[���X�^�[�g��
	STATUSGAME_CLEAR,		// �Q�[���N���A��
};

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct STATUS
{
	BOOL		use;		// 
	XMFLOAT3	pos;		// �|���S���̍��W
	float		w, h;		// ���ƍ���
	int			texNo;		// �g�p���Ă���e�N�X�`���ԍ�
	int			num;		// �l
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitStatus(void);
void UninitStatus(void);
void UpdateStatus(void);
void DrawStatus(void);

// �X�R�A�֌W
void AddScore(int add);
int GetScore(void);/*
void SetScore(int score);*/

STATUS* GetStatusOBJ(void);

void SetStatus(int status);
int GetStatus(void);

void AddRetry(int add);