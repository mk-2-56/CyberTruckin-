//=============================================================================
//
// �G�l�~�[���� [enemy.cpp]
// Author : GP11A132 15 �����ق̍�
//
//=============================================================================
#include "enemy.h"
#include "bg.h"
#include "player.h"
#include "fade.h"
#include "collision.h"
#include "enemyBullet.h"
#include "sound.h"
#include "effect.h"
#include "field.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_MAX					(4)		// �e�N�X�`���̐�

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iX)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iY)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// �A�j���[�V�����p�^�[����
#define ANIM_WAIT					(4)		// �A�j���[�V�����̐؂�ւ��Wait�l

#define JUMP_COUNT_MAX				(50.0f)	// �ő�W�����v�t���[��
#define JUMP_POWER					(240.0f)	// �W�����v��
#define JUMP_INTERVAL				(50.0f)	// 
#define WALK_SPEED					(-5.0f)
#define COUNT_BOMB					(15.0f)
#define COUNT_SHOT					(200)

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/enemyYellow.png",
	"data/TEXTURE/timerBomb.png",
	"data/TEXTURE/enemyBlue.png",
};

static BOOL				g_Load = FALSE;			// ���������s�������̃t���O
static ENEMY_PATTERN	g_EnemyPatt[ENEMY_PATT_ALL] =
{
	{TRUE, 100.0f, 100.0f, 0, 30, {0, 1, 1, 1.0f, 0, 1, 1, 1.0f}},				// damage, w, h, texNo, hpMax, anime
	{FALSE, 136.5f, 136.0f, 1, 1, {0, 21, 1, 0.18f, 0, 1, 1, 1.0f}},
	{TRUE, 100.0f, 100.0f, 0, 30, {0, 1, 1, 1.0f, 0, 1, 1, 1.0f}},
	{TRUE, 120.0f, 120.0f, 2, 30, {0, 1, 1, 1.0f, 0, 1, 1, 1.0f}},
};

static ENEMY			g_Enemy[ENEMY_MAX] =
{
	{FALSE, XMFLOAT3(2700.0f, 1380.0f, 0.0f), ENEMY_PATT_SHOT},
	{FALSE, XMFLOAT3(3300.0f, 1510.0f, 0.0f), ENEMY_PATT_SHOT_SINGLE},
	{FALSE, XMFLOAT3(3420.0f, 1135.0f + SCREEN_HEIGHT, 0.0f), ENEMY_PATT_BOMB},
	{FALSE, XMFLOAT3(3100.0f, 1430.0f + SCREEN_HEIGHT, 0.0f), ENEMY_PATT_BOMB},
	{FALSE, XMFLOAT3(3650.0f, 1660.0f + SCREEN_HEIGHT, 0.0f), ENEMY_PATT_BOMB},
	{FALSE, XMFLOAT3(6100.0f, 1660.0f, 0.0f), ENEMY_PATT_BOMB},
	{FALSE, XMFLOAT3(6300.0f, 1660.0f, 0.0f), ENEMY_PATT_BOMB},
	{FALSE, XMFLOAT3(6550.0f, 1660.0f, 0.0f), ENEMY_PATT_BOMB},
	{FALSE, XMFLOAT3(7000.0f, 600.0f, 0.0f), ENEMY_PATT_SHOT_SINGLE},
	{FALSE, XMFLOAT3(7700.0f, 600.0f, 0.0f), ENEMY_PATT_SHOT_SINGLE},

};

static int g_jumpCnt;


//=============================================================================
// ����������
//=============================================================================
HRESULT InitEnemy(void)
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

	// �G�l�~�[�\���̂̏�����
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		g_Enemy[i].use = ENEMY_USE_FALSE;
		g_Enemy[i].jump = FALSE;
		g_Enemy[i].wall  = FALSE;
		g_Enemy[i].rot  = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].move = XMFLOAT3(WALK_SPEED, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].countAnim = 0.0f;
		g_Enemy[i].time = 0.0f;
		g_Enemy[i].w = g_EnemyPatt[g_Enemy[i].type].w;
		g_Enemy[i].h = g_EnemyPatt[g_Enemy[i].type].h;
		g_Enemy[i].jumpY = 0.0f;
		g_Enemy[i].texNo = g_EnemyPatt[g_Enemy[i].type].texNo;
		g_Enemy[i].patternAnim = 0;
		g_Enemy[i].hp = g_EnemyPatt[g_Enemy[i].type].hpMax;
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitEnemy(void)
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

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateEnemy(void)
{
	FIELD* field = GetField();
	CHECKPT* checkPt = GetCheckPt();

	int enemyNum = 0;
	for (int i = 0; i < field->checkPtCnt; i++)
	{	// ���݂̃`�F�b�N�|�C���g�܂łɓo��̓G
		enemyNum += checkPt[i].enemyMax;
	}

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_Enemy[i].use == ENEMY_USE_TRUE)	// ���̃G�l�~�[���g���Ă���H
		{										// Yes
			// �n�`�Ƃ̓����蔻��p�ɍ��W�̃o�b�N�A�b�v������Ă���
			XMFLOAT3 pos_old = g_Enemy[i].pos;

			// �A�j���[�V����  
			g_Enemy[i].countAnim += g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].speed;
			if (g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].loop)
			{	// ���[�v����A�j���[�V�����Ȃ�
				if (g_Enemy[i].countAnim > g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].x)
				{
					g_Enemy[i].countAnim = 0.0f;
				}
			}
			else
			{	// ���[�v���Ȃ��A�j���[�V�����Ȃ�
				if (g_Enemy[i].countAnim > (g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].x - 1.0f))
				{
					// �Ō�̊G�ɂ��Ă���
					g_Enemy[i].countAnim = (float)(g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].x) - 1.0f;

				}
			}


			// �G�̃^�C�v�ɉ����čs���p�^�[�����Ⴄ
			switch (g_Enemy[i].type)
			{
			case ENEMY_PATT_SHOT:
				enemyShot(g_Enemy[i].pos, g_Enemy[i].time, BULLET_PATT_SPREAD);
				break;

			case ENEMY_PATT_BOMB:

				if(g_Enemy[i].time == COUNT_BOMB) PlaySound(SOUND_LABEL_SE_bombTimer);

				// �A�j���[�V�������I�������
				if (g_Enemy[i].countAnim == (float)(g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].x) - 1.0f)
				{
					SetEffect(g_Enemy[i].pos, 0, EFFECT_EXPLOSION);				// �����G�t�F�N�g������
					PlaySound(SOUND_LABEL_SE_bomb001);
					g_Enemy[i].use = ENEMY_USE_DONE;
				}
				break;

			case ENEMY_PATT_BACKANDFORTH:
				g_Enemy[i].pos.x += g_Enemy[i].move.x;
				enemyShot(g_Enemy[i].pos, g_Enemy[i].time, BULLET_PATT_SINGLE);

				// �ǂɓ��������������ς���
				if ((GetMap(g_Enemy[i].pos.x, g_Enemy[i].pos.y) > 0) || (g_Enemy[i].pos.x > checkPt[field->checkPtCnt].x) || (g_Enemy[i].pos.x < checkPt[field->checkPtCnt].prevX))
				{
					g_Enemy[i].move.x = 0 - g_Enemy[i].move.x;
				}
				break;

			case ENEMY_PATT_SHOT_SINGLE:
				enemyShot(g_Enemy[i].pos, g_Enemy[i].time, BULLET_PATT_SINGLE);
				break;
			}

			// �ړ����I�������v���C���[�Ƃ̓����蔻��
			if(g_EnemyPatt[g_Enemy[i].type].damage)
			{
				PLAYER* player = GetPlayer();
				for (int j = 0; j < PLAYER_MAX; j++)
				{
					// �����Ă�G�l�~�[�Ɠ����蔻�������
					if (player[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Enemy[i].pos, g_Enemy[i].w, g_Enemy[i].h,
							player[j].pos, player[j].w, player[j].h);

						// �������Ă���H
						if (ans == TRUE)
						{
							// �����������̏���
							DamagePlayer(PLAYER_DAMAGE_MIN);
						}
					}
				}
			}

			if (i < enemyNum)
			{
				// �ʉ߂����`�F�b�N�|�C���g�̓G�͂���FALSE�ɂ��Ă���
				g_Enemy[i].use = FALSE;
			}

			g_Enemy[i].time += 1.0f;
		}
	}

	PLAYER* player = GetPlayer();

	if ((GetMap(player->pos.x, player->pos.y) >= 0) && (GetMap(player->pos.x, player->pos.y) <= (checkPt[field->checkPtCnt].enemyMax - 1)))
	{
		int enemyNum = 0;

		for (int i = 0; i < field->checkPtCnt; i++)
		{
			enemyNum += checkPt[i].enemyMax;
		}
		// �Ή�����No.�̓G���o��������
		SetEnemy(enemyNum + GetMap(player->pos.x, player->pos.y));
	}
	


#ifdef _DEBUG	// �f�o�b�O����\������


#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawEnemy(void)
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

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_Enemy[i].use == ENEMY_USE_TRUE)			// ���̃G�l�~�[���g���Ă���H
		{										// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Enemy[i].texNo]);

			//�G�l�~�[�̈ʒu��e�N�X�`���[���W�𔽉f
			float ex = g_Enemy[i].pos.x - bg->pos.x + OUTSIDE_WIDTH;	// �G�l�~�[�̕\���ʒuX
			float ey = g_Enemy[i].pos.y - bg->pos.y;	// �G�l�~�[�̕\���ʒuY
			float ew = g_Enemy[i].w;		// �G�l�~�[�̕\����
			float eh = g_Enemy[i].h;		// �G�l�~�[�̕\������

			// �A�j���[�V�����p
			float tw = 1.0f / g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].x;	// �e�N�X�`���̕�
			float th = 1.0f / g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].y;	// �e�N�X�`���̍���
			float tx = (float)((int)g_Enemy[i].countAnim % g_EnemyPatt[g_Enemy[i].type].enemyAnime[g_Enemy[i].patternAnim].x) * tw;	// �e�N�X�`���̍���X���W
			float ty = 0;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, ex, ey, ew, eh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Enemy[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);

		}
	}

}


//=============================================================================
// Enemy�\���̂̐擪�A�h���X���擾
//=============================================================================
ENEMY* GetEnemy(void)
{
	return &g_Enemy[0];
}

ENEMY_PATTERN* GetEnemyPatt(void)
{
	return &g_EnemyPatt[0];
}


//=============================================================================
// Enemy�ɑ΂���A�N�V����
//=============================================================================
// �G���o��������
void SetEnemy(int label)
{
	if (g_Enemy[label].use != ENEMY_USE_DONE)
	{
		g_Enemy[label].use = ENEMY_USE_TRUE;
		g_Enemy[label].hp = g_EnemyPatt[g_Enemy[label].type].hpMax;		// HP���^���ɂ�����
	}
}

void DamageEnemy(int num, int damage)			// �ǂ̃^�C�v�̂ǂ̓G�ɉ��_���[�W�^���邩
{
	if (!g_EnemyPatt[g_Enemy[num].type].damage)
	{
		return;
	}

	g_Enemy[num].hp -= damage;
	SetEffect(g_Enemy[num].pos, 0, EFFECT_ENEMYHIT);
	PlaySound(SOUND_LABEL_SE_attack00);			// �����������̉�

	if (g_Enemy[num].hp <= 0)
	{
		// HP���Ȃ��Ȃ�����A��������G�ȊO�͎g�p�ς݂�
		g_Enemy[num].use = (g_Enemy[num].loop) ? ENEMY_USE_REUSE : ENEMY_USE_DONE;
		SetEffect(g_Enemy[num].pos, 0, EFFECT_ENEMYDEAD);								// �|�����Ƃ��̃G�t�F�N�g
	}
}


//=============================================================================
// Enemy�̍s���p�^�[��
//=============================================================================
// ���Ԋu�ŃV���b�g��ł^�C�v
void enemyShot(XMFLOAT3 pos, float time, int type)
{
	if((int)(time) % COUNT_SHOT == 0) SetEnemyBullet(pos, type);
}