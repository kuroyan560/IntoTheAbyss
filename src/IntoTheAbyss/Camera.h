#pragma once
#include"Vec.h"
#include"Singleton.h"
class Camera : public Singleton<Camera>
{
	friend class Singleton<Camera>;
	Camera() {}
	friend class ScrollMgr;

	// If camera focuses on something, it will be active.
	int active;

	// Info to focus.
	Vec2<float>target;
	float zoom;

	// These parameters affect scroll and scale on "ScrollMgr".
	Vec2<float>scrollAffect;
	//float zoomAffect;

public:
	void Init();
	void Update();
	// This "TargetPos" must not be affected by scroll.
	void Focus(const Vec2<float>& TargetPos, const float& Zoom);
	void Release() { active = 0; zoom = 1.0f; }

	const int& Active() { return active; }
};