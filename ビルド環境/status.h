//=============================================================================
//
// ステータス処理 [status.h]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define STATUS_SCORE_MAX			(99999)		// スコアの最大値
#define STATUS_SCORE_DIGIT			(5)			// 桁数
#define STATUS_RETRY_MAX			(99)		// リトライの最大値
#define STATUS_RETRY_DIGIT			(2)			// 桁数
#define STATUS_TIME_MAX				(995999)	// 時間の最大値
#define STATUS_TIME_DIGIT			(2)			// 桁数
#define TIME_INTERVAL1				(53)	// 分数と秒数の間の間隔
#define TIME_INTERVAL2				(115)	// 分数と秒数の間の間隔

// ステータスの種類
enum STATUSOBJ
{
	STATUS_SCORE = 0,		// スコア
	STATUS_RETRY,			// リトライ回数
	STATUS_TIME,			// 経過時間
	STATUS_TIME_MILLI,		// 経過時間(ミリ秒)
	STATUS_PLAYER,			// プレイヤーに関するステータス
	STATUS_PLAYER_HP,		// プレイヤーのHPに関するステータス
	STATUS_BOSS,			// ボスに関するステータス
	STATUS_BOSS_HP,			// ボスのHPに関するステータス
	STATUS_OUTSIDE,			// 画面外のステータス用背景
	STATUS_GAMECLEAR,		// ゲームクリア！
	STATUS_WARNING,			// warning
	STATUS_ALL,
};

// ゲームの進行具合
enum STATUSGAME
{
	STATUSGAME_DURING = 0,	// 進行中
	STATUSGAME_START,		// ゲームスタート時
	STATUSGAME_CLEAR,		// ゲームクリア時
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct STATUS
{
	BOOL		use;		// 
	XMFLOAT3	pos;		// ポリゴンの座標
	float		w, h;		// 幅と高さ
	int			texNo;		// 使用しているテクスチャ番号
	int			num;		// 値
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitStatus(void);
void UninitStatus(void);
void UpdateStatus(void);
void DrawStatus(void);

// スコア関係
void AddScore(int add);
int GetScore(void);/*
void SetScore(int score);*/

STATUS* GetStatusOBJ(void);

void SetStatus(int status);
int GetStatus(void);

void AddRetry(int add);