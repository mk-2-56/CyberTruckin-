//=============================================================================
//
// �v���C���[����̃o���b�g���� [playerBullet.h]
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
#define BULLET_PLAYER_MAX				(3)	// �o���b�g�ő吔


// �o���b�g�\����
struct PLAYER_BULLET
{
	BOOL				use;				// true:�g���Ă���  false:���g�p
	XMFLOAT3			pos;				// �o���b�g�̍��W
	float				w, h;				// ���ƍ���
	XMFLOAT3			rot;				// �o���b�g�̉�]��
	XMFLOAT3			move;				// �o���b�g�̈ړ���
	int					texNo;				// �ǂ̃e�N�X�`�����g����
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitPlayerBullet(void);
void UninitPlayerBullet(void);
void UpdatePlayerBullet(void);
void DrawPlayerBullet(void);

PLAYER_BULLET* GetPlayerBullet(void);

void SetPlayerBullet(XMFLOAT3 pos);
