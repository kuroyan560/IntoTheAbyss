#pragma once
#include "Vec.h"
#include "Singleton.h"

class GameSceneCameraMove : public Singleton<GameSceneCameraMove> {

public:

	/*===== メンバ変数 =====*/

	Vec2<float> move;		// 移動させる量

	GameSceneCameraMove() { move = {}; }

};