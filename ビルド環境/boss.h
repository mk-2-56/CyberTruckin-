//=============================================================================
//
// ボス処理 [boss.h]
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
#define BOSS_MAX		(1)		// ボスの最大数
#define BOSS_HP_MAX		(300)	// ボスのHP初期値

enum BossAction
{
	ACTION_NONE = 0,
	ACTION_DASH,
	ACTION_JUMP,
	ACTION_CLIMAX,
	ACTION_ALL,
};

enum ActionPhase
{
	PHASE_FIRST = 0,
	PHASE_SECOND,
	PHASE_THIRD,
	PHASE_FOURTH,
	PHASE_FIFTH,
	PHASE_ALL,
};

enum BossAnime
{
	BOSS_ANIM_NORMAL = 0,
	BOSS_ANIM_DASH,
	BOSS_ANIM_JUMP,
	BOSS_ANIM_WALL,
	BOSS_ANIM_DEATH,
	BOSS_ANIM_ALL,
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************

// ボスの構造体
struct BOSS
{
	BOOL				damage;				// 無敵時間かのフラグ
	BOOL				use;				// true:使っている  false:未使用
	BOOL				jump;				// ジャンプ中フラグ
	BOOL				left;				// どちらを向いてるか
	int					action;				// 攻撃パターン中フラグ
	int					phase;				// 攻撃パターンの段階
	XMFLOAT3			pos;				// ポリゴンの座標
	XMFLOAT3			energy;				// ボスにかかるエネルギーの総称
	XMFLOAT3			rot;				// ポリゴンの回転量
	XMFLOAT3			scl;				// ポリゴンの拡大縮小
	float				countAnim;			// アニメーションカウント
	float				time;				// 出現してからの時間
	float				w, h;				// 幅と高さ
	float				jumpY;				// ジャンプ力
	float				gravityY;			// ジャンプ力
	float				speed;				// スピード
	float				hp;					// HP
	float				opacity;			// 透過
	int					texNo;				// テクスチャ番号
	int					patternAnim;		// アニメーションパターンナンバー
};

struct BOSSANIM
{
	BOOL		loop;
	float		cntSpeed;		// アニメーションカウントを進めるスピード
	int			texNo;
	int			xMax;
};

struct BOSSACTION
{
	float		cntMax[PHASE_ALL];
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitBoss(void);
void UninitBoss(void);
void UpdateBoss(void);
void DrawBoss(void);

BOSS* GetBoss(void);

void DamageBoss(float damage);

void StartAction(int pattern);

void SetJump(XMFLOAT3 dirPos, XMFLOAT3 pos, BOOL stay, float power, int cntMax);

void SetIdle(void);