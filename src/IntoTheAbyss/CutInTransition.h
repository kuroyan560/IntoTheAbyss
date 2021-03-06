#pragma once
#include "KuroEngine.h"
#include <array>

class CutInTransition : public SceneTransition {
public:
	CutInTransition();
	~CutInTransition();
	void OnStart()override;
	bool OnUpdate()override;
	void OnDraw()override;

	inline bool GetNowTransition() { return isNowTransition; }

	std::shared_ptr<RenderTarget>backGroundTex;
private:

	std::array<int, 3> lunaHandle;			// ノックアウト時のLuna(左側のキャラ)の画像ハンドル アニメーション
	std::array<int, 3> lacyHandle;			// ノックアウト時のLuna(左側のキャラ)の画像ハンドル アニメーション
	Vec2<float>charaPos;					// マスクの内側にいるキャラの座標
	Vec2<float>titlePos;
	int lunaAnimHandle;
	int knockOutTimer;
	bool isEnd;
	bool isLeftChara;
	bool isNowTransition;
	int backAlpha;		// 後ろに描画する黒のアルファ値

	// ノックアウト時のステータス
	enum class KNOCK_OUT_PHASE {

		START_PHASE,		// 開始フェーズ
		STOP_PHASE,			// キャラカットイン中の一時停止フェーズ
		END_PHASE,			// 終了フェーズ

	};
	KNOCK_OUT_PHASE knockOutPhase;

	Vec2<float>pos, backGroundPos;
};
