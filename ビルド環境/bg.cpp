//=============================================================================
//
// BG���� [bg.cpp]
// Author : GP11A132 15 �����ق̍�
//
//=============================================================================
#include "bg.h"
#include "player.h"
#include "field.h"
#include "sound.h"
#include "status.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	//
 
#define TEXTURE_MAX					(10)				// �e�N�X�`���̐�

#define BG_SCRL_CNT_MAX				(60)			// �X�N���[���Ɏg���t���[����
#define BG_SCRL_X_ERROR				(20)			// �X�N���[���ɔ���x���̌덷
#define BG_SCRL_Y_ERROR				(15)			// �X�N���[���ɔ���y���̌덷
#define BG_DECO_POS					(820)			// �C���̊Ԋu
#define DOWN_SPEED					(0.1f)			// �V�䂪�~���X�s�[�h
#define DOWN_POS_FIRST				(-575.0f)			// �V�䂪�~��鏉���ʒu

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/bg_Yozora.jpg",
	"data/TEXTURE/bg_underGround.png",
	"data/TEXTURE/bg_underGround2.png",
	"data/TEXTURE/bg_underGround3.png",
	"data/TEXTURE/bg_caution.png",
	"data/TEXTURE/bg_boss.png",
	"data/TEXTURE/lavaFalls2.png",
	"data/TEXTURE/lava.png",
	"data/TEXTURE/ceiling.png",
	"data/TEXTURE/bossBullet.png",
};


static BOOL	g_Load = FALSE;		// ���������s�������̃t���O
static BG	g_BG;
static BGTYPE g_bgType[TEXTURE_MAX] = 
{
	{FALSE, XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f), 0, TEXTURE_HEIGHT, 0, 2.0f},	// use, pos, w, h, texNo, loop, auto, dir, scrlCnt
	{FALSE, XMFLOAT3(0.0f, -SCREEN_HEIGHT, 0.0f), 0, TEXTURE_HEIGHT, 1, 1.0f},
	{FALSE, XMFLOAT3(-GAMESCREEN_WIDTH, 0.0f, 0.0f), 0, TEXTURE_HEIGHT, 2, 1.5f},
	{FALSE, XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f), 0, TEXTURE_HEIGHT, 3, 1.0f},
	{FALSE, XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f), 0, TEXTURE_HEIGHT, 3, 1.0f},
	{FALSE, XMFLOAT3(-GAMESCREEN_WIDTH, 0.0f, 0.0f), 0, TEXTURE_HEIGHT, 4, 1.0f},
	{FALSE, XMFLOAT3(-GAMESCREEN_WIDTH, 0.0f, 0.0f), 0, TEXTURE_HEIGHT, 5, 1.0f},
};

static BGDECO g_bgDeco[BG_DECO_ALL] =
{
	{FALSE, XMFLOAT3(4820.0f, SCREEN_HEIGHT * 2, 0.0f), 128, 960, 0.1f, 6, 0, 1.0f, 3, 1, 2, 3}, // use, pos, w, h, speed, texNo, countAnim, x, y, checkPt, num;
	{FALSE, XMFLOAT3(4700.0f, 1640.0f + SCREEN_HEIGHT, 0.0f), 2368, 96, 0.1f, 7, 0, 1.0f, 3, 1, 2, 1},
	{FALSE, XMFLOAT3(7000.0f - GAMESCREEN_WIDTH, -420.0f, 0.0f), 2460.0f, 1082.0f, 0, 8, 0, 1.0f, 1, 1, 4, 1},
	{FALSE, XMFLOAT3(0.0f, 0.0f, 0.0f), 540.0f, 540.0f, 0, 9, 0, 0.0f, 5, 2, 6, 1},
};
static XMFLOAT3 g_restartPos = XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f);



//=============================================================================
// ����������
//=============================================================================
HRESULT InitBG(void)
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

	CHECKPT* checkPt = GetCheckPt();
	for (int i = 0; i < CHECKPOINT_MAX; i++)
	{
		g_bgType[i].w = checkPt[i].x - checkPt[i].prevX + MAPCHIP_WIDTH;	// �}�b�v�`�b�v�ꖇ�������`�悵�Ă���
		g_bgType[i].h = checkPt[i].y - checkPt[i].prevY;
	}

	// ���g���C�̂��тɃg���b�v�͈ʒu��������
	g_bgDeco[BG_DECO_CEILING].pos.y = DOWN_POS_FIRST;
	g_bgDeco[BG_DECO_BOSSCLIMAX].opacity = 0.0f;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitBG(void)
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
void UpdateBG(void)
{
	FIELD* field = GetField();
	XMFLOAT3 speed = XMFLOAT3(GAMESCREEN_WIDTH / BG_SCRL_CNT_MAX, SCREEN_HEIGHT / BG_SCRL_CNT_MAX, 0.0f);

	// �`�F�b�N�|�C���g�ɍ��킹�Ĕw�i���J�n
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if ((g_bgType[i].use == TRUE) && (i == field->checkPtCnt))
		{
			g_bgType[i].pos.x = g_BG.pos.x - g_restartPos.x;
			g_bgType[i].pos.y = g_BG.pos.y - g_restartPos.y;
		}
	}

	// �`�F�b�N�|�C���g�ɍ��킹�Ĕw�i�̑������J�n
	for (int j = 0; j < BG_DECO_ALL; j++)
	{
		if (g_bgDeco[j].use == TRUE)
		{
			g_bgDeco[j].countAnim+= g_bgDeco[j].speed;
			
			if (g_bgDeco[j].countAnim > (g_bgDeco[j].x * g_bgDeco[j].y)) g_bgDeco[j].countAnim = 0;
			
			if (j == BG_DECO_CEILING)
			{
				g_bgDeco[j].pos.y += DOWN_SPEED;			// �V�䂪����

				PLAYER* player = GetPlayer();
				// �����Ă�G�l�~�[�Ɠ����蔻�������
				if (player[0].use == TRUE)
				{
					BOOL ans = g_bgDeco[j].pos.y + g_bgDeco[j].h > player[0].pos.y - player[0].h / 2;

					// �������Ă���H
					if (ans == TRUE)
					{
						DamagePlayer(PLAYER_HP_MAX);		// ����
					}
				}
			}
		}
	}

	// �����X�N���[����
	if (g_BG.autoScrl)
	{
		PLAYER* player = GetPlayer();

		// �X�N���[�����͑���s��
		player[0].movable = FALSE;

		switch (g_BG.dir)
		{
		case SCRL_DIR_UP:				// ��ɃX�N���[��
			g_BG.pos.y -= speed.y;
			g_bgType[field->checkPtCnt+1].pos.y -= speed.y;
			break;

		case SCRL_DIR_RIGHT:			// �E�ɃX�N���[��
			g_BG.pos.x += speed.x;
			g_bgType[field->checkPtCnt + 1].pos.x += speed.x;
			break;

		case SCRL_DIR_DOWN:				// ���ɃX�N���[��
			g_BG.pos.y += speed.y;
			g_bgType[field->checkPtCnt + 1].pos.y += speed.y;
			break;

		case SCRL_DIR_LEFT:				// ���ɃX�N���[��
			g_BG.pos.x -= speed.x;
			g_bgType[field->checkPtCnt + 1].pos.x += speed.x;
			break;
		}
		g_BG.scrlCnt++;

		if (g_BG.scrlCnt > BG_SCRL_CNT_MAX)
		{
			g_BG.autoScrl = FALSE;
			g_BG.scrlCnt = 0;
			g_bgType[field->checkPtCnt].use = FALSE;		// �g�p�ς�

			for (int i = 0; i < BG_DECO_ALL; i++)
			{
				if (g_bgDeco[i].checkPt == field->checkPtCnt) g_bgDeco[i].use = FALSE;	// �������g�p�ς�
			}
			field->checkPtCnt++;

			g_restartPos.x = g_BG.pos.x;
			g_restartPos.y = g_BG.pos.y;

			// �X�N���[���̌덷�𒲐�
			switch (g_BG.dir)
			{
			case SCRL_DIR_UP:
				g_restartPos.y = g_BG.pos.y + BG_SCRL_Y_ERROR;
				break;

			case SCRL_DIR_RIGHT:
				g_restartPos.x = g_BG.pos.x - BG_SCRL_X_ERROR;
				break;

			case SCRL_DIR_DOWN:
				g_restartPos.y = g_BG.pos.y - BG_SCRL_Y_ERROR;
				break;

			case SCRL_DIR_LEFT:
				g_restartPos.x = g_BG.pos.x + BG_SCRL_X_ERROR;
				break;
			}

			if (field->checkPtCnt == (CHECKPOINT_MAX - 1))
			{
				STATUS* status = GetStatusOBJ();
				StopSound(SOUND_LABEL_BGM_GAME);
				PlaySound(SOUND_LABEL_SE_warning);
				status[STATUS_WARNING].use = TRUE;
			}

			// ����\�ɂ���
			player[0].movable = TRUE;
		}
	}




#ifdef _DEBUG	// �f�o�b�O����\������


#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawBG(void)
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

	for (int i = 0; i < CHECKPOINT_MAX; i++)
	{
		if (g_bgType[i].use == TRUE)
		{
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_bgType[i].texNo]);

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteLTColor(g_VertexBuffer,
				OUTSIDE_WIDTH - g_bgType[i].pos.x, 0 - g_bgType[i].pos.y, g_bgType[i].w, g_bgType[i].h,
				0.0f, 0.0f, g_bgType[i].loop, 1.0f,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

	for (int j = 0; j < BG_DECO_ALL; j++)
	{
		if (g_bgDeco[j].use == TRUE)
		{
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_bgDeco[j].texNo]);

			for (int k = 0; k < g_bgDeco[j].num; k++)
			{
				float dx = g_bgDeco[j].pos.x + BG_DECO_POS * k;
				float tw = 1.0f / g_bgDeco[j].x;	// �e�N�X�`���̕�
				float th = 1.0f / g_bgDeco[j].y;	// �e�N�X�`���̍���
				float tx = (float)((int)g_bgDeco[j].countAnim % g_bgDeco[j].x) * fabsf(tw);	// �e�N�X�`���̍���X���W
				float ty = (float)((int)g_bgDeco[j].countAnim / g_bgDeco[j].x) * th;	// �e�N�X�`���̍���Y���W


				// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
				SetSpriteLTColor(g_VertexBuffer,
					dx - g_BG.pos.x + OUTSIDE_WIDTH, g_bgDeco[j].pos.y - g_BG.pos.y, g_bgDeco[j].w, g_bgDeco[j].h,
					tx, ty, tw, th,
					XMFLOAT4(1.0f, 1.0f, 1.0f, g_bgDeco[j].opacity));

				// �|���S���`��
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}
}


//=============================================================================
// BG�\���̂̐擪�A�h���X���擾
//=============================================================================
BG* GetBG(void)
{
	return &g_BG;
}

//=============================================================================
// BGTYPE�\���̂̐擪�A�h���X���擾
//=============================================================================
BGTYPE* GetBGType(void)
{
	return &g_bgType[0];
}

//=============================================================================
// BGDECO�\���̂̐擪�A�h���X���擾
//=============================================================================
BGDECO* GetBGDeco(void)
{
	return &g_bgDeco[0];
}



//=============================================================================
// BG�̎����X�N���[�����J�n
//=============================================================================
void SetBGScroll(int dir, int num)
{
	if (num == CHECKPOINT_MAX - 1)
	{
		return;
	}

	// �w�肵�������Ɍ������ăX�N���[������
	g_BG.dir = dir;
	g_BG.autoScrl = TRUE;
	g_bgType[num + 1].use = TRUE;
	
	for (int i = 0; i < BG_DECO_ALL; i++)
	{
		if (g_bgDeco[i].checkPt == num + 1) g_bgDeco[i].use = TRUE;
	}
}

//=============================================================================
// BG�̃��Z�b�g
//=============================================================================
void ResetBG(void)
{
	// 2�T�ڂ̃Q�[���̂�BG���ŏ��̈ʒu�Ƀ��Z�b�g
	g_bgType[0].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_bgType[1].pos = XMFLOAT3(0.0f, -SCREEN_HEIGHT, 0.0f);
	g_bgType[2].pos = XMFLOAT3(-GAMESCREEN_WIDTH, 0.0f, 0.0f);
	g_bgType[3].pos = XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f);
	g_bgType[4].pos = XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f);
	g_bgType[5].pos = XMFLOAT3(-GAMESCREEN_WIDTH, 0.0f, 0.0f);
	g_bgType[6].pos = XMFLOAT3(-GAMESCREEN_WIDTH, 0.0f, 0.0f);

	g_restartPos = XMFLOAT3(0.0f, 900.0f, 0.0f);
}

					
					