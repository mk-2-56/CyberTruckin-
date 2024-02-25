//=============================================================================
//
// メイン処理 [main.h]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#pragma once


#pragma warning(push)
#pragma warning(disable:4005)

#define _CRT_SECURE_NO_WARNINGS			// scanf のwarning防止
#include <stdio.h>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>

#include <DirectXMath.h>


// 本来はヘッダに書かない方が良い
using namespace DirectX;


#define DIRECTINPUT_VERSION 0x0800		// 警告対処
#include "dinput.h"
#include "mmsystem.h"

#pragma warning(pop)


//*****************************************************************************
// ライブラリのリンク
//*****************************************************************************
#pragma comment (lib, "d3d11.lib")		
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "d3dx11.lib")	
#pragma comment (lib, "d3dx9.lib")	
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "dxerr.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "dinput8.lib")


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define FPS_VALUE		 (60)			// FPS
#define SCREEN_WIDTH	 (1680)			// ウインドウの幅
#define SCREEN_HEIGHT	 (900)			// ウインドウの高さ
#define OUTSIDE_WIDTH	 (240)			// 画面外ステータスの表示幅
#define GAMESCREEN_WIDTH (SCREEN_WIDTH - OUTSIDE_WIDTH * 2)
#define SCREEN_CENTER_X	 (SCREEN_WIDTH / 2)	// ウインドウの中心Ｘ座標
#define SCREEN_CENTER_Y	 (SCREEN_HEIGHT / 2)	// ウインドウの中心Ｙ座標

#define	MAP_W			 (1280.0f)
#define	MAP_H			 (1280.0f)
#define	MAP_TOP			 (MAP_H/2)
#define	MAP_DOWN		 (-MAP_H/2)
#define	MAP_LEFT		 (-MAP_W/2)
#define	MAP_RIGHT		 (MAP_W/2)

// 補間用のデータ構造体を定義
struct INTERPOLATION_DATA
{
	XMFLOAT3	pos;		// 頂点座標
	XMFLOAT3	rot;		// 回転
	XMFLOAT3	scl;		// 拡大縮小
	float		frame;		// 実行フレーム数 ( dt = 1.0f/frame )
};

enum
{
	MODE_TITLE = 0,			// タイトル画面
	MODE_GAME,				// ゲーム画面
	MODE_CREDIT,			// クレジット画面
	MODE_RESULT,			// リザルト画面
	MODE_MAX
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
long GetMousePosX(void);
long GetMousePosY(void);
char* GetDebugStr(void);

void SetMode(int mode);
int GetMode(void);

void SetLoadGame(BOOL flg);

XMVECTOR LinearInterpolation(XMVECTOR P0, XMVECTOR  P1, float s);
XMVECTOR QuadraticBezierCurve(XMVECTOR P0, XMVECTOR P1, XMVECTOR P2, float s);

float LinearInterpolation_single(float P0, float  P1, float s);
float QuadraticBezierCurve_single(float P0, float P1, float P2, float s);

void SetFps(int value);

void SetTimer(void);

DWORD GetTimer(void);