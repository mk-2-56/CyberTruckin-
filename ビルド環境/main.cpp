//=============================================================================
//
// ���C������ [main.cpp]
// Author : GP11A132 15 �����ق̍�
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "debugproc.h"
#include "input.h"

#include "title.h"
#include "bg.h"
#include "field.h"
#include "player.h"
#include "enemy.h"
#include "boss.h"
#include "enemyBullet.h"
#include "playerBullet.h"
#include "result.h"
#include "sound.h"
#include "fade.h"
#include "effect.h"
#include "status.h"
#include "guide.h"
#include "credit.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define CLASS_NAME		"AppClass"			// �E�C���h�E�̃N���X��
#define WINDOW_NAME		"Cyber Truckin'"	// �E�C���h�E�̃L���v�V������

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void Uninit(void);
void Update(void);
void Draw(void);


//*****************************************************************************
// �O���[�o���ϐ�:
//*****************************************************************************
long g_MouseX = 0;
long g_MouseY = 0;


#ifdef _DEBUG
int		g_CountFPS;							// FPS�J�E���^
char	g_DebugStr[2048] = WINDOW_NAME;		// �f�o�b�O�����\���p

#endif

int	g_Mode = MODE_TITLE;					// �N�����̉�ʂ�ݒ�
DWORD g_dwGameStartTime = 0;				// �Q�[���J�n���̎���
DWORD g_dwGameProgressTime;					// �Q�[���̌o�ߎ���
unsigned int g_fps = FPS_VALUE;


//=============================================================================
// ���C���֐�
//=============================================================================
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);	// �����Ă��ǂ����ǁA�x�����o��i���g�p�錾�j
	UNREFERENCED_PARAMETER(lpCmdLine);		// �����Ă��ǂ����ǁA�x�����o��i���g�p�錾�j

	// ���Ԍv���p
	DWORD dwExecLastTime;
	DWORD dwFPSLastTime;
	DWORD dwCurrentTime;
	DWORD dwFrameCount;

	WNDCLASSEX	wcex = {
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		WndProc,
		0,
		0,
		hInstance,
		NULL,
		LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		NULL,
		CLASS_NAME,
		NULL
	};
	HWND		hWnd;
	MSG			msg;
	
	// �E�B���h�E�N���X�̓o�^
	RegisterClassEx(&wcex);

	// �E�B���h�E�̍쐬
	hWnd = CreateWindow(CLASS_NAME,
		WINDOW_NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,																		// �E�B���h�E�̍����W
		CW_USEDEFAULT,																		// �E�B���h�E�̏���W
		SCREEN_WIDTH + GetSystemMetrics(SM_CXDLGFRAME) * 2,									// �E�B���h�E����
		SCREEN_HEIGHT + GetSystemMetrics(SM_CXDLGFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),	// �E�B���h�E�c��
		NULL,
		NULL,
		hInstance,
		NULL);

	// �E�B���h�E���[�h���t���X�N���[�����[�h���̏���
	BOOL mode = TRUE;

	int id = MessageBox(NULL, "Window���[�h�Ńv���C���܂����H", "�N�����[�h", MB_YESNOCANCEL | MB_ICONQUESTION);
	switch (id)
	{
	case IDYES:		// Yes�Ȃ�Window���[�h�ŋN��
		mode = TRUE;
		break;
	case IDNO:		// No�Ȃ�t���X�N���[�����[�h�ŋN��
		mode = FALSE;	// ���ɂ���ē����Ȃ���������
		break;
	case IDCANCEL:	// CANCEL�Ȃ�I��
	default:
		return -1;
		break;
	}

	// ����������(�E�B���h�E���쐬���Ă���s��)
	if(FAILED(Init(hInstance, hWnd, mode)))
	{
		return -1;
	}

	// �t���[���J�E���g������
	timeBeginPeriod(1);	// ����\��ݒ�
	dwExecLastTime = dwFPSLastTime = timeGetTime();	// �V�X�e���������~���b�P�ʂŎ擾
	dwCurrentTime = dwFrameCount = 0;

	// �E�C���h�E�̕\��(�����������̌�ɌĂ΂Ȃ��Ƒʖ�)
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	// ���b�Z�[�W���[�v
	while(1)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{// PostQuitMessage()���Ă΂ꂽ�烋�[�v�I��
				break;
			}
			else
			{
				// ���b�Z�[�W�̖|��Ƒ��o
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }
		else
		{
			dwCurrentTime = timeGetTime();

			g_dwGameProgressTime = dwCurrentTime - g_dwGameStartTime;		// �Q�[�����J�n���Ă���̎���

			if ((dwCurrentTime - dwFPSLastTime) >= 1000)	// 1�b���ƂɎ��s
			{
#ifdef _DEBUG
				g_CountFPS = dwFrameCount;
#endif
				dwFPSLastTime = dwCurrentTime;				// FPS�𑪒肵��������ۑ�
				dwFrameCount = 0;							// �J�E���g���N���A
			}

			if ((dwCurrentTime - dwExecLastTime) >= (1000 / g_fps))	// 1/60�b���ƂɎ��s
			{
				dwExecLastTime = dwCurrentTime;	// ��������������ۑ�

#ifdef _DEBUG	// �f�o�b�O�ł̎�����FPS��\������
				PLAYER* player = GetPlayer();
				FIELD* field = GetField();
				BG* bg = GetBG();
				ENEMY* enemy = GetEnemy();
				BOSS* boss = GetBoss();
				EFFECT* effect = GetEffect();
				PLAYER_BULLET* bullet = GetPlayerBullet();
				BGTYPE* bgtype = GetBGType();

				int mapNum = GetMap(player->pos.x, player->pos.y + player->h);
				wsprintf(g_DebugStr, WINDOW_NAME);
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], "FPS:%d posX:%dposY:%d enemyUse%d BGposX%d BGposY%d bgTypeposY%d bgUse%d field%d", g_CountFPS, (int)player->pos.x, (int)player->pos.y, (int)(enemy[0].use), (int)(bg->pos.x), (int)(bg->pos.y), (int)(bgtype[0].pos.y), (int)(bgtype[0].use), (int)(field->checkPtCnt));
#endif

				Update();			// �X�V����
				Draw();				// �`�揈��

#ifdef _DEBUG	// �f�o�b�O�ł̎������\������
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], " MX:%d MY:%d", GetMousePosX(), GetMousePosY());
				SetWindowText(hWnd, g_DebugStr);
#endif

				dwFrameCount++;
			}
		}
	}

	timeEndPeriod(1);				// ����\��߂�

	// �E�B���h�E�N���X�̓o�^������
	UnregisterClass(CLASS_NAME, wcex.hInstance);

	// �I������
	Uninit();

	return (int)msg.wParam;
}

//=============================================================================
// �v���V�[�W��
//=============================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		}
		break;

	case WM_MOUSEMOVE:
		g_MouseX = LOWORD(lParam);
		g_MouseY = HIWORD(lParam);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//=============================================================================
// ����������
//=============================================================================
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	// �`��̏�����
	InitRenderer(hInstance, hWnd, bWindow);

	// �J�����̏�����
	InitCamera();

	// ���C�g��L����
	SetLightEnable(FALSE);

	// �w�ʃ|���S�����J�����O
	SetCullingMode(CULL_MODE_BACK);

	// ���͏����̏�����
	InitInput(hInstance, hWnd);

	// �T�E���h�����̏�����
	InitSound(hWnd);

	// �t�F�[�h�����̏�����
	InitFade();

	// �ŏ��̃��[�h���Z�b�g
	SetMode(g_Mode);	// ������SetMode�̂܂܂ŁI

	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void Uninit(void)
{
	// �I���̃��[�h���Z�b�g
	SetMode(MODE_MAX);

	// �t�F�[�h�̏I������
	UninitFade();

	// �T�E���h�̏I������
	UninitSound();

	// ���͂̏I������
	UninitInput();

	// �J�����̏I������
	UninitCamera();

	// �����_���[�̏I������
	UninitRenderer();
}

//=============================================================================
// �X�V����
//=============================================================================
void Update(void)
{
	// ���͂̍X�V����
	UpdateInput();

	// �J�����X�V
	UpdateCamera();

	// ���[�h�ɂ���ď����𕪂���
	switch (g_Mode)
	{
	case MODE_TITLE:		// �^�C�g����ʂ̍X�V
		UpdateTitle();
		break;

	case MODE_CREDIT:		// �N���W�b�g��ʂ̍X�V
		UpdateCredit();
		break;

	case MODE_GAME:			// �Q�[����ʂ̍X�V
		UpdateBG();
		UpdateField();
		UpdatePlayer();
		UpdateGuide();		// �K�C�h�X�V
		UpdateEnemy();
		UpdateBoss();
		UpdatePlayerBullet();
		UpdateEnemyBullet();
		UpdateEffect();
		UpdateStatus();

		break;

	case MODE_RESULT:		// ���U���g��ʂ̍X�V
		UpdateResult();
		break;
	}

	UpdateFade();			// �t�F�[�h�̍X�V����
}

//=============================================================================
// �`�揈��
//=============================================================================
void Draw(void)
{
	// �o�b�N�o�b�t�@�N���A
	Clear();

	SetCamera();

	// 2D�̕���`�悷�鏈��
	SetViewPort(TYPE_FULL_SCREEN);

	// Z��r�Ȃ�
	SetDepthEnable(FALSE);

	// ���C�e�B���O�𖳌�
	SetLightEnable(FALSE);


	// ���[�h�ɂ���ď����𕪂���
	switch (g_Mode)
	{
	case MODE_TITLE:		// �^�C�g����ʂ̕`��
		DrawTitle();
		break;

	case MODE_CREDIT:		// �N���W�b�g��ʂ̕`��
		DrawCredit();
		break;

	case MODE_GAME:			// �Q�[����ʂ̕`��
		DrawBG();
		DrawField();
		DrawEnemyBullet();		// �d�Ȃ鏇�Ԃ��ӎ����Ă�
		DrawPlayerBullet();
		DrawEnemy();
		DrawBoss();
		DrawGuide();		
		DrawPlayer();
		DrawEffect();
		DrawStatus();
		break;

	case MODE_RESULT:		// ���U���g��ʂ̕`��
		DrawResult();
		break;
	}


	DrawFade();				// �t�F�[�h��ʂ̕`��


#ifdef _DEBUG
	// �f�o�b�O�\��
	DrawDebugProc();
#endif

	// �o�b�N�o�b�t�@�A�t�����g�o�b�t�@����ւ�
	Present();
}


long GetMousePosX(void)
{
	return g_MouseX;
}


long GetMousePosY(void)
{
	return g_MouseY;
}


#ifdef _DEBUG
char* GetDebugStr(void)
{
	return g_DebugStr;
}
#endif



//=============================================================================
// ���[�h�̐ݒ�
//=============================================================================
void SetMode(int mode)
{
	// ���[�h��ς���O�ɑS������������������Ⴄ
	StopSound();			// �܂��Ȃ��~�߂�

	// ���[�h��ς���O�ɑS������������������Ⴄ

	// �^�C�g����ʂ̏I������
	UninitTitle();

	// �N���W�b�g�̏I������
	UninitCredit();

	// �K�C�h�̏I������
	UninitGuide();

	// BG�̏I������
	UninitBG();

	// field�̏I������
	UninitField();

	// �v���C���[�̏I������
	UninitPlayer();

	// �G�l�~�[�̏I������
	UninitEnemy();
	UninitBoss();

	// �o���b�g�̏I������
	UninitPlayerBullet();
	UninitEnemyBullet();

	// �X�e�[�^�X�̏I������
	UninitStatus();

	// ���U���g�̏I������
	UninitResult();

	// �G�t�F�N�g�̏I������
	UninitEffect();


	g_Mode = mode;	// ���̃��[�h���Z�b�g���Ă���

	switch (g_Mode)
	{
	case MODE_TITLE:
		// �^�C�g����ʂ̏�����
		InitTitle();
		PlaySound(SOUND_LABEL_BGM_TITLE);
		break;

	case MODE_CREDIT:
		// �^�C�g����ʂ̏�����
		InitCredit();
		PlaySound(SOUND_LABEL_BGM_Credit);
		break;

	case MODE_GAME:
		// �Q�[����ʂ̏�����
		InitGuide();
		InitBG();
		InitField();
		InitPlayer();
		InitEnemy();
		InitBoss();
		InitPlayerBullet();
		InitEnemyBullet();
		InitEffect();
		InitStatus();

		break;

	case MODE_RESULT:
		InitResult();
		PlaySound(SOUND_LABEL_BGM_RESULT);
		break;

	case MODE_MAX:
		break;
	}
}

//=============================================================================
// ���[�h�̎擾
//=============================================================================
int GetMode(void)
{
	return g_Mode;
}

//=============================================================================
// pos�Ȃ�float3�^�̒l���g���x�W�F�Ȑ�
//=============================================================================
// 1���x�W�F�Ȑ�
XMVECTOR LinearInterpolation(XMVECTOR P0, XMVECTOR  P1, float s)
{
	XMVECTOR A0;
	A0 = P0 + (P1 - P0) * s;
	return A0;
}

// 2���x�W�F�Ȑ�
XMVECTOR QuadraticBezierCurve(XMVECTOR P0, XMVECTOR P1, XMVECTOR P2, float s)
{
	XMVECTOR A0;
	A0 = LinearInterpolation(P0, P1, s);

	XMVECTOR A1;
	A1 = LinearInterpolation(P1, P2, s);

	XMVECTOR B0;
	B0 = LinearInterpolation(A0, A1, s);

	return B0;
}


//=============================================================================
// �v�Z���p�̒P���ȃx�W�F�Ȑ�
//=============================================================================
// 1���x�W�F�Ȑ�
float LinearInterpolation_single(float P0, float  P1, float s)
{
	float A0;
	A0 = P0 + (P1 - P0) * s;
	return A0;
}

// 2���x�W�F�Ȑ�
float QuadraticBezierCurve_single(float P0, float P1, float P2, float s)
{
	float A0;
	A0 = LinearInterpolation_single(P0, P1, s);

	float A1;
	A1 = LinearInterpolation_single(P1, P2, s);

	float B0;
	B0 = LinearInterpolation_single(A0, A1, s);

	return B0;
}

//=============================================================================
// �Q�[���X�s�[�h��ς���
//=============================================================================
void SetFps(int value)
{
	g_fps = value;
}

//=============================================================================
// �^�C�}�[���X�^�[�g������
//=============================================================================
void SetTimer(void)
{
	if (g_dwGameStartTime == 0)					// �ŏ���1��̂݃^�C�}�[���n�߂�
	{
		g_dwGameStartTime = timeGetTime();
	}
}

DWORD GetTimer(void)
{
	return g_dwGameProgressTime;
}