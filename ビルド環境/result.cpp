//=============================================================================
//
// ���U���g��ʏ��� [result.cpp]
// Author : GP11A132 15 �����ق̍�
//
//=============================================================================
#include "result.h"
#include "input.h"
#include "status.h"
#include "fade.h"
#include "field.h"
#include "bg.h"
#include "guide.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// �w�i�T�C�Y
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)						// 
#define TEXTURE_MAX					(2)									// �e�N�X�`���̐�
#define STATUS_POINT_DIGIT			(4)									// �{�[�i�X�|�C���g�̌���
#define PENALTY_PER_RETRY			(100)								// ���g���C1�񂲂Ƃ̃y�i���e�B
#define BONUS_PER_TIME				(1000)								// �^�C���{�[�i�X�̒ǉ���l
#define STANDARD_TIME				(5)									// �{�[�i�X�̊

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
RESULT g_result[RESULTTYPE_ALL]
{
	{XMFLOAT3(810.0f, 318, 0.0f), 67, 106, 0},		// pos, w, h, num
	{XMFLOAT3(860.0f, SCREEN_HEIGHT - 276, 0.0f), 28.8f, 45.6f, 0},
	{XMFLOAT3(924.0f, SCREEN_HEIGHT - 175, 0.0f), 28.8f, 45.6f, 0},
	{XMFLOAT3(1316.0f, SCREEN_HEIGHT - 358, 0.0f), 24, 38, 0},
	{XMFLOAT3(1325.0f, SCREEN_HEIGHT - 352, 0.0f), 24, 38, 0},
	{XMFLOAT3(1350.0f, SCREEN_HEIGHT - 202, 0.0f), 24, 38, 0},
};

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/board_result.png",
	"data/TEXTURE/fontDegital_white.png",
};

static BOOL						g_Load = FALSE;
static int						g_time = 0;
static int						g_score = 0;

//=============================================================================
// ����������
//=============================================================================
HRESULT InitResult(void)
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

	STATUS* status = GetStatusOBJ();
	g_score = status[STATUS_SCORE].num;
	g_time = status[STATUS_TIME].num;
	g_result[RESULTTYPE_RETRY].num = status[STATUS_RETRY].num;
	g_result[RESULTTYPE_BONUS].num = 0;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitResult(void)
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
void UpdateResult(void)
{
	if (GetKeyboardTrigger(DIK_RETURN))
	{// Enter��������A�X�e�[�W��؂�ւ���
		SetFade(FADE_OUT, MODE_TITLE);
		ResetGame();
	}
	// �Q�[���p�b�h�œ��͏���
	else if (IsButtonTriggered(0, BUTTON_START))
	{
		SetFade(FADE_OUT, MODE_TITLE);
		ResetGame();
	}
	else if (IsButtonTriggered(0, BUTTON_B))
	{
		SetFade(FADE_OUT, MODE_TITLE);
		ResetGame();
	}

	g_result[RESULTTYPE_PENALTY].num = g_result[RESULTTYPE_RETRY].num * PENALTY_PER_RETRY;
	g_result[RESULTTYPE_SCORE].num = g_score + g_result[RESULTTYPE_BONUS].num - g_result[RESULTTYPE_PENALTY].num;

	// windows����擾�������Ԃ𕪐��ւƕϊ�
	int min = g_time / 10 / 6000;
	if (min < STANDARD_TIME)
	{
		g_result[RESULTTYPE_BONUS].num = BONUS_PER_TIME;
	}
	else if (min < STANDARD_TIME * 2)
	{
		g_result[RESULTTYPE_BONUS].num = BONUS_PER_TIME / 2;
	}

#ifdef _DEBUG	// �f�o�b�O����\������
	
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawResult(void)
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

	// ���U���g�̔w�i��`��
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSpriteLeftTop(g_VertexBuffer, 0.0f, 0.0f, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}



	// ���낢��Ȑ��l�\��
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		int numTime = g_time / 10;
		int sec = (numTime / 100) % 60;
		int min = numTime / 6000;

		// �X�R�A�̈ʒu��e�N�X�`���[���W�𔽉f
		float sy = g_result[RESULTTYPE_TIME].pos.y;									// �X�R�A�̕\���ʒuY
		float sw = g_result[RESULTTYPE_TIME_MILLI].w;										// �X�R�A�̕\����
		float sh = g_result[RESULTTYPE_TIME_MILLI].h;										// �X�R�A�̕\������

		float tw = 1.0f / 10;		// �e�N�X�`���̕�
		float th = 1.0f / 1;		// �e�N�X�`���̍���
		float ty = 0.0f;			// �e�N�X�`���̍���Y���W

		// �~���b����
		for (int i = 0; i < STATUS_TIME_DIGIT; i++)
		{
			// ����\�����錅�̐���
			float x = (float)(numTime % 10);

			float sx = g_result[RESULTTYPE_TIME_MILLI].pos.x - g_result[RESULTTYPE_TIME_MILLI].w * i;		// �X�R�A�̕\���ʒuX
			float sw = g_result[RESULTTYPE_TIME_MILLI].w;										// �X�R�A�̕\����
			float sh = g_result[RESULTTYPE_TIME_MILLI].h;										// �X�R�A�̕\������
			float tx = x * tw;																// �e�N�X�`���̍���X���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColor(g_VertexBuffer, sx - 2 * i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);

			// ���̌���
			numTime /= 10;
		}

		// �b������
		for (int i = 0; i < STATUS_TIME_DIGIT; i++)
		{
			// ����\�����錅�̐���
			float x = (float)(sec % 10);

			float sx = g_result[RESULTTYPE_TIME].pos.x - g_result[RESULTTYPE_TIME].w * i;		// �X�R�A�̕\���ʒuX
			float tx = x * tw;															// �e�N�X�`���̍���X���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColor(g_VertexBuffer, sx - TIME_INTERVAL1 - 2 * i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);

			// ���̌���
			sec /= 10;
		}

		// ��������
		for (int i = 0; i < STATUS_TIME_DIGIT; i++)
		{
			// ����\�����錅�̐���
			float x = (float)(min % 10);

			float sx = g_result[RESULTTYPE_TIME].pos.x - g_result[RESULTTYPE_TIME].w * i;		// �X�R�A�̕\���ʒuX
			float tx = x * tw;															// �e�N�X�`���̍���X���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColor(g_VertexBuffer, sx - TIME_INTERVAL2 - 2 * i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);

			// ���̌���
			min /= 10;
		}

		// �^�C���{�[�i�X��\��
		int bonus = g_result[RESULTTYPE_BONUS].num;
		for (int i = 0; i < STATUS_POINT_DIGIT; i++)
		{
			// ����\�����錅�̐���
			float x = (float)(bonus % 10);

			float sx = g_result[RESULTTYPE_BONUS].pos.x - g_result[RESULTTYPE_BONUS].w * i;		// �X�R�A�̕\���ʒuX
			float sy = g_result[RESULTTYPE_BONUS].pos.y;										// �X�R�A�̕\���ʒuY
			float sw = g_result[RESULTTYPE_BONUS].w;											// �X�R�A�̕\����
			float sh = g_result[RESULTTYPE_BONUS].h;											// �X�R�A�̕\������
			float tx = x * tw;																	// �e�N�X�`���̍���X���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColor(g_VertexBuffer, sx - 2 * i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);

			// ���̌���
			bonus /= 10;
		}

		// �g�[�^���X�R�A��\��
		int total = g_result[RESULTTYPE_SCORE].num;
		for (int i = 0; i < STATUS_SCORE_DIGIT; i++)
		{
			// ����\�����錅�̐���
			float x = (float)(total % 10);

			float sx = g_result[RESULTTYPE_SCORE].pos.x - g_result[RESULTTYPE_SCORE].w * i;		// �X�R�A�̕\���ʒuX
			float sy = g_result[RESULTTYPE_SCORE].pos.y;										// �X�R�A�̕\���ʒuY
			float sw = g_result[RESULTTYPE_SCORE].w;											// �X�R�A�̕\����
			float sh = g_result[RESULTTYPE_SCORE].h;											// �X�R�A�̕\������
			float tx = x * tw;																	// �e�N�X�`���̍���X���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColor(g_VertexBuffer, sx - 2 * i, sy, sw, sh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);

			// ���̌���
			total /= 10;
		}

		// ���g���C�Ɋւ���X�R�A
		{
			int numRetry = g_result[RESULTTYPE_RETRY].num;
			for (int i = 0; i < STATUS_RETRY_DIGIT; i++)
			{
				// ����\�����錅�̐���
				float x = (float)(numRetry % 10);

				// �X�R�A�̈ʒu��e�N�X�`���[���W�𔽉f
				float sx = g_result[RESULTTYPE_RETRY].pos.x - g_result[RESULTTYPE_RETRY].w * i;		// �X�R�A�̕\���ʒuX
				float sy = g_result[RESULTTYPE_RETRY].pos.y;										// �X�R�A�̕\���ʒuY
				float sw = g_result[RESULTTYPE_RETRY].w;											// �X�R�A�̕\����
				float sh = g_result[RESULTTYPE_RETRY].h;											// �X�R�A�̕\������
				float tx = x * tw;																	// �e�N�X�`���̍���X���W

				// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
				SetSpriteColor(g_VertexBuffer, sx - 2 * i, sy, sw, sh, tx, ty, tw, th,
					XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

				// �|���S���`��
				GetDeviceContext()->Draw(4, 0);

				// ���̌���
				numRetry /= 10;
			}

			// �y�i���e�B�̒l��\��
			int penalty = g_result[RESULTTYPE_PENALTY].num;
			for (int i = 0; i < STATUS_POINT_DIGIT; i++)
			{
				// ����\�����錅�̐���
				float x = (float)(penalty % 10);

				float sx = g_result[RESULTTYPE_PENALTY].pos.x - g_result[RESULTTYPE_PENALTY].w * i;		// �X�R�A�̕\���ʒuX
				float sy = g_result[RESULTTYPE_PENALTY].pos.y;											// �X�R�A�̕\���ʒuY
				float sw = g_result[RESULTTYPE_PENALTY].w;												// �X�R�A�̕\����
				float sh = g_result[RESULTTYPE_PENALTY].h;												// �X�R�A�̕\������
				float tx = x * tw;																		// �e�N�X�`���̍���X���W

				// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
				SetSpriteColor(g_VertexBuffer, sx - 2 * i, sy, sw, sh, tx, ty, tw, th,
					XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

				// �|���S���`��
				GetDeviceContext()->Draw(4, 0);

				// ���̌���
				penalty /= 10;
			}
		}

	}
}



//=============================================================================
// �Q�[���̃��Z�b�g
//=============================================================================
void ResetGame(void)
{
	STATUS* status = GetStatusOBJ();
	status[STATUS_SCORE].num = 0;
	status[STATUS_RETRY].num = 0;

	FIELD* field = GetField();
	field->checkPtCnt = 0;

	ResetGuideCount();
	ResetBG();
}