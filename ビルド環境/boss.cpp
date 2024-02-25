//=============================================================================
//
// �{�X���� [boss.cpp]
// Author : GP11A132 15 �����ق̍�
//
//=============================================================================
#include "boss.h"
#include "bg.h"
#include "player.h"
#include "collision.h"
#include "enemyBullet.h"
#include "playerBullet.h"
#include "sound.h"
#include "effect.h"
#include "field.h"
#include "main.h"
#include "status.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(201)													// �e�N�X�`���̐�
#define TEXTURE_HEIGHT				(252)													// �e�N�X�`���̐�
#define TEXTURE_MAX					(5)														// �e�N�X�`���̐�

#define GROUND_POS					(750)
#define BOSS_Y						(GROUND_POS - TEXTURE_HEIGHT / 2)

#define TEXTURE_PATTERN_DIVIDE_X	(1)														// �A�j���p�^�[���̃e�N�X�`�����������iX)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)														// �A�j���p�^�[���̃e�N�X�`�����������iY)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)		// �A�j���[�V�����p�^�[����
#define ANIM_WAIT					(4)														// �A�j���[�V�����̐؂�ւ��Wait�l

#define JUMP_COUNT_MIN				(10)													// ���W�����v�t���[����
#define JUMP_COUNT_MID				(30)													// ���W�����v�̃t���[����
#define JUMP_COUNT_MAX				(60)													// ��W�����v�̃t���[����
#define JUMP_POWER					(300.0f)												// �W�����v��
#define JUMP_POWER_MID				(370.0f)												// ���W�����v��
#define JUMP_POWER_LARGE			(450.0f)												// �傫���W�����v��
#define DUSH_SPEED					(15.0f)													// �_�b�V���X�s�[�h

#define	BOSS_GRAVITY				(2.4f)													// �d��
#define	BOSS_GRAVITY_MAX			(10.0f)													// �ő�~����

#define FOLLOW_EFFECT_MAX			(6)														// �Ǐ]�G�t�F�N�g�����o����
#define BULLET_HEIGHT				(550)													// �o���b�g�̊����
#define BULLET_INTERVAL				(75)													// �o���b�g�̊Ԋu
#define CLIMAX_END					(950)													// �ő�U������߂�^�C�~���O
#define BOSS_DECO_OPACTY			(0.7f)													// �{�X�̃I�[���̃A���t�@�l
#define BOSS_DOWN					(2.0f)													// �{�X�����񂾂�~���
#define BG_OPACITY					(0.02f)
#define	BOSS_TIME1					(30)
#define	BOSS_TIME2					(50)
#define	BOSS_TIME3					(300)
#define	BOSS_TIME4					(135)
#define	BOSS_TIME5					(600)
#define	BOSS_SCORE					(10000)

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/normalBoss.png",
	"data/TEXTURE/wallBoss.png",
	"data/TEXTURE/deathBoss.png",
};

static BOOL				g_Load = FALSE;			// ���������s�������̃t���O
static BOSS				g_Boss[BOSS_MAX];

static BOOL g_dir;								// 0:�E 1:��
static int g_jumpCnt;
static int g_effectCnt;
static int g_patternOld;						// �O�̃A�j���[�V�����p�^�[��
static int g_phaseOld;							// �O�̍s���i�K
static float g_leftEnd;							// ���[�̍��W
static int g_climaxCnt = 0;							// �N���C�}�b�N�X�Z������������

static XMFLOAT3 g_startPos;						// �A�N�V�����X�^�[�g���̈ʒu
static XMFLOAT3 g_targetPos;					// �v���C���[�̈ʒu��ۑ�����p
static XMFLOAT3 g_effectPos;					// �G�t�F�N�g�𔭐�������ʒu

static BOSSANIM g_BossAnime[BOSS_ANIM_ALL] =
{
	{1, 0.2f, 0, 8},								// loop, cntSpeed, texNo, xMax,
	{1, 0.2f, 0, 8},
	{1, 0.2f, 0, 8},
	{1, 0.2f, 1, 3},
	{0, 0.0f, 2, 1},
};

static BOSSACTION g_BossAction[ACTION_ALL] =
{
	{0},
	{40, 0, 40, 0},
	{40, 0, 15, 10},
	{50, 0, 20, 15},
};


//=============================================================================
// ����������
//=============================================================================
HRESULT InitBoss(void)
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
	for (int i = 0; i < BOSS_MAX; i++)
	{
		g_Boss[i].damage = FALSE;
		g_Boss[i].use = FALSE;
		g_Boss[i].jump = FALSE;
		g_Boss[i].left = TRUE;									// �ŏ��͍�����
		g_Boss[i].action = ACTION_NONE;
		g_Boss[i].phase  = PHASE_FIRST;
		g_Boss[i].pos = XMFLOAT3(10400.0f, 0.0f, 0.0f);
		g_Boss[i].energy = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Boss[i].rot  = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Boss[i].scl = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Boss[i].countAnim = 0.0f;
		g_Boss[i].time = 0.0f;
		g_Boss[i].w = TEXTURE_WIDTH;
		g_Boss[i].h = TEXTURE_HEIGHT;
		g_Boss[i].jumpY = 0.0f;
		g_Boss[i].gravityY = 0.0f;
		g_Boss[i].speed = 0.0f;
		g_Boss[i].texNo = 0;
		g_Boss[i].patternAnim = BOSS_ANIM_NORMAL;				// �ŏ��̃A�j���[�V����
		g_Boss[i].hp = BOSS_HP_MAX;
		g_Boss[i].opacity = 0.0f;
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitBoss(void)
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
void UpdateBoss(void)
{
	FIELD* field = GetField();
	CHECKPT* checkPt = GetCheckPt();
	BGDECO* bgDeco = GetBGDeco();
	g_leftEnd = checkPt[field->checkPtCnt].prevX;	// �{�X�̍s�����W�̍��[���Z�b�g


	if (g_Boss[0].use == TRUE)										// �����Ă���H
	{
		if (g_Boss[0].hp > 0)										// �����hp���܂�����H
		{
			PLAYER* player = GetPlayer();

			XMVECTOR epos = XMLoadFloat3(&g_Boss[0].pos);			// �O�̍��W�̃o�b�N�A�b�v
			g_patternOld = g_Boss[0].patternAnim;					// �O�̃A�j���[�V�����̃o�b�N�A�b�v
			g_phaseOld = g_Boss[0].phase;							// �O�̍s���t�F�[�X�̃o�b�N�A�b�v

			// �{�X�ɂ�����͂�߂�
			g_Boss[0].energy.y = 0;

			// �A�j���[�V����  
			g_Boss[0].countAnim += g_BossAnime[g_Boss[0].patternAnim].cntSpeed;
			if (g_BossAnime[g_Boss[0].patternAnim].loop)
			{	// ���[�v����A�j���[�V�����Ȃ�
				if (g_Boss[0].countAnim > g_BossAnime[g_Boss[0].patternAnim].xMax)
				{
					g_Boss[0].countAnim = 0.0f;
				}
			}
			else
			{	// ���[�v���Ȃ��A�j���[�V�����Ȃ�
				if (g_Boss[0].countAnim > (g_BossAnime[g_Boss[0].patternAnim].xMax - 1.0f))
				{
					// �Ō�̊G�ɂ��Ă���
					g_Boss[0].countAnim = (float)(g_BossAnime[g_Boss[0].patternAnim].xMax) - 1.0f;
				}
			}



			// ���̃A�N�V�����ɂ���ď�����؂�ւ�
			switch (g_Boss[0].action)
			{
			case ACTION_NONE:
				g_Boss[0].pos.y += BOSS_DOWN;				// �o�ꎞ�͂��񂾂�~���

				if (g_Boss[0].pos.y > BOSS_Y)
				{
					StartAction(ACTION_DASH);
					player[0].movable = TRUE;
					g_Boss[0].damage = TRUE;
				}
				break;

			case ACTION_DASH:

				switch (g_Boss[0].phase)
				{
				case PHASE_FIRST:								// �܂��ړ�����������
					g_Boss[0].left = g_dir;
					g_Boss[0].patternAnim = BOSS_ANIM_NORMAL;

					// �ҋ@�s��
					SetIdle();
					break;

				case PHASE_SECOND:								// �W�����v���ė��[(�X�^�[�g�n�_)�ɍs��

					XMFLOAT3 dirPos;
					g_dir ? dirPos = XMFLOAT3(g_Boss[0].w / 2 + g_leftEnd, BOSS_Y, 0.0f) : dirPos = XMFLOAT3(MAP_WIDTH -  g_Boss[0].w / 2, BOSS_Y, 0.0f);

					SetJump(dirPos, g_Boss[0].pos, FALSE, JUMP_POWER_LARGE, JUMP_COUNT_MAX);
					break;

				case PHASE_THIRD:								// ���̐i�s����������

					g_Boss[0].left = (g_dir) ? 0 : 1;			// �t������
					g_Boss[0].patternAnim = BOSS_ANIM_NORMAL;

					SetIdle();
					break;

				case PHASE_FOURTH:								// �ːi����

					g_Boss[0].damage = FALSE;
					XMFLOAT3 reversePos;						// �t�̒[�Ɍ������ēːi����
					g_dir ? reversePos = XMFLOAT3(MAP_WIDTH - g_Boss[0].w / 2, BOSS_Y, 0.0f) : reversePos = XMFLOAT3(g_Boss[0].w / 2 + g_leftEnd, BOSS_Y, 0.0f);
					XMVECTOR reverseVec = XMLoadFloat3(&reversePos) - epos;

					float angle = atan2f(reverseVec.m128_f32[1], reverseVec.m128_f32[0]);
					float speed = DUSH_SPEED;
					float move = cosf(angle) * fabsf(speed);

					g_Boss[0].patternAnim = BOSS_ANIM_NORMAL;
					g_Boss[0].pos.x += move;

					// �Ǐ]�G�t�F�N�g���Z�b�g
					if ((fabsf(g_Boss[0].time * move) > g_Boss[0].w / 1.5) && (g_effectCnt < FOLLOW_EFFECT_MAX))
					{
						g_Boss[0].time = 0.0f;					// �J�E���g����U�߂�
						g_effectPos = XMFLOAT3(g_Boss[0].pos.x + (g_Boss[0].time * move), BOSS_Y + 45, 0.0f);
						SetEffect(g_effectPos, 0, EFFECT_BOSS_FOLLOW);

						g_effectCnt++;
					}

					// �[�ɓ���������
					if (fabsf(g_Boss[0].pos.x - reversePos.x) < 8.0f)
					{
						int next = rand() % 3;
						if (g_Boss[0].hp <= BOSS_HP_MAX / 2)			// HP�������ȉ��ɂȂ�����
						{
							if (g_climaxCnt == 0)
							{
								StartAction(ACTION_CLIMAX);
							}
							else if (g_Boss[0].hp <= BOSS_HP_MAX / 4)
							{
								StartAction(ACTION_CLIMAX);
							}
							else
							{
								next == 0 ? StartAction(ACTION_JUMP) : StartAction(ACTION_DASH);
								g_Boss[0].damage = TRUE;
							}
						}
						else
						{
							next == 0 ? StartAction(ACTION_JUMP) : StartAction(ACTION_DASH);
							g_Boss[0].damage = TRUE;
						}
					}
					break;
				}

				break;


			case ACTION_JUMP:

				switch (g_Boss[0].phase)
				{
				case PHASE_FIRST:								// �ҋ@�s��

					SetIdle();
					break;

				case PHASE_SECOND:								// �߂��̕ǂɃW�����v

					XMFLOAT3 dirPos;
					dirPos = g_Boss[0].pos.x > (SCREEN_WIDTH / 2 - OUTSIDE_WIDTH + g_leftEnd) ? XMFLOAT3(MAP_WIDTH - g_Boss[0].w / 2, 200.0f, 0.0f) : XMFLOAT3(g_Boss[0].w / 2 + g_leftEnd, 200.0f, 0.0f);
					SetJump(dirPos, g_Boss[0].pos, TRUE, JUMP_POWER_LARGE, JUMP_COUNT_MID);
					break;

				case PHASE_THIRD:								// �ҋ@�s��

					g_Boss[0].patternAnim = BOSS_ANIM_WALL;
					g_Boss[0].pos.x > (SCREEN_WIDTH / 2 - OUTSIDE_WIDTH + g_leftEnd) ? g_Boss[0].left = TRUE : g_Boss[0].left = FALSE;
					SetIdle();
					break;

				case PHASE_FOURTH:								// �v���C���[�Ɍ������Ĕ��

					// ����������ɏオ���Ăقڗ�����W�����v
					if (g_Boss[0].jump == TRUE)
					{
						g_targetPos = player[0].pos;			// �����O�̃t���[���̍��W��ڕW�n�ɂ��Ĕ�����]�n���c��
						XMVECTOR dirVec = XMLoadFloat3(&g_targetPos) - XMLoadFloat3(&g_Boss[0].pos);

						float angle = atan2f(dirVec.m128_f32[1], dirVec.m128_f32[0]);
						float speed = 18.0f;

						float jumpAngle = (XM_PI / JUMP_COUNT_MIN) * g_jumpCnt;
						float y = 5 * cosf(XM_PI / 2 + jumpAngle);

						g_Boss[0].jumpY = y;
						g_Boss[0].pos.x += cosf(angle) * fabsf(speed);
						g_Boss[0].patternAnim = BOSS_ANIM_NORMAL;

						g_jumpCnt++;

						g_Boss[0].energy.y += g_Boss[0].jumpY;


						if (g_jumpCnt > JUMP_COUNT_MIN)
						{
							// �W�����v�͈�U�X�g�b�v
							g_jumpCnt = (int)(JUMP_COUNT_MIN)+1;

							if (g_Boss[0].pos.y > BOSS_Y)
							{
								g_Boss[0].gravityY = 0;
								g_Boss[0].pos.y = BOSS_Y;
								g_Boss[0].jump = FALSE;
								SetEnemyBullet(XMFLOAT3(g_Boss[0].pos.x, g_Boss[0].pos.y + g_Boss[0].h / 4, 0.0f), BULLET_PATT_HORIZONTAL);
								StartAction(ACTION_DASH);
							}
							else
							{
								g_Boss[0].gravityY += BOSS_GRAVITY;

								if (g_Boss[0].gravityY > BOSS_GRAVITY_MAX)
								{
									g_Boss[0].gravityY = BOSS_GRAVITY_MAX;
								}
								g_Boss[0].energy.y += g_Boss[0].gravityY;
							}
						}
					}
					// �W�����v����H
					else
					{
						g_Boss[0].jump = TRUE;
						g_jumpCnt = 0;
						g_Boss[0].jumpY = 0.0f;
					}

					break;
				}

				break;

			case ACTION_CLIMAX:									// ���`
				switch (g_Boss[0].phase)
				{
				case PHASE_FIRST:								// �ҋ@�s��
					SetIdle();
					break;

				case PHASE_SECOND:								// �����Ɍ������ăW�����v
					XMFLOAT3 dirPos;
					dirPos = XMFLOAT3((g_leftEnd + MAP_WIDTH) / 2, 300.0f, 0.0f);
					SetJump(dirPos, g_Boss[0].pos, TRUE, JUMP_POWER_MID, JUMP_COUNT_MID);
					break;

				case PHASE_THIRD:
					SetIdle();
					break;

				case PHASE_FOURTH:
					bgDeco[BG_DECO_BOSSCLIMAX].use = TRUE;
					bgDeco[BG_DECO_BOSSCLIMAX].opacity = BOSS_DECO_OPACTY;		// �����l�ɖ߂�
					bgDeco[BG_DECO_BOSSCLIMAX].pos = XMFLOAT3(g_Boss[0].pos.x - bgDeco[BG_DECO_BOSSCLIMAX].w /2, g_Boss[0].pos.y - bgDeco[BG_DECO_BOSSCLIMAX].h / 2.5f, 0.0f);

					if ((int)(g_Boss[0].time) % BULLET_INTERVAL == 0)
					{
						float posX = (g_dir) ? g_leftEnd : MAP_WIDTH;
						SetEnemyBullet(XMFLOAT3(posX, BULLET_HEIGHT, 0.0f), BULLET_PATT_BOSSCLIMAX);
					}
					if (g_Boss[0].time > CLIMAX_END) SetIdle();

					break;


				case PHASE_FIFTH:
					bgDeco[BG_DECO_BOSSCLIMAX].opacity -= BG_OPACITY;
					g_Boss[0].pos.y += BOSS_DOWN;
					g_Boss[0].damage = FALSE;
					if (bgDeco[BG_DECO_BOSSCLIMAX].opacity < 0)
					{
						bgDeco[BG_DECO_BOSSCLIMAX].use = FALSE;
					}

					if (g_Boss[0].pos.y > BOSS_Y)
					{
						StartAction(ACTION_DASH);
						g_climaxCnt++;
						g_Boss[0].damage = TRUE;
					}
					break;
				}
				break;
			}


			// �ړ����I�������v���C���[�Ƃ̓����蔻��
			{
				for (int j = 0; j < PLAYER_MAX; j++)
				{
					// �����Ă�G�l�~�[�Ɠ����蔻�������
					if ((player[j].use == TRUE) && (g_Boss[0].action != ACTION_CLIMAX))
					{
						BOOL ans = CollisionBB(XMFLOAT3(g_Boss[0].pos.x, g_Boss[0].pos.y + g_Boss[0].h / 2, 0.0f), g_Boss[0].w, g_Boss[0].h / 2,
							player[j].pos, player[j].w, player[j].h);

						// �������Ă���H
						if (ans == TRUE)
						{
							// �����������̏���
							DamagePlayer(10.0f);		// �Ƃ肠����
						}
					}
				}
			}

			// �{�X�ɂ��������G�l���M�[(�d�͂Ȃ�)�����W�ɔ��f
			XMVECTOR pos = XMLoadFloat3(&g_Boss[0].pos);
			XMVECTOR energy = XMLoadFloat3(&g_Boss[0].energy);
			pos += energy;
			XMStoreFloat3(&g_Boss[0].pos, pos);

			g_Boss[0].time += 1.0f;

			// �A�j���[�V�������ς������J�E���g��0�ɖ߂�
			if (g_patternOld != g_Boss[0].patternAnim)
			{
				g_Boss[0].countAnim = 0;
			}

			// �t�F�[�Y���ς������G�t�F�N�g�J�E���g��0�ɖ߂�
			if (g_phaseOld != g_Boss[0].phase)
			{
				g_Boss[0].time = 0;
				g_effectCnt = 0;
			}
		}

		else
		{	// ���ʊԍۂ̉��o		

			g_Boss[0].time += 1.0f;
			PLAYER* player = GetPlayer();
			player->movable = FALSE;

			if (g_Boss[0].time == BOSS_TIME1)						// ���������X���[�ɂ���
			{
				SetFps(FPS_VALUE);
			}
			if (g_Boss[0].time == BOSS_TIME2)
			{
				StopSound(SOUND_LABEL_BGM_BOSS);
				PlaySound(SOUND_LABEL_SE_bossDead);
				SetEffect(XMFLOAT3(g_Boss[0].pos.x, g_Boss[0].pos.y + g_Boss[0].h / 3, 0.0f), 0, EFFECT_BOSSDEAD);
			}
			if (g_Boss[0].time == BOSS_TIME3)
			{
				g_Boss[0].use = FALSE;
				g_Boss[0].time = 0;
			}
		}
	}
	else if(g_Boss[0].hp <= 0)								// use��false����hp�͂��ł�0���{�X��|������̏���
	{
		PLAYER* player = GetPlayer();
		player->movable = FALSE;
		g_Boss[0].time += 1.0f;
		if (g_Boss[0].time == BOSS_TIME4)
		{
			PlaySound(SOUND_LABEL_SE_gameClear);
		}
		if (g_Boss[0].time == BOSS_TIME5)
		{
			SetStatus(STATUSGAME_CLEAR);
			AddScore(BOSS_SCORE);
			PlaySound(SOUND_LABEL_SE_gameClear2);
		}
	}

#ifdef _DEBUG	// �f�o�b�O����\������


#endif
}


//=============================================================================
// �`�揈��
//=============================================================================
void DrawBoss(void)
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

	for (int i = 0; i < BOSS_MAX; i++)
	{
		if (g_Boss[i].use == TRUE)			// ���̃G�l�~�[���g���Ă���H
		{										// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_BossAnime[g_Boss[i].patternAnim].texNo]);

			//�G�l�~�[�̈ʒu��e�N�X�`���[���W�𔽉f
			float ex = g_Boss[i].pos.x - bg->pos.x + OUTSIDE_WIDTH;	// �G�l�~�[�̕\���ʒuX
			float ey = g_Boss[i].pos.y - bg->pos.y;	// �G�l�~�[�̕\���ʒuY
			float ew = g_Boss[i].w;		// �G�l�~�[�̕\����
			float eh = g_Boss[i].h;		// �G�l�~�[�̕\������

			float tw = 1.0f / g_BossAnime[g_Boss[i].patternAnim].xMax;	// �e�N�X�`���̕�
			float th = 1.0f;	// �e�N�X�`���̍���
			float tx = (float)((int)g_Boss[i].countAnim % g_BossAnime[g_Boss[i].patternAnim].xMax) * tw;	// �e�N�X�`���̍���X���W
			float ty = 0.0f;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, ex, ey, ew, eh, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, g_Boss[i].opacity),
				g_Boss[i].rot.z, g_Boss[i].left);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

}


//=============================================================================
// BOSS�\���̂̐擪�A�h���X���擾
//=============================================================================
BOSS* GetBoss(void)
{
	return &g_Boss[0];
}


//=============================================================================
// Boss�ɑ΂���A�N�V����
//=============================================================================
void DamageBoss(float damage)
{
	g_Boss[0].hp -= damage;
	PlaySound(SOUND_LABEL_SE_attack00);			// �����������̉�

	if (g_Boss[0].hp <= 0)
	{
		PLAYER* player = GetPlayer();
		g_Boss[0].patternAnim = BOSS_ANIM_DEATH;
		player[0].movable = FALSE;
		player[0].damage = FALSE;

		// �������������ɏ�����΂����
		PLAYER_BULLET* bullet = GetPlayerBullet();
		bullet->pos.x > g_Boss[0].pos.x ? g_dir = 1 : g_dir = 0;

		// HP���Ȃ��Ȃ������U�Q�[������������
		SetFps(FPS_VALUE / 2);
		g_Boss[0].time = 0.0f;
	}
}

// �^�C�v�ɉ������U���p�^�[�����J�n
void StartAction(int pattern)
{
	g_Boss[0].time = 0.0f;						// �J�E���g��߂�
	g_Boss[0].action = pattern;

	if (pattern == ACTION_DASH)
	{
		g_dir = rand() % 2;						// 0:�E 1:��

		int skip = rand() % 5;
		if (skip == 0)
		{										// �܂�ɓr���̃t�F�[�Y���΂�
			g_Boss[0].phase = PHASE_THIRD;
			return;
		}
	}

	g_Boss[0].phase = PHASE_FIRST;
}

// �W�����v�s��
void SetJump(XMFLOAT3 dirPos, XMFLOAT3 pos, BOOL stay, float power, int cntMax)
{
	// �W�����v�������H
	if (g_Boss[0].jump == TRUE)
	{
		XMVECTOR dirVec = XMLoadFloat3(&dirPos) - XMLoadFloat3(&pos);

		float angle = atan2f(dirVec.m128_f32[1], dirVec.m128_f32[0]);
		float speed = (dirPos.x - g_startPos.x) / (cntMax);

		float jumpAngle;
		jumpAngle = stay ? (XM_PI / (cntMax * 2)) * g_jumpCnt : (XM_PI / cntMax) * g_jumpCnt;
		float y = power * cosf(XM_PI / 2 + jumpAngle);

		g_Boss[0].jumpY = y;

		g_Boss[0].pos.x += cosf(angle) * fabsf(speed);

		g_Boss[0].patternAnim = BOSS_ANIM_NORMAL;

		g_jumpCnt++;

		if (g_jumpCnt > cntMax)
		{
			g_Boss[0].jump = FALSE;

			// �����l�ɖ߂�
			g_jumpCnt = 0;
			g_Boss[0].jumpY = 0.0f;

			// ���̍s���t�F�[�Y��
			g_Boss[0].phase++;

			if(stay) return;
		}
	}
	// �W�����v����H
	else
	{
		g_Boss[0].jump = TRUE;
		g_jumpCnt = 0;
		g_Boss[0].jumpY = 0.0f;

		g_startPos = pos;
	}

	g_Boss[0].pos.y = g_startPos.y + g_Boss[0].jumpY;
}


// �ҋ@�s��
void SetIdle(void)
{
	if (g_Boss[0].time > g_BossAction[g_Boss[0].action].cntMax[g_Boss[0].phase])	// �w�肳�ꂽ�J�E���g���ҋ@
	{
		PLAYER* player = GetPlayer();
		g_targetPos = player[0].pos;		// �v���C���[�̈ʒu��ۑ����Ă���

		g_Boss[0].phase++;					// ���̃A�N�V�����t�F�[�Y��
	}
}