//=============================================================================
//
// プレイヤー処理 [player.h]
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
#define PLAYER_MAX			(1)			// プレイヤーのMax人数
#define	PLAYER_OFFSET_CNT	(16)		// 16分身
#define	PLAYER_HP_MAX		(100.0f)	// 最大HP
#define	PLAYER_HP_LAW		(20.0f)		// ピンチフラグ
#define	PLAYER_LOSE_HEIGHT	(25.0f)		// スライディングで減る高さ
#define	PLAYER_DAMAGE_MIN	(10.0f)		// プレイヤーに与えられるダメージ、弱いもの
#define LOSE_COLLISIONH_POS	(15.0f)		// スライディングで下げる判定


// プレイヤーの行動をカウントする用
enum PLAYER_COUNT
{
	PLAYER_COUNT_JUMP = 0,
	PLAYER_COUNT_DAMAGE,
	PLAYER_COUNT_ATTACK,
	PLAYER_COUNT_SLIDING,
	PLAYER_COUNT_DEAD,

	PLAYER_COUNT_ALL,
};

// アニメーションの種類
enum ANIME
{
	ANIME_NORMAL = 0,			// 通常
	ANIME_JUMP,					// ジャンプ
	ANIME_WALK,					// 歩き
	ANIME_HASHIGO,				// 昇り降り
	ANIME_ATTACK,				// 攻撃
	ANIME_ATTACKRUN,			// 攻撃(走りながら)
	ANIME_DAMAGE,				// ダメージを受ける
	ANIME_DOWN,					// 落下中
	ANIME_CRISIS,				// ピンチ(HPが少ない)
	ANIME_SLIDING,				// スライディング
	ANIME_ALL,
};

enum UPDOWN
{
	UPDOWN_FALSE = 0,
	UPDOWN_AVAILABLE,
	UPDOWN_DURING,
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct PLAYER
{
	XMFLOAT3	pos;			// ポリゴンの座標
	XMFLOAT3	energy;			// // 移動速度
	XMFLOAT3	pos_old;		// 移動前の座標
	XMFLOAT3	rot;			// ポリゴンの回転量
	BOOL		use;			// true:使っている  false:未使用
	BOOL		movable;		// true:操作可能  false:操作不可
	float		w, h;			// 幅と高さ
	float		collisionW, collisionH;			// 当たり判定用の幅と高さ
	float		hp;
	float		opacity;

	float		countAnim;		// アニメーションカウント
	int			patternAnim;	// アニメーションパターンナンバー
	int			pattern_old;	// ひとつ前のアニメーションパターン
	
	BOOL		left;			// 向き（true:左  false:右）
	BOOL		moving;			// キー入力があるかのフラグ

	BOOL		jump;			// ジャンプフラグ
	BOOL		damage;			// ダメージ中かのフラグ
	BOOL		attack;			// 攻撃フラグ
	BOOL		ground;			// 地面フラグ
	BOOL		sliding;		// スライディング中のフラグ

	float		jumpY;			// ジャンプの高さ
	float		gravityY;		// 重力
	int			updown;			// 0:昇り降りできない 1:昇り降りできる 2:昇り降り中
};

struct PLAYERANIM
{
	BOOL		loop;
	float		cntSpeed;		// アニメーションカウントを進めるスピード
	int			texNo;
	int			xMax;
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

PLAYER* GetPlayer(void);

void DamagePlayer(float damage);
