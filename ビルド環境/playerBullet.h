//=============================================================================
//
// プレイヤーからのバレット処理 [playerBullet.h]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define BULLET_PLAYER_MAX				(3)	// バレット最大数


// バレット構造体
struct PLAYER_BULLET
{
	BOOL				use;				// true:使っている  false:未使用
	XMFLOAT3			pos;				// バレットの座標
	float				w, h;				// 幅と高さ
	XMFLOAT3			rot;				// バレットの回転量
	XMFLOAT3			move;				// バレットの移動量
	int					texNo;				// どのテクスチャを使うか
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitPlayerBullet(void);
void UninitPlayerBullet(void);
void UpdatePlayerBullet(void);
void DrawPlayerBullet(void);

PLAYER_BULLET* GetPlayerBullet(void);

void SetPlayerBullet(XMFLOAT3 pos);
