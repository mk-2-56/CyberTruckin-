//=============================================================================
//
// エネミー処理 [enemy.h]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define ENEMY_MAX		(10)

enum enemyType
{
	ENEMY_PATT_SHOT,			// 打ってくる敵
	ENEMY_PATT_BOMB,			// 爆弾
	ENEMY_PATT_BACKANDFORTH,	// 行ったり来たり
	ENEMY_PATT_SHOT_SINGLE,		// 一発打つ敵
	ENEMY_PATT_ALL,
};

enum enemyAnime
{
	ENEMY_ANIM_NORMAL = 0,
	ENEMY_ANIM_ATTACK,
	ENEMY_ANIM_ALL,
};

enum enemyUse
{
	ENEMY_USE_FALSE = 0,	// まだ使用してない
	ENEMY_USE_TRUE,			// 使用中
	ENEMY_USE_DONE,			// 使用した(もう使用しない)
	ENEMY_USE_REUSE,		// 使用したけど復活可能
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************

// エネミー自体
struct ENEMY
{
	BOOL				loop;				// 繰り返し登場するか
	XMFLOAT3			pos;				// ポリゴンの座標
	int					type;				// エネミーのタイプ


	BOOL				jump;				// ジャンプ中フラグ
	BOOL				wall;				// 壁に当たった
	XMFLOAT3			rot;				// ポリゴンの回転量
	XMFLOAT3			move;				// ポリゴンの移動量
	XMFLOAT3			scl;				// ポリゴンの拡大縮小
	float				countAnim;			// アニメーションカウント
	float				time;				// 出現してからの時間
	float				w, h;				// 幅と高さ
	float				jumpY;				// ジャンプ力
	int					use;				// 中身はenemyUseの内容
	int					texNo;				// テクスチャ番号
	int					patternAnim;		// アニメーションパターンナンバー
	int					hp;					// HP
};

struct ENEMY_ANIME
{
	BOOL		loop;
	int			x, y;
	float		speed;
};

// エネミーのタイプ
struct ENEMY_PATTERN
{
	BOOL			damage;						// ダメージを与えるか
	float			w, h;						// 幅と高さ
	int				texNo;						// テクスチャ番号
	int				hpMax;						// HPの初期値(max状態)
	ENEMY_ANIME		enemyAnime[ENEMY_ANIM_ALL];
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

ENEMY* GetEnemy(void);
ENEMY_PATTERN* GetEnemyPatt(void);

void SetEnemy(int label);
void DamageEnemy(int num, int damage);

void enemyShot(XMFLOAT3 pos, float time, int type);