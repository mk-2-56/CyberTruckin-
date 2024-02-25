//=============================================================================
//
// �G�t�F�N�g���� [effect.cpp]
// Author : GP11A132 15 �����ق̍�
//
//=============================================================================
#include "effect.h"
#include "bg.h"
#include "player.h"
#include "collision.h"
#include "sound.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define EMISSION_FULL 0			//�p�[�e�B�N���S�����t���O
#define EMISSION_RATE 5			//�p�[�e�B�N���̐����Ԋu(duration/EMISSION_RATE�̐����G�t�F�N�g���o��)

#define EMISSION_WIDTH  50		//�p�[�e�B�N�������͈́i�����j
#define EMISSION_HEIGHT 50		//�p�[�e�B�N�������͈́i�����j
#define EMISSION_PERTIME 0.01f	//�o�ߎ��Ԃ̊


#define TEXTURE_MAX					(10)		// �e�N�X�`���̐�

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void ResetParticle(int i, int n);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char* g_TexturName[] = { 
	"data/TEXTURE/effect_enemyDead.png",
	"data/TEXTURE/effect_enemyHit.png",
	"data/TEXTURE/effect_explosion.png",
	"data/TEXTURE/effect_follow1_red.png",
	"data/TEXTURE/hitEffect01.png",
	"data/TEXTURE/playerDead01.png",
	"data/TEXTURE/attackLights.png",
	"data/TEXTURE/bossExplotion.png",
	"data/TEXTURE/effect_fireGround2.png",
	"data/TEXTURE/effect_sliding.png",
};


static BOOL			g_Load = FALSE;					// ���������s�������̃t���O
static EFFECT		effectWk[EFFECT_NUM_EFFECTS];	// �G�t�F�N�g�\����
static EFFECT_TYPE	effectType[EFFECT_ALL] ={
	{FALSE, 0, 180, 180, 5, 2, 1.0f, 0, 1, 3},		// loop, �����^�C�v, w, h, x������, y������, opacity, texNo, numParts, timeAnim
	{FALSE, 0, 150, 150, 1, 1, 1.0f, 1, 1, 5},
	{FALSE, 0, 600, 600, 3, 3, 1.0f, 2, 1, 3},
	{FALSE, 0, 192, 192, 5, 2, 1.0f, 3, 1, 4},
	{FALSE, 0, 86, 86, 5, 1, 0.8f, 4, 1, 4},
	{FALSE, 1, 360, 360, 10, 1, 0.8f, 5, 1, 8},
	{FALSE, 1, 60, 60, 7, 1, 1.0f, 6, 1, 1},
	{FALSE, 0, 500, 500, 2, 13, 1.0f, 7, 1, 14},
	{TRUE, 0, 146, 97, 1, 5, 1.0f, 8, 1, 4},
	{FALSE, 0, 40, 36, 4, 1, 1.0f, 9, 1, 2},
};

//=============================================================================
// ����������
//=============================================================================
HRESULT InitEffect(void)
{
	ID3D11Device* pDevice = GetDevice();

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


	// ����������
	for (int i = 0; i < EFFECT_NUM_EFFECTS; i++)
	{
		effectWk[i].use = FALSE;
		effectWk[i].elapsed = 0;

		for (int n = 0; n < EFFECT_NUM_PARTS; n++)
		{
			ResetParticle(i, n);
		}
	}


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �p�[�e�B�N���̃��Z�b�g
//=============================================================================
void ResetParticle(int i, int n) 
{
	effectWk[i].pParticle[n].pos = XMFLOAT3(effectWk[i].pos.x, effectWk[i].pos.y, 0.0f);	// ���W�f�[�^��������
	effectWk[i].pParticle[n].move = XMFLOAT3(0.0f, 0.0f, 0.0f);					// �ړ���

	effectWk[i].pParticle[n].PatternAnim = 0;									// �A�j���p�^�[���ԍ��������_���ŏ�����
	effectWk[i].pParticle[n].CountAnim = 0;										// �A�j���J�E���g��������
	effectWk[i].pParticle[n].liveTime = 2000;
	effectWk[i].pParticle[n].isFinish = 0;
	effectWk[i].pParticle[n].time = 0.0f;
	effectWk[i].pParticle[n].reverse = FALSE;
}

//=============================================================================
// �I������
//=============================================================================
void UninitEffect(void)
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
void UpdateEffect(void)
{
	for (int i = 0; i < EFFECT_NUM_EFFECTS; i++)
	{
		if (effectWk[i].use)
		{
			if (effectWk[i].isEnding){
				effectWk[i].use = FALSE;
				continue;
			}

			//�G�~�b�g���L���ł���΃G�t�F�N�g�쐬�������s��
			if (effectWk[i].isRemoveOnFinish == FALSE)
			{

				//�G�t�F�N�g�쐬���[�g�̑�������
				if (effectWk[i].effectCount < effectWk[i].numParts)
					effectWk[i].emitCounter++;

				//�o�b�t�@�ɋ󂫂�����A�ǉ��^�C�~���O�����Ă���ΐV���ȃG�t�F�N�g��ǉ�����
				while ((effectWk[i].effectCount < effectWk[i].numParts) && (effectWk[i].emitCounter > EMISSION_RATE))
				{
					//�S�̒ǉ��t���O��ON�ł���΋󂫗̈�S�ĂɐV���ȃG�t�F�N�g��ǉ�����
					if (EMISSION_FULL)
						effectWk[i].effectCount = effectWk[i].numParts;
					else
						effectWk[i].effectCount++;

					//�G�t�F�N�g�쐬���[�g�̏�����
					effectWk[i].emitCounter = 0;
				}

				effectWk[i].elapsed++;

				//���Ԓ���
				if ((effectWk[i].duration != -1) && (effectWk[i].duration < effectWk[i].elapsed)){
					effectWk[i].isRemoveOnFinish = TRUE;
				}
			}

			int effectIndex = 0;
			int effectPatternNum = effectWk[i].x * effectWk[i].y;

			//�G�t�F�N�g�̍X�V����
			while (effectIndex < effectWk[i].effectCount)
			{
				float vP0 = 1.0f;
				float vP1 = 0.4f;					// ����_
				float vP2 = 0.0f;
				float temp = effectWk[i].pParticle[effectIndex].time;

				PLAYER* player = GetPlayer();

				// �G�t�F�N�g���Ƃɏ�����ς���
				switch (effectWk[i].effectType)
				{
				case EFFECT_EXPLOSION:													// �����G�t�F�N�g
					
					effectWk[i].pParticle[effectIndex].time += EMISSION_PERTIME;

					if (effectWk[i].pParticle[effectIndex].time > 1.0f)
					{
						effectWk[i].pParticle[effectIndex].time = 0.0f;
					}

					// �����G�t�F�N�g�͂��񂾂񓧉߂����ĉ�������
					effectWk[i].pParticle[effectIndex].opacity = QuadraticBezierCurve_single(vP0, vP1, vP2, temp);

					BOOL ans1;
					effectWk[i].collisionW = effectWk[i].w * (effectWk[i].pParticle[effectIndex].time + 0.5f);
					effectWk[i].collisionH = effectWk[i].h * (effectWk[i].pParticle[effectIndex].time + 0.5f);
					ans1 = CollisionBB(XMFLOAT3(effectWk[i].pos.x, effectWk[i].pos.y, 0.0f), effectWk[i].collisionW, effectWk[i].collisionH,
						player[0].pos, player[0].w, player[0].h);

					// �������Ă���H
					if (ans1 == TRUE)
					{
						// �����������̏���
						DamagePlayer(20.0f);
					}
					break;

				case EFFECT_BOSS_FOLLOW:												// �{�X�̒Ǐ]�G�t�F�N�g

					for (int j = 0; j < PLAYER_MAX; j++)
					{
						if (player[j].use == TRUE)
						{
							float heightAngle = (XM_PI / (effectWk[i].x * effectWk[i].y)) * (effectWk[i].pParticle[effectIndex].PatternAnim + 1);
							float h = effectType[EFFECT_BOSS_FOLLOW].h / 3 * cosf(XM_PI / 2 - heightAngle);
							effectWk[i].collisionH = h;

							BOOL ans2;
							ans2 = CollisionBB(XMFLOAT3(effectWk[i].pos.x, effectWk[i].pos.y + effectWk[i].h / 3, 0.0f), effectWk[i].collisionW, effectWk[i].collisionH,
								player[j].pos, player[j].w, player[j].h);

							// �������Ă���H
							if (ans2 == TRUE)
							{
								// �����������̏���
								DamagePlayer(10.0f);
							}
						}
					}
					break;

				case EFFECT_PLAYERATTACK:

					// �v���C���[�̏e�̈ʒu�ɒǏ]����
					if (player[0].left)
					{
						effectWk[i].pParticle[effectIndex].pos = XMFLOAT3(player[0].pos.x - player[0].w / 2, player[0].pos.y - 15, 0.0f);
					}
					else
					{
						effectWk[i].pParticle[effectIndex].pos = XMFLOAT3(player[0].pos.x + player[0].w / 2, player[0].pos.y - 15, 0.0f);
					}
					break;
				}

				// �o�ߎ���
				if (effectWk[i].pParticle[effectIndex].liveTime > 0)
				{
					//�����t���[������
					effectWk[i].pParticle[effectIndex].liveTime--;

					//�A�j���p�^�[���i�s
					if (++effectWk[i].pParticle[effectIndex].CountAnim > effectWk[i].timeAnim) {
						//�A�j���p�^�[�����ő�l�ɒB�����ꍇ�ł��I��
						if (++effectWk[i].pParticle[effectIndex].PatternAnim >= (effectPatternNum - 1))
						{
							if (!effectWk[i].loop)
							{
								effectWk[i].pParticle[effectIndex].PatternAnim = effectPatternNum - 1;
								effectWk[i].pParticle[effectIndex].liveTime = 0;
							}
						}

						effectWk[i].pParticle[effectIndex].CountAnim = 0;
					}

					effectIndex++;

				}
				else{
					if (effectWk[i].isRemoveOnFinish){
						if (effectWk[i].pParticle[effectIndex].isFinish == 0)
						{
							effectWk[i].pParticle[effectIndex].isFinish = 1;
							effectWk[i].numFinish++;
						}
						//�I������
						if (effectWk[i].numFinish == effectWk[i].effectCount)
						{
							effectWk[i].isEnding = TRUE;
							break;
						}
						effectIndex++;
					}
					else{
						//�o�b�t�@������������
						ResetParticle(i, effectIndex);

						//�����łȂ���΃C���f�b�N�X���l�߂�
						if (effectIndex != (effectWk[i].effectCount - 1))
						{
							effectWk[i].pParticle[effectIndex] = effectWk[i].pParticle[effectWk[i].effectCount - 1];
							ResetParticle(i, (effectWk[i].effectCount - 1));
						}
						effectWk[i].effectCount--;
					}
				}
			}
		}
	}
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawEffect(void)
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
	PLAYER* player = GetPlayer();

	for (int i = 0; i < EFFECT_NUM_EFFECTS; i++)
	{
		if (effectWk[i].use == TRUE)		// ���̃G�t�F�N�g���g���Ă���H
		{									// Yes

			// �����^�C�v��ς���
			if (effectType[effectWk[i].effectType].synthesis) SetBlendState(BLEND_MODE_ADD);

			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[effectWk[i].texNo]);

			for (int n = 0; n < effectWk[i].effectCount; n++)
			{
				if (effectWk[i].pParticle[n].isFinish == 0) {
					//�o���b�g�̈ʒu��e�N�X�`���[���W�𔽉f
					float px = effectWk[i].pParticle[n].pos.x - bg->pos.x + OUTSIDE_WIDTH;	// �G�t�F�N�g�̕\���ʒuX
					float py = effectWk[i].pParticle[n].pos.y - bg->pos.y;	// �G�t�F�N�g�̕\���ʒuY
					float pw = effectWk[i].w;			// �G�t�F�N�g�̕\����
					float ph = effectWk[i].h;			// �G�t�F�N�g�̕\������

					// px -= effectWk[i].w / effectWk[i].x;
					// py -= effectWk[i].h / effectWk[i].y;

					float tw = 1.0f / effectWk[i].x;	// �e�N�X�`���̕�
					float th = 1.0f / effectWk[i].y;	// �e�N�X�`���̍���
					float tx = (float)(effectWk[i].pParticle[n].PatternAnim % effectWk[i].x) * tw;	// �e�N�X�`���̍���X���W
					float ty = (float)(effectWk[i].pParticle[n].PatternAnim / effectWk[i].x) * th;	// �e�N�X�`���̍���Y���W

					// �v���C���[�̌����ɂ���ăe�N�X�`���𔽓]������G�t�F�N�g�̏ꍇ
					if ((effectWk[i].effectType == EFFECT_PLAYERATTACK) || effectWk[i].effectType == EFFECT_SLIDING)
					{
						effectWk[i].pParticle[n].reverse = player[0].left;
					}

					// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
					SetSpriteColorRotation(g_VertexBuffer,
						px, py, pw, ph,
						tx, ty, tw, th,
						XMFLOAT4(1.0f, 1.0f, 1.0f, effectWk[i].pParticle[n].opacity), 0.0f, effectWk[i].pParticle[n].reverse);

					// �|���S���`��
					GetDeviceContext()->Draw(4, 0);
				}
			}

			SetBlendState(BLEND_MODE_ALPHABLEND);	// �����^�C�v�����ɖ߂�

		}
	}

}


//=============================================================================
// �G�t�F�N�g�\���̂̐擪�A�h���X���擾
//=============================================================================
EFFECT *GetEffect(void)
{
	return &effectWk[0];
}


//=============================================================================
// �G�t�F�N�g�̃Z�b�g
// 
// int duration		�G�t�F�N�g�������̐�������
//=============================================================================
void SetEffect(XMFLOAT3 pos, int duration, int label)
{
	// �������g�p�̃G�t�F�N�g��������������s���Ȃ�( =����ȏ�\���ł��Ȃ����Ď� )
	for (int i = 0; i < EFFECT_NUM_EFFECTS; i++)
	{
		if (effectWk[i].use == FALSE)		// ���g�p��Ԃ̃G�t�F�N�g��������
		{
			effectWk[i].use = TRUE;
			effectWk[i].isEnding = FALSE;
			effectWk[i].isRemoveOnFinish = FALSE;

			effectWk[i].duration = duration;
			effectWk[i].pos = pos;							// ���W���Z�b�g

			effectWk[i].effectCount = 0;
			effectWk[i].elapsed = 0;
			effectWk[i].emitCounter = EMISSION_RATE;
			effectWk[i].numFinish = 0;

			effectWk[i].w = effectType[label].w;			// �I�񂾃G�t�F�N�g�^�C�v�ɑ΂�������Z�b�g
			effectWk[i].h = effectType[label].h;
			effectWk[i].collisionW = (float)(effectType[label].w * 0.7);			// �I�񂾃G�t�F�N�g�^�C�v�ɑ΂�������Z�b�g
			effectWk[i].collisionH = effectType[label].h;
			effectWk[i].x = effectType[label].x;
			effectWk[i].y = effectType[label].y;
			effectWk[i].texNo = effectType[label].texNo;
			effectWk[i].numParts = effectType[label].numParts;
			effectWk[i].timeAnim = effectType[label].timeAnim;
			effectWk[i].loop = effectType[label].loop;
			effectWk[i].effectType = label;

			//�p�[�e�B�N���̏�����
			for (int n = 0; n < effectWk[i].numParts; n++)
			{
				ResetParticle(i, n);
				effectWk[i].pParticle[n].opacity = effectType[label].opacity;
			}

			return;							// 1�Z�b�g�����̂ŏI������
		}
	}
}
