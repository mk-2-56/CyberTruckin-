//=============================================================================
//
// �G����̃o���b�g���� [enemyBullet.cpp]
// Author : GP11A132 15 �����ق̍�
//
//=============================================================================
#include "enemyBullet.h"
#include "enemy.h"
#include "collision.h"
#include "status.h"
#include "bg.h"
#include "effect.h"
#include "player.h"
#include "field.h"
#include "sound.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_MAX					(3)		// �e�N�X�`���̐�

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iX)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iY)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// �A�j���[�V�����p�^�[����
#define ANIM_WAIT					(4)		// �A�j���[�V�����̐؂�ւ��Wait�l
#define BULLET_HEIGHT_INTERVAL		(60)	// �o���b�g�̍����Ԋu
#define BOSS_CLIMAX_NUM				(4)	// �{�X���ŏI�U���ŏo���e��


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[] = {
	"data/TEXTURE/enemyBullet.png",
	"data/TEXTURE/bossBullet.png",
	"data/TEXTURE/bossBullet2.png",
};

static BOOL		g_Load = FALSE;			// ���������s�������̃t���O
static BULLET_PATTERN	g_BulletPatt[BULLET_PATT_ALL] = {
	{35.0f, 35.0f, 10.0f, 4.0f, 8, 0, 1, 1},						// w, h, damage, speed, max, texNo, x, y
	{100.0f, 100.0f, 10.0f, 12.0f, 2, 1, 5, 2},
	{68.0f, 38.0f, 10.0f, 6.5f, 10, 2, 4, 1},
	{40.0f, 40.0f, 10.0f, 5.0f, 1, 0, 1, 1},
};
static BULLET g_Bullet[BULLET_MAX];
static float g_leftEnd;

//=============================================================================
// ����������
//=============================================================================
HRESULT InitEnemyBullet(void)
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
	for (int i = 0; i < BULLET_MAX; i++)
	{
		g_Bullet[i].use = FALSE;
		g_Bullet[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Bullet[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Bullet[i].move = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Bullet[i].w = 0.0f;
		g_Bullet[i].h = 0.0f;
		g_Bullet[i].damage = 0.0f;
		g_Bullet[i].speed = 0.0f;
		g_Bullet[i].texNo = 0;
		g_Bullet[i].cntAnime = 0;
		g_Bullet[i].type = 0;
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitEnemyBullet(void)
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
void UpdateEnemyBullet(void)
{
	int bulletCount = 0;				// ���������o���b�g�̐�
	FIELD* field = GetField();
	CHECKPT* checkpt = GetCheckPt();
	g_leftEnd = checkpt[field->checkPtCnt].prevX;	// �{�X�̍s�����W�̍��[���Z�b�g

	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Bullet[i].use == TRUE)	// ���̃o���b�g���g���Ă�H
		{								// Yes

			// �o���b�g�̈ړ�����
			XMVECTOR pos = XMLoadFloat3(&g_Bullet[i].pos);
			XMVECTOR move = XMLoadFloat3(&g_Bullet[i].move);
			pos += move;
			XMStoreFloat3(&g_Bullet[i].pos, pos);

			// �A�j���[�V����
			g_Bullet[i].cntAnime++;
			if (g_Bullet[i].cntAnime > g_BulletPatt[g_Bullet[i].type].x * g_BulletPatt[g_Bullet[i].type].y)
			{
				g_Bullet[i].cntAnime = 0;								// ���[�v������
			}


			// �����蔻�菈��
			{
				PLAYER* player = GetPlayer();

				// �����Ă�v���C���[�Ɠ����蔻�������
				if (player[0].use == TRUE)
				{
					XMFLOAT3 playerPos = player[0].pos;
					if (player[i].sliding)
					{
						playerPos = XMFLOAT3(player[0].pos.x, player[0].pos.y + LOSE_COLLISIONH_POS, 0.0f);
					}

					BOOL ans = CollisionBB(g_Bullet[i].pos, g_Bullet[i].w, g_Bullet[i].h,
						player[0].pos, player[0].collisionW, player[0].collisionH);
					// �������Ă���H
					if (ans == TRUE)
					{
						// �����������̏���
						DamagePlayer(g_Bullet[i].damage);
						g_Bullet[i].use = FALSE;
					}
				}

				if ((GetMap(g_Bullet[i].pos.x, g_Bullet[i].pos.y) > MAPCHIP_NOCOLLISION) ||
					((g_Bullet[i].pos.x > checkpt[field->checkPtCnt].x) || (g_Bullet[i].pos.x < checkpt[field->checkPtCnt].prevX)) || 
					((g_Bullet[i].pos.y > checkpt[field->checkPtCnt].y) || (g_Bullet[i].pos.y < checkpt[field->checkPtCnt].prevY)))
				{	// �ǂɓ�����������̓`�F�b�N�|�C���g�𒴂���Ə�����
					g_Bullet[i].use = FALSE;
				}
			}
			bulletCount++;
		}
		else
		{
			g_Bullet[i].move.x = 0;
			g_Bullet[i].move.y = 0;
		}
		
	}


}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawEnemyBullet(void)
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


	for (int i = 0; i < BULLET_MAX; i++)
	{
		if (g_Bullet[i].use == TRUE)	// ���̃o���b�g���g���Ă�H
		{								// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Bullet[i].texNo]);

			//�o���b�g�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Bullet[i].pos.x - bg->pos.x + OUTSIDE_WIDTH;	// �o���b�g�̕\���ʒuX
			float py = g_Bullet[i].pos.y - bg->pos.y;	// �o���b�g�̕\���ʒuY
			float pw = g_Bullet[i].w;		// �o���b�g�̕\����
			float ph = g_Bullet[i].h;		// �o���b�g�̕\������

			float tw = 1.0f / g_BulletPatt[g_Bullet[i].type].x;	// �e�N�X�`���̕�
			float th = 1.0f / g_BulletPatt[g_Bullet[i].type].y;	// �e�N�X�`���̍���
			float tx = (float)(g_Bullet[i].cntAnime % g_BulletPatt[g_Bullet[i].type].x) * tw;	// �e�N�X�`���̍���X���W
			float ty = (float)(g_Bullet[i].cntAnime / g_BulletPatt[g_Bullet[i].type].x) * th;	// �e�N�X�`���̍���Y���W


			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer,
				px, py, pw, ph,
				tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Bullet[i].rot.z, g_Bullet[i].left);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}
}


//=============================================================================
// �o���b�g�̔��ːݒ�
//=============================================================================
void SetEnemyBullet(XMFLOAT3 pos, int type)
{
	int counter = 0;
	int num1 = 0;
	int num2 = 0;
	int fin = rand() % 2;					// ����o���e����x��2����1�����A2����1�̊m��
	int currentPos;
	PLAYER* player = GetPlayer();

	// �������g�p�̒e�����������甭�˂��Ȃ�( =����ȏ㌂�ĂȂ����Ď� )
	for (int i = 0; i < g_BulletPatt[type].max; i++)
	{
		if (g_Bullet[i].use == FALSE)		// ���g�p��Ԃ̃o���b�g��������
		{
			g_Bullet[i].use = TRUE;			// �g�p��Ԃ֕ύX����
			g_Bullet[i].pos = pos;			// ���W���Z�b�g

			// �o���b�g�̃^�C�v���Ƃ̑������Z�b�g
			g_Bullet[i].w = g_BulletPatt[type].w;
			g_Bullet[i].h = g_BulletPatt[type].h;
			g_Bullet[i].damage = g_BulletPatt[type].damage;
			g_Bullet[i].speed = g_BulletPatt[type].speed;
			g_Bullet[i].texNo = g_BulletPatt[type].texNo;
			g_Bullet[i].type = type;

			switch (type)
			{ // �^�C�v���Ƃɓ�����ς���
			case BULLET_PATT_SPREAD:

				g_Bullet[i].move.x = sinf(XM_PI / 4 * i) * g_Bullet[i].speed;	// �������Z�b�g
				g_Bullet[i].move.y = cosf(XM_PI / 4 * i) * g_Bullet[i].speed;	// �������Z�b�g
				break;

			case BULLET_PATT_HORIZONTAL:
				g_Bullet[i].move.x = i == 0 ? g_Bullet[i].speed : g_Bullet[i].speed * -1;
				break;

			case BULLET_PATT_BOSSCLIMAX:
				currentPos = i % BOSS_CLIMAX_NUM;							// ����o���ʒu

				if (pos.x == g_leftEnd)
				{
					g_Bullet[i].move.x = g_Bullet[i].speed;
					g_Bullet[i].left = FALSE;
				}
				else
				{
					g_Bullet[i].move.x = g_Bullet[i].speed * -1;				// �t����
					g_Bullet[i].left = TRUE;
				}
				g_Bullet[i].pos.y = pos.y + (BULLET_HEIGHT_INTERVAL * currentPos);

				if (fin == 0) return;

				if (counter == 0)
				{
					num1 = currentPos;

					// �����ŏ�i�̒e��������
					if (num1 == 0)
					{
						// ���̒e�͐�΂Ɏ��̒i�ɂ���(�����łȂ��Ɣ������Ȃ�)
						g_Bullet[i + 1].pos.y = pos.y + (BULLET_HEIGHT_INTERVAL * (currentPos + 1));
						return;
					}
					counter++;
				}
				else
				{	// 2���ڂɊւ���
					num2 = currentPos;
					
					// �����ŏ�i�̒e��������
					if (num2 == 0)
					{	// 1���ڂ͎��̒i�łȂ��Ƃ����Ȃ��̂ł���ȊO��FALSE�ɂ���
						if (num1 != num2 + 1) g_Bullet[i].use = FALSE;
					}
					return;
				}
				break;

			case BULLET_PATT_SINGLE:
				BOOL left = pos.x > player[0].pos.x;
				g_Bullet[i].move.x = (left) ? g_Bullet[i].speed * -1 : g_Bullet[i].speed;
				break;
			}
		}
	}
}

