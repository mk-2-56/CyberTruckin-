//=============================================================================
//
// �T�E���h���� [sound.h]
// Author : GP11A132 15 �����ق̍�
//
//=============================================================================
#pragma once

#include <windows.h>
#include "xaudio2.h"

//*****************************************************************************
// �T�E���h�t�@�C��
//*****************************************************************************
enum 
{
	SOUND_LABEL_BGM_TITLE,		// BGM0
	SOUND_LABEL_BGM_BOSS,		// BGM1
	SOUND_LABEL_BGM_GAME,		// BGM2
	SOUND_LABEL_BGM_Credit,		// �N���W�b�gBGM
	SOUND_LABEL_BGM_RESULT,		// ���U���gBGM
	SOUND_LABEL_SE_shot00,		// �e���ˉ�
	SOUND_LABEL_SE_attack00,	// �G�ɓ���������
	SOUND_LABEL_SE_damage00,	// �v���C���[���_���[�W�����炤��
	SOUND_LABEL_SE_playerDead00,// �v���C���[�����񂾂Ƃ��̉�
	SOUND_LABEL_SE_bombTimer,	// �������e�̃^�C�}�[
	SOUND_LABEL_SE_bomb001,		// �������e�̔���
	SOUND_LABEL_SE_bossDead,	// �{�X�����S
	SOUND_LABEL_SE_gameClear,	// �Q�[���N���A
	SOUND_LABEL_SE_gameClear2,	// �Q�[���N���A2
	SOUND_LABEL_SE_gameStart,	// �Q�[���X�^�[�g
	SOUND_LABEL_SE_titleClick,	// �X�^�[�g��
	SOUND_LABEL_SE_menuSelect,	// �J�[�\���ړ���
	SOUND_LABEL_SE_warning,		// warning

	SOUND_LABEL_MAX,
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
bool InitSound(HWND hWnd);
void UninitSound(void);
void PlaySound(int label);
void StopSound(int label);
void StopSound(void);

void SetVolumeSound(float volume);
void SetVolumeSound(int label, float volume);

void SetFrequencyRatio(float pitch);
void SetFrequencyRatio(int label, float pitch);