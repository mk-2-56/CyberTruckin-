//=============================================================================
//
// メイン処理 [main.cpp]
// Author : GP11A132 15 高橋ほの香
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
// マクロ定義
//*****************************************************************************
#define CLASS_NAME		"AppClass"			// ウインドウのクラス名
#define WINDOW_NAME		"Cyber Truckin'"	// ウインドウのキャプション名

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void Uninit(void);
void Update(void);
void Draw(void);


//*****************************************************************************
// グローバル変数:
//*****************************************************************************
long g_MouseX = 0;
long g_MouseY = 0;


#ifdef _DEBUG
int		g_CountFPS;							// FPSカウンタ
char	g_DebugStr[2048] = WINDOW_NAME;		// デバッグ文字表示用

#endif

int	g_Mode = MODE_TITLE;					// 起動時の画面を設定
DWORD g_dwGameStartTime = 0;				// ゲーム開始時の時間
DWORD g_dwGameProgressTime;					// ゲームの経過時間
unsigned int g_fps = FPS_VALUE;


//=============================================================================
// メイン関数
//=============================================================================
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);	// 無くても良いけど、警告が出る（未使用宣言）
	UNREFERENCED_PARAMETER(lpCmdLine);		// 無くても良いけど、警告が出る（未使用宣言）

	// 時間計測用
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
	
	// ウィンドウクラスの登録
	RegisterClassEx(&wcex);

	// ウィンドウの作成
	hWnd = CreateWindow(CLASS_NAME,
		WINDOW_NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,																		// ウィンドウの左座標
		CW_USEDEFAULT,																		// ウィンドウの上座標
		SCREEN_WIDTH + GetSystemMetrics(SM_CXDLGFRAME) * 2,									// ウィンドウ横幅
		SCREEN_HEIGHT + GetSystemMetrics(SM_CXDLGFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),	// ウィンドウ縦幅
		NULL,
		NULL,
		hInstance,
		NULL);

	// ウィンドウモードかフルスクリーンモードかの処理
	BOOL mode = TRUE;

	int id = MessageBox(NULL, "Windowモードでプレイしますか？", "起動モード", MB_YESNOCANCEL | MB_ICONQUESTION);
	switch (id)
	{
	case IDYES:		// YesならWindowモードで起動
		mode = TRUE;
		break;
	case IDNO:		// Noならフルスクリーンモードで起動
		mode = FALSE;	// 環境によって動かない事がある
		break;
	case IDCANCEL:	// CANCELなら終了
	default:
		return -1;
		break;
	}

	// 初期化処理(ウィンドウを作成してから行う)
	if(FAILED(Init(hInstance, hWnd, mode)))
	{
		return -1;
	}

	// フレームカウント初期化
	timeBeginPeriod(1);	// 分解能を設定
	dwExecLastTime = dwFPSLastTime = timeGetTime();	// システム時刻をミリ秒単位で取得
	dwCurrentTime = dwFrameCount = 0;

	// ウインドウの表示(初期化処理の後に呼ばないと駄目)
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	// メッセージループ
	while(1)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{// PostQuitMessage()が呼ばれたらループ終了
				break;
			}
			else
			{
				// メッセージの翻訳と送出
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }
		else
		{
			dwCurrentTime = timeGetTime();

			g_dwGameProgressTime = dwCurrentTime - g_dwGameStartTime;		// ゲームが開始してからの時間

			if ((dwCurrentTime - dwFPSLastTime) >= 1000)	// 1秒ごとに実行
			{
#ifdef _DEBUG
				g_CountFPS = dwFrameCount;
#endif
				dwFPSLastTime = dwCurrentTime;				// FPSを測定した時刻を保存
				dwFrameCount = 0;							// カウントをクリア
			}

			if ((dwCurrentTime - dwExecLastTime) >= (1000 / g_fps))	// 1/60秒ごとに実行
			{
				dwExecLastTime = dwCurrentTime;	// 処理した時刻を保存

#ifdef _DEBUG	// デバッグ版の時だけFPSを表示する
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

				Update();			// 更新処理
				Draw();				// 描画処理

#ifdef _DEBUG	// デバッグ版の時だけ表示する
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], " MX:%d MY:%d", GetMousePosX(), GetMousePosY());
				SetWindowText(hWnd, g_DebugStr);
#endif

				dwFrameCount++;
			}
		}
	}

	timeEndPeriod(1);				// 分解能を戻す

	// ウィンドウクラスの登録を解除
	UnregisterClass(CLASS_NAME, wcex.hInstance);

	// 終了処理
	Uninit();

	return (int)msg.wParam;
}

//=============================================================================
// プロシージャ
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
// 初期化処理
//=============================================================================
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	// 描画の初期化
	InitRenderer(hInstance, hWnd, bWindow);

	// カメラの初期化
	InitCamera();

	// ライトを有効化
	SetLightEnable(FALSE);

	// 背面ポリゴンをカリング
	SetCullingMode(CULL_MODE_BACK);

	// 入力処理の初期化
	InitInput(hInstance, hWnd);

	// サウンド処理の初期化
	InitSound(hWnd);

	// フェード処理の初期化
	InitFade();

	// 最初のモードをセット
	SetMode(g_Mode);	// ここはSetModeのままで！

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void Uninit(void)
{
	// 終了のモードをセット
	SetMode(MODE_MAX);

	// フェードの終了処理
	UninitFade();

	// サウンドの終了処理
	UninitSound();

	// 入力の終了処理
	UninitInput();

	// カメラの終了処理
	UninitCamera();

	// レンダラーの終了処理
	UninitRenderer();
}

//=============================================================================
// 更新処理
//=============================================================================
void Update(void)
{
	// 入力の更新処理
	UpdateInput();

	// カメラ更新
	UpdateCamera();

	// モードによって処理を分ける
	switch (g_Mode)
	{
	case MODE_TITLE:		// タイトル画面の更新
		UpdateTitle();
		break;

	case MODE_CREDIT:		// クレジット画面の更新
		UpdateCredit();
		break;

	case MODE_GAME:			// ゲーム画面の更新
		UpdateBG();
		UpdateField();
		UpdatePlayer();
		UpdateGuide();		// ガイド更新
		UpdateEnemy();
		UpdateBoss();
		UpdatePlayerBullet();
		UpdateEnemyBullet();
		UpdateEffect();
		UpdateStatus();

		break;

	case MODE_RESULT:		// リザルト画面の更新
		UpdateResult();
		break;
	}

	UpdateFade();			// フェードの更新処理
}

//=============================================================================
// 描画処理
//=============================================================================
void Draw(void)
{
	// バックバッファクリア
	Clear();

	SetCamera();

	// 2Dの物を描画する処理
	SetViewPort(TYPE_FULL_SCREEN);

	// Z比較なし
	SetDepthEnable(FALSE);

	// ライティングを無効
	SetLightEnable(FALSE);


	// モードによって処理を分ける
	switch (g_Mode)
	{
	case MODE_TITLE:		// タイトル画面の描画
		DrawTitle();
		break;

	case MODE_CREDIT:		// クレジット画面の描画
		DrawCredit();
		break;

	case MODE_GAME:			// ゲーム画面の描画
		DrawBG();
		DrawField();
		DrawEnemyBullet();		// 重なる順番を意識してね
		DrawPlayerBullet();
		DrawEnemy();
		DrawBoss();
		DrawGuide();		
		DrawPlayer();
		DrawEffect();
		DrawStatus();
		break;

	case MODE_RESULT:		// リザルト画面の描画
		DrawResult();
		break;
	}


	DrawFade();				// フェード画面の描画


#ifdef _DEBUG
	// デバッグ表示
	DrawDebugProc();
#endif

	// バックバッファ、フロントバッファ入れ替え
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
// モードの設定
//=============================================================================
void SetMode(int mode)
{
	// モードを変える前に全部メモリを解放しちゃう
	StopSound();			// まず曲を止める

	// モードを変える前に全部メモリを解放しちゃう

	// タイトル画面の終了処理
	UninitTitle();

	// クレジットの終了処理
	UninitCredit();

	// ガイドの終了処理
	UninitGuide();

	// BGの終了処理
	UninitBG();

	// fieldの終了処理
	UninitField();

	// プレイヤーの終了処理
	UninitPlayer();

	// エネミーの終了処理
	UninitEnemy();
	UninitBoss();

	// バレットの終了処理
	UninitPlayerBullet();
	UninitEnemyBullet();

	// ステータスの終了処理
	UninitStatus();

	// リザルトの終了処理
	UninitResult();

	// エフェクトの終了処理
	UninitEffect();


	g_Mode = mode;	// 次のモードをセットしている

	switch (g_Mode)
	{
	case MODE_TITLE:
		// タイトル画面の初期化
		InitTitle();
		PlaySound(SOUND_LABEL_BGM_TITLE);
		break;

	case MODE_CREDIT:
		// タイトル画面の初期化
		InitCredit();
		PlaySound(SOUND_LABEL_BGM_Credit);
		break;

	case MODE_GAME:
		// ゲーム画面の初期化
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
// モードの取得
//=============================================================================
int GetMode(void)
{
	return g_Mode;
}

//=============================================================================
// posなどfloat3型の値を使うベジェ曲線
//=============================================================================
// 1次ベジェ曲線
XMVECTOR LinearInterpolation(XMVECTOR P0, XMVECTOR  P1, float s)
{
	XMVECTOR A0;
	A0 = P0 + (P1 - P0) * s;
	return A0;
}

// 2次ベジェ曲線
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
// 計算式用の単純なベジェ曲線
//=============================================================================
// 1次ベジェ曲線
float LinearInterpolation_single(float P0, float  P1, float s)
{
	float A0;
	A0 = P0 + (P1 - P0) * s;
	return A0;
}

// 2次ベジェ曲線
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
// ゲームスピードを変える
//=============================================================================
void SetFps(int value)
{
	g_fps = value;
}

//=============================================================================
// タイマーをスタートさせる
//=============================================================================
void SetTimer(void)
{
	if (g_dwGameStartTime == 0)					// 最初の1回のみタイマーを始める
	{
		g_dwGameStartTime = timeGetTime();
	}
}

DWORD GetTimer(void)
{
	return g_dwGameProgressTime;
}