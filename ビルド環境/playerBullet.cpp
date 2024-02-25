//=============================================================================
//
// �v���C���[����̃o���b�g���� [playerBullet.cpp]
// Author : GP11A132 15 �����ق̍�
//
//=============================================================================
#include "enemy.h"
#include "collision.h"
#include "playerBullet.h"
#include "bg.h"
#include "effect.h"
#include "player.h"
#include "field.h"
#include "sound.h"
#include "status.h"
#include "boss.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(25)	// �o���b�g�T�C�Y
#define TEXTURE_HEIGHT				(25)	// 
#define TEXTURE_MAX					(1)		// �e�N�X�`���̐�

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iX)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iY)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// �A�j���[�V�����p�^�[����
#define ANIM_WAIT					(4)		// �A�j���[�V�����̐؂�ւ��Wait�l
#define	RIGHT_ANGLE					(3)		// ���˕Ԃ�̊p�x
#define	LEFT_ANGLE					(5)
#define	ENEMY_DAMAGE				(10)


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[] = {
	"data/TEXTURE/bullet01.png",
};

static BOOL		g_Load = FALSE;			// ���������s�������̃t���O
static float	g_Speed = 16.0f;
static PLAYER_BULLET	g_Bullet[BULLET_PLAYER_MAX];

//=============================================================================
// ����������
//=============================================================================
HRESULT InitPlayerBullet(void)
{
	ID3D11Device *pDevice = GetDevice();

	//�e�N�X�`������
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// �o���b�g�\���̂̏�����
	for (int i = 0; i < BULLET_PLAYER_MAX; i++)
	{
		g_Bullet[i].use = FALSE;
		g_Bullet[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Bullet[i].w = TEXTURE_WIDTH;
		g_Bullet[i].h = TEXTURE_HEIGHT;
		g_Bullet[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Bullet[i].move = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Bullet[i].texNo = 0;
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitPlayerBullet(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

}

//=============================================================================
// �X�V����
//=============================================================================
void UpdatePlayerBullet(void)
{
	int bulletCount = 0;				// ���������o���b�g�̐�
	FIELD* field = GetField();
	CHECKPT* checkpt = GetCheckPt();

	for (int i = 0; i < BULLET_PLAYER_MAX; i++)
	{
		if (g_Bullet[i].use == TRUE)	// ���̃o���b�g���g���Ă���H
		{								// Yes

			// �o���b�g�̈ړ�����
			XMVECTOR pos = XMLoadFloat3(&g_Bullet[i].pos);
			XMVECTOR move = XMLoadFloat3(&g_Bullet[i].move);
			pos += move;
			XMStoreFloat3(&g_Bullet[i].pos, pos);

			if (GetMap(g_Bullet[i].pos.x, g_Bullet[i].pos.y) > MAPCHIP_NOCOLLISION)
			{	// �ǂɓ�����ƃG�t�F�N�g�t���ŏ�����
				g_Bullet[i].use = false;
				SetEffect(g_Bullet[i].pos, 0, EFFECT_HIT_WALL);
			}
			else if ((g_Bullet[i].pos.x > checkpt[field->checkPtCnt].x) || (g_Bullet[i].pos.x < checkpt[field->checkPtCnt].prevX))
			{	// �`�F�b�N�|�C���g�𒴂���Ƃ������������
				g_Bullet[i].use = false;
			}

			// �����蔻�菈��
			{
				ENEMY* enemy = GetEnemy();
				ENEMY_PATTERN* enemyPatt = GetEnemyPatt();
				BOSS* boss = GetBoss();

				// �G�l�~�[�̐��������蔻����s��
				for (int j = 0; j < ENEMY_MAX; j++)
				{
					// �����Ă�G�l�~�[�Ɠ����蔻�������
					if (enemy[j].use == ENEMY_USE_TRUE)
					{
						BOOL ans = CollisionBB(g_Bullet[i].pos, g_Bullet[i].w, g_Bullet[i].h,
							enemy[j].pos, enemy[j].w, enemy[j].h);

						// �������Ă���H
						if ((ans == TRUE) && (enemyPatt[enemy[j].type].damage))		// �_���[�W��^���Ă���^�C�v�̓G��������
						{
							// �����������̏���
							DamageEnemy(j, ENEMY_DAMAGE);
							g_Bullet[i].use = FALSE;
							AddScore(100);
						}
					}
				}

				// �{�X�̐��������蔻����s��
				for (int k = 0; k < BOSS_MAX; k++)
				{
					// �����Ă�{�X�Ɠ����蔻�������
					if (boss[k].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Bullet[i].pos, g_Bullet[i].w, g_Bullet[i].h,
							boss[k].pos, boss[k].w, boss[k].h);

						// �������Ă���H
						if (ans == TRUE)
						{
							if (boss[k].damage == TRUE)
							{
								// �����������̏���
								DamageBoss(PLAYER_DAMAGE_MIN);
								SetEffect(g_Bullet[i].pos, 0, EFFECT_HIT_WALL);
								g_Bullet[i].use = FALSE;
							}
							else if(g_Bullet[i].pos.x > boss[k].pos.x)
							{
								g_Bullet[i].move.x = sinf(XM_PIDIV4 * RIGHT_ANGLE) * g_Speed;	// �E�ɂ͂˂�����
								g_Bullet[i].move.y = cosf(XM_PIDIV4 * RIGHT_ANGLE) * g_Speed;
							}
							else
							{
								g_Bullet[i].move.x = sinf(XM_PIDIV4 * LEFT_ANGLE) * g_Speed;	// ���ɂ͂˂�����
								g_Bullet[i].move.y = cosf(XM_PIDIV4 * LEFT_ANGLE) * g_Speed;
							}
						}
					}
				}

			}
			bulletCount++;
		}

		
	}


}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawPlayerBullet(void)
{
	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �}�g���N�X�ݒ�
	SetWorldViewProjection2D();

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	BG* bg = GetBG();

	for (int i = 0; i < BULLET_PLAYER_MAX; i++)
	{
		if (g_Bullet[i].use == TRUE)		// ���̃o���b�g���g���Ă���H
		{									// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Bullet[i].texNo]);

			//�o���b�g�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Bullet[i].pos.x - bg->pos.x + OUTSIDE_WIDTH;	// �o���b�g�̕\���ʒuX
			float py = g_Bullet[i].pos.y - bg->pos.y;	// �o���b�g�̕\���ʒuY
			float pw = g_Bullet[i].w;		// �o���b�g�̕\����
			float ph = g_Bullet[i].h;		// �o���b�g�̕\������

			float tw = 1.0f;	// �e�N�X�`���̕�
			float th = 1.0f;	// �e�N�X�`���̍���
			float tx = 0.0f;	// �e�N�X�`���̍���X���W
			float ty = 0.0f;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, 
				px, py, pw, ph, 
				tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Bullet[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

}


//=============================================================================
// �o���b�g�\���̂̐擪�A�h���X���擾
//=============================================================================
PLAYER_BULLET *GetPlayerBullet(void)
{
	return &g_Bullet[0];
}


//=============================================================================
// �o���b�g�̔��ːݒ�
//=============================================================================
void SetPlayerBullet(XMFLOAT3 pos)
{
	// �������g�p�̒e�����������甭�˂��Ȃ�( =����ȏ㌂�ĂȂ����Ď� )
	for (int i = 0; i < BULLET_PLAYER_MAX; i++)
	{
		if (g_Bullet[i].use == FALSE)							// ���g�p��Ԃ̃o���b�g��������
		{
			g_Bullet[i].use = TRUE;								// �g�p��Ԃ֕ύX����
			PlaySound(SOUND_LABEL_SE_shot00);
			g_Bullet[i].move.y = 0;

			PLAYER* player = GetPlayer();
			if (player[0].left)
			{
				g_Bullet[i].move.x = 0.0f - g_Speed;
				g_Bullet[i].pos = XMFLOAT3(pos.x - player[0].w / 2, pos.y - 15, 0.0f);
				SetEffect(g_Bullet[i].pos, 0, EFFECT_PLAYERATTACK);
			}
			else
			{
				g_Bullet[i].move.x = g_Speed;
				g_Bullet[i].pos = XMFLOAT3(pos.x + player[0].w / 2, pos.y - 15, 0.0f);
				SetEffect(g_Bullet[i].pos, 0, EFFECT_PLAYERATTACK);
			}
			return;												// 1���Z�b�g�����̂ŏI������
		}

	}
}

