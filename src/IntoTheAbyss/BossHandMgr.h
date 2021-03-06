#pragma once
#include"BossHand.h"
#include<memory>

struct HandData
{
	Vec2<float>pos;
	float radius;
	float angle;
	float startAngle;
};

class BossHandMgr
{
public:
	BossHandMgr(int L_DEFAULT_HANDLE, int R_DEFAULT_HANDLE, int L_HOLD_HANDLE, int R_HOLD_HANDLE, bool SHAKE_FLAG);
	void Init(bool DEBUG = true);
	void InitRadius();
	void Update(const Vec2<float> &POS = {});
	void Draw();

	void Hold(const Vec2<float> &DIR, bool HOLD);

	void ImGuiDraw();
	std::unique_ptr<BossHand>leftHand, rightHand;

	HandData leftHandData, rightHandData;
	bool holdFlag;
	bool lockOnFlag;
	bool drawHandFlag;

	Vec2<float>centralPos, targetPos, nowSwingVec;

	float leftAngle, rightAngle;
	float endLeftAngleLerp, endRightAngleLerp;
	float endLeftAngleRadius, endRightAngleRadius;
	float holdRadian;

	bool initFlag;
	bool initStartFlag;
	bool debugFlag;

	int timer;
	float normalLeftRadian, normalRightRadian;

	Vec2<float>holdDir;
	bool startEffectFlag;
	bool initStartEffectFlag;

private:

	bool StartEffect();

};
