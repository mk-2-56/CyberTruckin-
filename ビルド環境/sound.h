//=============================================================================
//
// サウンド処理 [sound.h]
// Author : GP11A132 15 高橋ほの香
//
//=============================================================================
#pragma once

#include <windows.h>
#include "xaudio2.h"

//*****************************************************************************
// サウンドファイル
//*****************************************************************************
enum 
{
	SOUND_LABEL_BGM_TITLE,		// BGM0
	SOUND_LABEL_BGM_BOSS,		// BGM1
	SOUND_LABEL_BGM_GAME,		// BGM2
	SOUND_LABEL_BGM_Credit,		// クレジットBGM
	SOUND_LABEL_BGM_RESULT,		// リザルトBGM
	SOUND_LABEL_SE_shot00,		// 銃発射音
	SOUND_LABEL_SE_attack00,	// 敵に当たった音
	SOUND_LABEL_SE_damage00,	// プレイヤーがダメージをくらう音
	SOUND_LABEL_SE_playerDead00,// プレイヤーが死んだときの音
	SOUND_LABEL_SE_bombTimer,	// 時限爆弾のタイマー
	SOUND_LABEL_SE_bomb001,		// 時限爆弾の爆発
	SOUND_LABEL_SE_bossDead,	// ボスが死亡
	SOUND_LABEL_SE_gameClear,	// ゲームクリア
	SOUND_LABEL_SE_gameClear2,	// ゲームクリア2
	SOUND_LABEL_SE_gameStart,	// ゲームスタート
	SOUND_LABEL_SE_titleClick,	// スタート音
	SOUND_LABEL_SE_menuSelect,	// カーソル移動音
	SOUND_LABEL_SE_warning,		// warning

	SOUND_LABEL_MAX,
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
bool InitSound(HWND hWnd);
void UninitSound(void);
void PlaySound(int label);
void StopSound(int label);
void StopSound(void);

void SetVolumeSound(float volume);
void SetVolumeSound(int label, float volume);

void SetFrequencyRatio(float pitch);
void SetFrequencyRatio(int label, float pitch);