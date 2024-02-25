//=============================================================================
//
// エネミーからのバレット処理 [enemyBullet.h]
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
#define BULLET_MAX				(100)	// 画面に表示する最大数

enum bulletType
{
	BULLET_PATT_SPREAD = 0,		// 360度に広がっていく軌道
	BULLET_PATT_HORIZONTAL,		// 左右に広がる軌道
	BULLET_PATT_BOSSCLIMAX,		// ボスのクライマックス
	BULLET_PATT_SINGLE,		// シングルショット
	BULLET_PATT_ALL,
};


//*****************************************************************************
// 構造体定義
//*****************************************************************************


// バレット自体
struct BULLET
{
	BOOL				left;				// 画像反転フラグ
	BOOL				use;				// true:使っている  false:未使用
	XMFLOAT3			pos;				// バレットの座標
	XMFLOAT3			rot;				// バレットの回転量
	XMFLOAT3			move;				// バレットの移動量
	float				w, h;				// 幅と高さ
	float				damage;				// プレイヤーに与えるダメージ量
	float				speed;				// スピード
	int					texNo;				// どのテクスチャを使うか
	int					cntAnime;			// アニメーションカウント
	int					type;				// どのタイプのBulletか
};

// バレットのタイプ
struct BULLET_PATTERN
{
	float				w, h;				// 幅と高さ
	float				damage;				// プレイヤーに与えるダメージ量
	float				speed;				// スピード
	int					max;				// 最大弾数
	int					texNo;				// どのテクスチャを使うか
	int					x,y;				// アニメーションパターン数
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitEnemyBullet(void);
void UninitEnemyBullet(void);
void UpdateEnemyBullet(void);
void DrawEnemyBullet(void);

// BULLET *GetBullet(void);

void SetEnemyBullet(XMFLOAT3 pos, int type);
