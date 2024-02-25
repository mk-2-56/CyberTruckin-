//=============================================================================
//
// �v���C���[���� [player.cpp]
// Author : GP11A132 15 �����ق̍� 
//
//=============================================================================
#include "player.h"
#include "input.h"
#include "bg.h"
#include "playerBullet.h"
#include "enemy.h"
#include "collision.h"
#include "status.h"
#include "field.h"
#include "sound.h"
#include "effect.h"
#include "fade.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(99)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(121)	// 
#define TEXTURE_MAX					(11)	// �e�N�X�`���̐�

// #define TEXTURE_PATTERN_DIVIDE_X	(3)		// �A�j���p�^�[���̃e�N�X�`�����������iX)
// #define TEXTURE_PATTERN_DIVIDE_Y	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iY)
// #define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// �A�j���[�V�����p�^�[����
// #define ANIM_WAIT					(4)		// �A�j���[�V�����̐؂�ւ��Wait�l

// �v���C���[�̉�ʓ��z�u���W
#define PLAYER_DISP_X				(GAMESCREEN_WIDTH/2)
#define PLAYER_DISP_Y				(SCREEN_HEIGHT/2 + TEXTURE_HEIGHT)

#define	PLAYER_DUSH_SPEED			(6.0f)	// �ړ��X�s�[�h
#define	PLAYER_SLIDING_SPEED		(0.8f)	// �X���C�f�B���O�X�s�[�h
#define	PLAYER_SLIDING_MAX			(3.0f)	// �X���C�f�B���OMAX

// �W�����v����
#define	PLAYER_JUMP_CNT_FIRST		(20.0f)	// �㏸����t���[�����̏����l
#define	PLAYER_JUMP_POWER_FIRST		(10.0f)	// �W�����v�͂̏����l
#define	PLAYER_JUMP_POWER_MAX		(23.0f)	// �W�����v�͂̍ő�l

#define PLAYER_ATTACK_ACTION		(20.0f) // �U�����[�V�����̎���
#define PLAYER_SLIDING_ACTION		(15.0f) // �X���C�f�B���O���[�V�����̎���
#define	PLAYER_GRAVITY				(2.0f)		// �d��
#define	PLAYER_GRAVITY_MAX			(10.0f)		// �ő�~����

#define	PLAYER_COLLISION_DAMAGE		(10.0f)	// �G�ƂԂ������Ƃ��̃_���[�W
#define	PLAYER_DAMAGE_INTERVAL		(52.0f)	// ���G����(��x�_���[�W��������Ă��玟�ɂ��炤�܂ł̊Ԋu)�̃t���[����
#define LOSE_COLLISIONH				(0.7f)	// �X���C�f�B���O�Ō��炷����


//*****************************************************************************
// 
// �v���g�^�C�v�錾
//*****************************************************************************
// void DrawPlayerOffset(int no);


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/normalPlayer.png",
	"data/TEXTURE/jumpPlayer.png",
	"data/TEXTURE/walkPlayer.png",
	"data/TEXTURE/hashigoPlayer.png",
	"data/TEXTURE/attackPlayer.png",
	"data/TEXTURE/attackRunPlayer.png",
	"data/TEXTURE/damagePlayer.png",
	"data/TEXTURE/downPlayer.png",
	"data/TEXTURE/crisisPlayer.png",
	"data/TEXTURE/slidingPlayer.png",
};


static BOOL		g_Load = FALSE;							// ���������s�������̃t���O
static PLAYER	g_Player[PLAYER_MAX];					// �v���C���[�\����

static float    g_jumpCntMax = PLAYER_JUMP_CNT_FIRST;	// �W�����v�J�E���g
static float    g_jumpPower = PLAYER_JUMP_POWER_FIRST;	// �W�����v��
static float	g_playerCnt[PLAYER_COUNT_ALL];

static PLAYERANIM	g_PlayerAnim[ANIME_ALL] = {
	{0,0.0f,0,1},							// loop, cntSpeed, texNo, xMax
	{0,0.05f,1,2},
	{1,0.1f,2,4},
	{1,0.1f,3,4},
	{0,0.2f,4,2},
	{1,0.1f,5,4},
	{0,0.0f,6,1},
	{0,0.0f,7,1},
	{1,0.05f,8,2},
	{0,0.0f,9,1},
};

//=============================================================================
// ����������
//=============================================================================
HRESULT InitPlayer(void)
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


	// �v���C���[�\���̂̏�����
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		g_Player[i].use = TRUE;
		g_Player[i].movable = TRUE;

		FIELD* field = GetField();
		// �Ō�Ɏ��񂾏ꏊ�ɂ���Ĉʒu��ς���
		switch (field->checkPtCnt)
		{
		case 0:				// 1��
			g_Player[i].pos = XMFLOAT3(150.0f, 1100.0f, 0.0f);
			break;

		case 1:				// 2��
			g_Player[i].pos = XMFLOAT3(3780.0f, 1500.0f, 0.0f);
			break;

		case 2:				// 3��
			g_Player[i].pos = XMFLOAT3(4110.0f, 2540.0f, 0.0f);
			break;

		case 3:				// 4��
			g_Player[i].pos = XMFLOAT3(7002.0f, 1656.0f, 0.0f);
			break;

		case 4:				// 5��
			g_Player[i].pos = XMFLOAT3(5975.0f, 756.0f, 0.0f);
			break;

		case 5:				// 5��
			g_Player[i].pos = XMFLOAT3(8370.0f, 680.0f, 0.0f);
			break;

		case 6:				// 6��
			g_Player[i].pos = XMFLOAT3(9500.0f, 680.0f, 0.0f);
			break;
		}

		g_Player[i].energy = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Player[i].pos_old = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Player[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Player[i].w = TEXTURE_WIDTH;
		g_Player[i].h = TEXTURE_HEIGHT;
		g_Player[i].collisionW = g_Player[i].w;
		g_Player[i].collisionH = g_Player[i].h;
		g_Player[i].hp = PLAYER_HP_MAX;
		g_Player[i].opacity = 1.0f;
		g_Player[i].countAnim = 0;

		g_Player[i].left = FALSE;							// �X�^�[�g���͉E����
		// g_Player[i].dir = ANIME_NORMAL;
		g_Player[i].moving = FALSE;							// �ړ����t���O
		g_Player[i].patternAnim = ANIME_NORMAL;				// �ŏ��̃A�j���[�V����
		g_Player[i].pattern_old = ANIME_NORMAL;				// �ʏ�ɂ��Ă���
		// g_Player[i].patternAnim = g_Player[i].dir * TEXTURE_PATTERN_DIVIDE_X;
		// g_Player[i].patternAnim = g_Player[i].patternAnim * g_anime[1][g_Player[i].anim];

		// �W�����v�̏�����
		g_Player[i].jump = FALSE;
		g_Player[i].jumpY = 0.0f;

		// �U���̏�����
		g_Player[i].attack = FALSE;

		g_Player[i].ground = FALSE;
		g_Player[i].updown = 0;
		g_Player[i].gravityY = 0.0f;						// �d�͂̏�����

		g_Player[i].damage = FALSE;							// �ŏ��͍U���ɓ������Ă��Ȃ�
	}


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitPlayer(void)
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
void UpdatePlayer(void)
{
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		// �����Ă���v���C���[��������������
		if (g_Player[i].use == TRUE)
		{
			// �o�b�N�A�b�v������Ă���
			g_Player[i].pos_old = g_Player[i].pos;				// �����蔻��p�Ɉړ��O�̍��W�̃o�b�N�A�b�v
			g_Player[i].pattern_old = g_Player[i].patternAnim;	// �O�̃A�j���[�V�����̃o�b�N�A�b�v

			// �v���C���[�ɂ�����͂�߂�
			g_Player[i].energy.y = 0;

			FIELD* field = GetField();
			CHECKPT* checkpt = GetCheckPt();


			// �A�j���[�V����  
			if ((g_Player[i].patternAnim != ANIME_HASHIGO) && (g_Player[i].movable)) g_Player[i].countAnim += g_PlayerAnim[g_Player[i].patternAnim].cntSpeed;

			if (g_PlayerAnim[g_Player[i].patternAnim].loop)
			{	// ���[�v����A�j���[�V�����Ȃ�
				if (g_Player[i].countAnim > g_PlayerAnim[g_Player[i].patternAnim].xMax)
				{
					g_Player[i].countAnim = 0.0f;
				}
			}
			else
			{	// ���[�v���Ȃ��A�j���[�V�����Ȃ�
				if (g_Player[i].countAnim > (g_PlayerAnim[g_Player[i].patternAnim].xMax - 1.0f))
				{
					// �Ō�̊G�ɂ��Ă���
					g_Player[i].countAnim = (float)(g_PlayerAnim[g_Player[i].patternAnim].xMax) - 1.0f;

				}
			}
			if ((g_Player[i].moving == FALSE) && (g_Player[i].updown != UPDOWN_DURING)) {
				g_Player[i].hp >= PLAYER_HP_LAW ? g_Player[i].patternAnim = ANIME_NORMAL : g_Player[i].patternAnim = ANIME_CRISIS;
			}

			// �_���[�W�����������
			if (g_Player[i].damage == TRUE)
			{
				g_playerCnt[PLAYER_COUNT_DAMAGE]++;
				g_Player[i].patternAnim = ANIME_DAMAGE;

				if (g_playerCnt[PLAYER_COUNT_DAMAGE] > PLAYER_DAMAGE_INTERVAL / 2)
				{
					// �������߂ɑ���\�ɖ߂�
					g_Player[i].movable = TRUE;

					// ���G���ԏI��
					if (g_playerCnt[PLAYER_COUNT_DAMAGE] > PLAYER_DAMAGE_INTERVAL)
					{
						g_Player[i].damage = FALSE;
						g_playerCnt[PLAYER_COUNT_DAMAGE] = 0.0f;
					}
				}
			}


			// ����\�ȂƂ��L�[���͂ňړ� 
			if (g_Player[i].movable == TRUE)
			{
				float speed = PLAYER_DUSH_SPEED;

				g_Player[i].moving = FALSE;


				// ���Ƀn�V�S�ȂǏ���~��ł�����̂�����Ƃ�
				if (g_Player[i].updown != UPDOWN_FALSE)
				{
					// �L�[�{�[�h�ł̏���
					if (GetKeyboardPress(DIK_DOWN))
					{
						g_Player[i].pos.y += speed;
						g_Player[i].countAnim += g_PlayerAnim[g_Player[i].patternAnim].cntSpeed;
						g_Player[i].moving = TRUE;
					}
					else if (GetKeyboardPress(DIK_UP))
					{
						g_Player[i].pos.y -= speed;
						g_Player[i].countAnim += g_PlayerAnim[g_Player[i].patternAnim].cntSpeed;
						g_Player[i].moving = TRUE;
					}

					// �Q�[���p�b�h�ł̏���
					if (IsButtonPressed(0, BUTTON_DOWN))
					{
						g_Player[i].pos.y += speed;
						g_Player[i].countAnim += g_PlayerAnim[g_Player[i].patternAnim].cntSpeed;
						g_Player[i].moving = TRUE;
					}
					else if (IsButtonPressed(0, BUTTON_UP))
					{
						g_Player[i].pos.y -= speed;
						g_Player[i].countAnim += g_PlayerAnim[g_Player[i].patternAnim].cntSpeed;
						g_Player[i].moving = TRUE;
					}

					if (g_Player[i].updown == UPDOWN_DURING)
					{
						g_Player[i].patternAnim = ANIME_HASHIGO;
					}
				}
				
				
				if (g_Player[i].updown != UPDOWN_DURING)
				{
					if (GetKeyboardPress(DIK_RIGHT))
					{
						g_Player[i].pos.x += speed;
						g_Player[i].patternAnim = ANIME_WALK;
						g_Player[i].left = FALSE;
						g_Player[i].moving = TRUE;
					}
					else if (GetKeyboardPress(DIK_LEFT))
					{
						g_Player[i].pos.x -= speed;
						g_Player[i].patternAnim = ANIME_WALK;
						g_Player[i].left = TRUE;
						g_Player[i].moving = TRUE;
					}

					if (IsButtonPressed(0, BUTTON_RIGHT))
					{
						g_Player[i].pos.x += speed;
						g_Player[i].patternAnim = ANIME_WALK;
						g_Player[i].left = FALSE;
						g_Player[i].moving = TRUE;
					}
					else if (IsButtonPressed(0, BUTTON_LEFT))
					{
						g_Player[i].pos.x -= speed;
						g_Player[i].patternAnim = ANIME_WALK;
						g_Player[i].left = TRUE;
						g_Player[i].moving = TRUE;
					}
				}


				// �W�����v�������H
				if (g_Player[i].jump == TRUE)
				{
					// ����������قǃW�����v�͂��オ��
					if ((g_jumpPower < PLAYER_JUMP_POWER_MAX) && ((GetKeyboardPress(DIK_J)) || IsButtonPressed(0, BUTTON_A)))
					{
						g_jumpPower += 1.0f;
						g_jumpCntMax += 1.0f;
					}
					float angle = (XM_PI / (g_jumpCntMax * 2)) * g_playerCnt[PLAYER_COUNT_JUMP];
					float y = g_jumpPower * cosf(XM_PI / 2 + angle);
					g_Player[i].jumpY = g_jumpPower + y;
					g_Player[i].moving = TRUE;

					g_Player[i].patternAnim = ANIME_JUMP;

					g_playerCnt[PLAYER_COUNT_JUMP]++;

					if (g_playerCnt[PLAYER_COUNT_JUMP] > g_jumpCntMax)
					{
						g_Player[i].jump = FALSE;

						// �����l�ɖ߂�
						g_playerCnt[PLAYER_COUNT_JUMP] = 0;
						g_Player[i].jumpY = 0.0f;
						g_jumpPower = PLAYER_JUMP_POWER_FIRST;
						g_jumpCntMax = PLAYER_JUMP_CNT_FIRST;
					}
				}
				// �W�����v�{�^���������H
				else if ((g_Player[i].jump == FALSE) && ((GetKeyboardTrigger(DIK_J)) || IsButtonTriggered(0, BUTTON_A)))
				{
					if (g_Player[i].ground)
					{
						g_Player[i].jump = TRUE;
						g_playerCnt[PLAYER_COUNT_JUMP] = 0;
						g_Player[i].jumpY = 0.0f;
					}
				}
				else
				{
					// �v���C���[�̑������n�ʁH
					if (g_Player[i].ground)
					{
						g_Player[i].gravityY = 0.0f;
						g_Player[i].energy.y = 0.0f;

						if (((GetKeyboardTrigger(DIK_G)) || (IsButtonTriggered(0, BUTTON_X))) && (g_Player[i].updown != UPDOWN_DURING))
						{
							g_Player[i].sliding = TRUE;
							float addX = (g_Player[i].left) ? g_Player[i].w / 2 : -g_Player[i].w / 2;
							SetEffect(XMFLOAT3(g_Player[i].pos.x + addX, g_Player[i].pos.y + g_Player[i].h / 3, 0.0f), 0, EFFECT_SLIDING);
						}
					}
					else 
					{
						g_Player[i].gravityY += PLAYER_GRAVITY;

						if (g_Player[i].gravityY > PLAYER_GRAVITY_MAX)
						{
							g_Player[i].gravityY = PLAYER_GRAVITY_MAX;
						}
						g_Player[i].energy.y += g_Player[i].gravityY;
					}
				}

				g_Player[i].energy.y -= g_Player[i].jumpY;

				// �o���b�g����
				if (GetKeyboardTrigger(DIK_SPACE))
				{
					XMFLOAT3 pos = g_Player[i].pos;
					SetPlayerBullet(pos);
					g_Player[i].attack = TRUE;
				}
				if (IsButtonTriggered(0, BUTTON_B))
				{
					XMFLOAT3 pos = g_Player[i].pos;
					SetPlayerBullet(pos);
					g_Player[i].attack = TRUE;
				}

				// MAP�O�`�F�b�N�i�`�F�b�N�|�C���g�܂ł̓X�N���[������j
				BG* bg = GetBG();

				if (g_Player[i].pos.x > checkpt[field->checkPtCnt].x)
				{
					g_Player[i].pos.x = checkpt[field->checkPtCnt].x;
				}

				if (g_Player[i].pos.x < checkpt[field->checkPtCnt].prevX)
				{
					g_Player[i].pos.x = checkpt[field->checkPtCnt].prevX;
				}

				if (g_Player[i].pos.y > checkpt[field->checkPtCnt].y)
				{
					g_Player[i].pos.y = checkpt[field->checkPtCnt].y;
				}

				if (g_Player[i].pos.y < checkpt[field->checkPtCnt].prevY)
				{
					g_Player[i].pos.y = checkpt[field->checkPtCnt].prevY;
				}

				// �v���C���[�̗����ʒu����MAP�̃X�N���[�����W���v�Z����
				bg->pos.x = g_Player[i].pos.x - PLAYER_DISP_X;

				// �Q�[����ʂ̍��E�̒[�̓`�F�b�N�|�C���g�܂�
				if (bg->pos.x < checkpt[field->checkPtCnt].prevX) bg->pos.x = checkpt[field->checkPtCnt].prevX;
				if (bg->pos.x > checkpt[field->checkPtCnt].x - GAMESCREEN_WIDTH) bg->pos.x = checkpt[field->checkPtCnt].x - GAMESCREEN_WIDTH;


				bg->pos.y = g_Player[i].pos.y - PLAYER_DISP_Y;

				// �Q�[����ʂ̏㉺�̒[�̓`�F�b�N�|�C���g�܂�
				if (bg->pos.y < checkpt[field->checkPtCnt].prevY) bg->pos.y = checkpt[field->checkPtCnt].prevY;
				if (bg->pos.y > checkpt[field->checkPtCnt].y - SCREEN_HEIGHT) bg->pos.y = checkpt[field->checkPtCnt].y - SCREEN_HEIGHT;

			}


			// �ړ����I�������G�l�~�[�Ƃ̓����蔻��
			{
				ENEMY* enemy = GetEnemy();
				ENEMY_PATTERN* enemyPatt = GetEnemyPatt();

				// �G�l�~�[�̐��������蔻����s��
				for (int j = 0; j < ENEMY_MAX; j++)
				{
					// �����Ă�G�l�~�[�Ɠ����蔻�������
					if ((enemy[j].use == ENEMY_USE_TRUE) && (enemyPatt[enemy[j].type].damage))
					{
						BOOL ans = CollisionBB(g_Player[i].pos, g_Player[i].w, g_Player[i].h,
							enemy[j].pos, enemy[j].w, enemy[j].h);
						// �������Ă���H
						if (ans == TRUE)
						{
							// �����������̏���
							DamagePlayer(PLAYER_COLLISION_DAMAGE);
						}
					}
				}
			}

			// �v���C���[�ɂ��������G�l���M�[(�d�͂Ȃ�)�����W�ɔ��f
			XMVECTOR pos = XMLoadFloat3(&g_Player[i].pos);
			XMVECTOR energy = XMLoadFloat3(&g_Player[i].energy);
			pos += energy;
			XMStoreFloat3(&g_Player[i].pos, pos);

			// �O�̃t���[�����v���C���[�̈ʒu���������Ă�����(�������H)
			if ((g_Player[i].pos.y > g_Player[i].pos_old.y) && (g_Player[i].updown != UPDOWN_DURING))
			{
				g_Player[i].patternAnim = ANIME_DOWN;
			}

			// �����`�F�b�N�|�C���g�ɓ��B������
			if ((g_Player[i].pos.x == checkpt[field->checkPtCnt].x) && (checkpt[field->checkPtCnt].dir == 1))		// �E�����̏ꍇ
			{
				SetBGScroll(SCRL_DIR_RIGHT, field->checkPtCnt);
			}
			else if ((g_Player[i].pos.y == checkpt[field->checkPtCnt].y) && (checkpt[field->checkPtCnt].dir == 2))	// �������̏ꍇ
			{
				SetBGScroll(SCRL_DIR_DOWN, field->checkPtCnt);
			}
			else if ((g_Player[i].pos.y == checkpt[field->checkPtCnt].prevY) && (checkpt[field->checkPtCnt].dir == 0))	// ������̏ꍇ
			{
				SetBGScroll(SCRL_DIR_UP, field->checkPtCnt);
			}


#ifdef _DEBUG	// �f�o�b�O����\������
			BG* bg = GetBG();

			if (GetKeyboardTrigger(DIK_R))
			{
				g_Player[i].pos = XMFLOAT3(bg->pos.x, bg->pos.y, 0.0f);
			}
#endif
			// �U���A�N�V�������H
			if (g_Player[i].attack == TRUE)
			{
				g_playerCnt[PLAYER_COUNT_ATTACK]++;
				g_Player[i].moving = TRUE;

				// �����Ă��Ԃ������瑖��Ȃ���U���̕��̃A�j���[�V������
				g_Player[i].patternAnim == ANIME_WALK ? g_Player[i].patternAnim = ANIME_ATTACKRUN : g_Player[i].patternAnim = ANIME_ATTACK;

				if (g_playerCnt[PLAYER_COUNT_ATTACK] > PLAYER_ATTACK_ACTION)
				{
					g_Player[i].attack = FALSE;
					g_playerCnt[PLAYER_COUNT_ATTACK] = 0.0f;
				}
			}

			// �X���C�f�B���O���H
			if (g_Player[i].sliding)
			{
				g_Player[i].moving = TRUE;
				g_Player[i].w = TEXTURE_HEIGHT;
				g_Player[i].collisionH = g_Player[i].h * LOSE_COLLISIONH;
				g_Player[i].patternAnim = ANIME_SLIDING;

				float slide = PLAYER_SLIDING_SPEED;
				g_Player[i].energy.x += (g_Player[i].left) ? slide * -1 : slide;
				g_playerCnt[PLAYER_COUNT_SLIDING]++;

				// speed�̌��E�l
				if (slide > PLAYER_SLIDING_MAX) slide = PLAYER_SLIDING_MAX;

				if (g_playerCnt[PLAYER_COUNT_SLIDING] > PLAYER_SLIDING_ACTION)
				{
					g_Player[i].energy.x = 0.0f;
					g_Player[i].sliding = FALSE;
					g_Player[i].w = TEXTURE_WIDTH;
					g_Player[i].moving = FALSE;
					g_Player[i].collisionH = g_Player[i].h;
					g_playerCnt[PLAYER_COUNT_SLIDING] = 0.0f;
				}
			}


			// �A�j���[�V�������ς������J�E���g��0�ɖ߂�
			if (g_Player[i].patternAnim != g_Player[i].pattern_old)
			{
				g_Player[i].countAnim = 0;
			}

		}

		// �v���C���[������
		else
		{
			g_playerCnt[PLAYER_COUNT_DEAD]++;
			
			if (g_playerCnt[PLAYER_COUNT_DEAD] > PLAYER_DAMAGE_INTERVAL)
			{
				FIELD* field = GetField();
				AddRetry(1);
				SetFade(FADE_OUT, MODE_GAME);						// ��U�t�F�[�h�A�E�g
				g_playerCnt[PLAYER_COUNT_DEAD] = 0.0f;

			}
		}
	}


#ifdef _DEBUG	// �f�o�b�O����\������
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawPlayer(void)
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

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (g_Player[i].use == TRUE)		// ���̃v���C���[���g���Ă���H
		{									// Yes

			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_PlayerAnim[g_Player[i].patternAnim].texNo]);

			//�v���C���[�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Player[i].pos.x - bg->pos.x + OUTSIDE_WIDTH;	// �v���C���[�̕\���ʒuX
			float py = g_Player[i].pos.y - bg->pos.y;	// �v���C���[�̕\���ʒuY
			float pw = g_Player[i].w;		// �v���C���[�̕\����
			float ph = g_Player[i].h;		// �v���C���[�̕\������

			// �A�j���[�V�����p
			float tw = 1.0f / g_PlayerAnim[g_Player[i].patternAnim].xMax;			// �e�N�X�`���̕�
			float th = 1.0f / 1;													// �e�N�X�`���̍���
			float tx = (float)((int)g_Player[i].countAnim % g_PlayerAnim[g_Player[i].patternAnim].xMax) * fabsf(tw);	// �e�N�X�`���̍���X���W
			float ty = 0;		// �e�N�X�`���̍���Y���W(1���̉摜�ɑS�����߂�Ȃ�x�Ŋ���)

			float angle = (XM_PI / PLAYER_ATTACK_ACTION) * g_playerCnt[PLAYER_COUNT_DAMAGE];

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, g_Player[i].opacity * fabsf(sinf(XM_PI / 2 + angle))),	// �_���[�W���󂯂��炵�΂炭�`�J�`�J������
				g_Player[i].rot.z, g_Player[i].left);								// �������̎��͉摜�𔽓]

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);

		}
	}


}


//=============================================================================
// Player�\���̂̐擪�A�h���X���擾
//=============================================================================
PLAYER* GetPlayer(void)
{
	return &g_Player[0];
}

//=============================================================================
// �v���C���[��HP�����炷
//=============================================================================
void DamagePlayer(float damage)
{
	if (g_Player[0].damage == FALSE)		// ���̃_���[�W�����炦���ԂɂȂ��Ă�����
	{
		g_Player[0].hp -= damage;

		if (g_Player[0].hp <= 0)
		{
			// HP���Ȃ��Ȃ�����use��FALSE��
			g_Player[0].use = FALSE;
			SetEffect(g_Player[0].pos, 0, EFFECT_PLAYERDEAD);		// HP���Ȃ��Ȃ����Ƃ��̃G�t�F�N�g
			PlaySound(SOUND_LABEL_SE_playerDead00);
			StopSound(SOUND_LABEL_BGM_GAME);
		}
		else
		{
			g_Player[0].damage = TRUE;
			g_Player[0].movable = FALSE;
			PlaySound(SOUND_LABEL_SE_damage00);			// �����������̉�
		}
	}
}



