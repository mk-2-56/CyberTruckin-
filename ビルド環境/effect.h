//=============================================================================
//
// エフェクト処理 [effect.h]
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

#define EFFECT_NUM_PARTS 30		//エフェクト内パーティクル最大数
#define EFFECT_NUM_EFFECTS 10	//エフェクト最大数

enum effectType
{
	EFFECT_ENEMYDEAD = 0,	// エネミーが死んだとき
	EFFECT_ENEMYHIT,		// エネミーにヒット
	EFFECT_EXPLOSION,		// 爆発
	EFFECT_BOSS_FOLLOW,		// ボスダッシュの追従エフェクト
	EFFECT_HIT_WALL,		// プレイヤーの弾が壁に当たって消える
	EFFECT_PLAYERDEAD,		// プレイヤーが死んだ
	EFFECT_PLAYERATTACK,	// プレイヤー攻撃時のエフェクト
	EFFECT_BOSSDEAD,		// ボスが死んだ
	EFFECT_FIREGROUND,		// 地面が燃えてる
	EFFECT_SLIDING,		// スライディング
	EFFECT_ALL,
};


//*****************************************************************************
// 構造体宣言
//*****************************************************************************

typedef struct	// エフェクト構造体
{
	BOOL			reverse;					// 反転フラグ
	XMFLOAT3		pos;						// ポリゴンの移動量
	XMFLOAT3		move;						// 移動量
	float			time;						// 経過時間
	float			opacity;					// 透過率
	int				PatternAnim;				// アニメーションパターンナンバー
	int				CountAnim;					// アニメーションカウント

	int				liveTime;

	bool			isFinish;
}PARTICLE;

typedef struct	// エフェクト構造体
{
	BOOL			loop;						// ループ
	BOOL			use;
	int				isEnding;
	bool			isRemoveOnFinish;

	XMFLOAT3		pos;						// ポリゴンの移動量

	int				duration;
	int				elapsed;
	int				numFinish;

	int				effectCount;
	int				emitCounter;

	int				numParts;					// そのエフェクトのパーティクル数

	float			w,h;
	float			collisionW, collisionH;
	int				x,y;						// アニメーション分割数
	int				texNo;
	int				timeAnim;					// 切り替わるタイム数
	int				effectType;					// どのエフェクトか

	PARTICLE		pParticle[EFFECT_NUM_PARTS];

}EFFECT;

typedef struct	// エフェクトの種類の構造体
{
	BOOL			loop;						// ループするか
	BOOL			synthesis;					// true:加算合成 false:減算合成
	float			w,h;
	int				x, y;
	float			opacity;
	int				texNo;
	int				numParts;					// そのエフェクトのパーティクル数
	int				timeAnim;					// 切り替わるタイム数

}EFFECT_TYPE;


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitEffect(void);
void UninitEffect(void);
void UpdateEffect(void);
void DrawEffect(void);

EFFECT* GetEffect(void);
void SetEffect(XMFLOAT3 pos, int duration, int label);
