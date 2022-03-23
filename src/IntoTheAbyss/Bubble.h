#pragma once
#include "Vec.h"

// BubbleClass
class Bubble {

public:

	/*===== メンバ変数 =====*/

	Vec2<float> pos;		// position
	float radius;			// Draw Radius
	int breakCoolTime;		// Resporn cool time
	bool isBreak;			// states ga break ka
	int graphHandle;


public:

	/*===== 定数 =====*/

	const float RADIUS = 35.0f;				// Draw and CheckHit Radius
	const int BREAK_COOL_TIME = 120;		// Resporn cool time
	const float OFFSET_SCALE = 32.0f;


public:

	/*===== メンバ関数 =====*/

	// constructor
	Bubble();

	// generate
	void Generate(const Vec2<float>& generatePos);

	// Initialize
	void Init();

	// Update
	void Update();

	// Draw
	void Draw();

};