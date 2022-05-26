#pragma once
#include "Vec.h"
#include "CharacterManager.h"
#include "Camera.h"
#include "Singleton.h"

#include "RoundFinishParticle.h"

class RoundFinishEffect : public Singleton<RoundFinishEffect> {

public:

	/*===== メンバ変数 =====*/

	enum class EFFECT_STATUS {

		NUM1_ZOOMIN,			// �@二人の真ん中にズームインする。
		NUM2_ENEMY_SHAKE,		// �A敵をシェイクさせる。
		NUM3_ENEMY_EXP,			// �B敵を爆発させる。
		NUM4_RETURN_DEFPOS,		// �D各キャラを規定の座標に戻す。
		NUM5_RETURN_PLAYER_DEFPOS,

	};

	EFFECT_STATUS status;		// ステータス

	int timer;					// 各タイマー
	bool isEnd;					// 演出が終わったかのフラグ
	Vec2<float> shakeAmount;	// シェイクさせる処理全般に使用するやつ

	// 各ステータスの時間
	const int NUM1_ZOOMIN_TIMER = 90;
	const int NUM2_ENEMY_SHAKE_TIMER = 120;
	const int NUM3_ENEMY_EXP_TIMER = 240;
	const int NUM4_RETURN_DEFPOS_TIMER = 120;
	const int NUM5_RETURN_PLAYER_DEF_POS = 120;



public:

	// コンストラクタ
	RoundFinishEffect();

	// 初期化処理
	void Init();

	// 開始処理
	void Start();

	// 更新処理
	void Update(const Vec2<float>& LineCenterPos);

	// 描画処理
	void Draw();

};