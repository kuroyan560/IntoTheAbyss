#pragma once
#include"../Common/Vec.h"
#include<array>
#include"../Engine/DrawFunc.h"
#include"../Engine/Sprite.h"

class ScreenEdge
{
public:
	ScreenEdge();

	void Init(const float &DISTANCE, const Vec2<float> INIT_POS, const float ANGLE);
	void StartWinEffect(int FADE_OUT_TIMER = 120);
	void StartFinishEffect();
	void CaluPos(const float &RATE);

	void Update();
	void Draw();

private:
	float maxDistance;
	Vec2<float>basePos;
	Vec2<float>pos;
	Vec2<float>lerpPos;

	std::array<Vec2<float>, 2> scrollChancePos;
	Vec2<float> vel;
	Vec2<float> speed;
	float uv;

	Vec2<float>shakeValue;
	float shakeRate;
	bool reversFlag;

	float angle;
	Vec2<float> distance;

	enum VEC_TYPE{ X_VEC, Y_VEC };
	enum WIN_TYPE{ WIN_LEFT, WIN_RIGHT };
	VEC_TYPE vecType;
	WIN_TYPE winType;

	float rate;

	float chanceAlpha;
	float sinCurve;

	bool winFlag;
	bool getAwayFlag;
	bool initWinFlag;
	bool initGetAwayFlag;
	int initTimer;
	int fadeOutMaxTimer;
	Vec2<float> winPos;

	Vec2<float>adjPos;
	Vec2<float>adjTexPos;
	Vec2<float>adjYPos;

	//画像ハンドル-----------------------
	std::array<Sprite, 2> chanceRender;

	int playerChancehandle;
	int playerFlamehandle;

	int enemyChancehandle;
	int enemyFlamehandle;

	int nowFlameHandle;
	int nowChanceHandle;
	//画像ハンドル-----------------------


	void Rate(float *RATE, const float &MAX_RATE)
	{
		*RATE += 1.0f / MAX_RATE;
		if (1.0f <= *RATE)
		{
			*RATE = 1.0f;
		}
	}
	void MRate(float *RATE, const float &MAX_RATE)
	{
		*RATE += -1.0f / MAX_RATE;
		if (*RATE <= 0.0f)
		{
			*RATE = 0.0f;
		}
	}

};

