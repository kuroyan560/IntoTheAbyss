#include "Camera.h"

void Camera::Init()
{
	active = 0;
	scrollAffect = { 0,0 };
	//zoomAffect = 0.0f;
	zoom = 1.0f;
}

#include"ScrollMgr.h"
#include"ShakeMgr.h"
#include"KuroMath.h"
#include"WinApp.h"
void Camera::Update()
{
	if (active)
	{
		//描画上の位置を求める
		const auto targetOnDraw = ScrollManager::Instance()->Affect(target);
		//画面中央との差分を求める
		const auto differ =  targetOnDraw - WinApp::Instance()->GetExpandWinCenter() - scrollAffect;

		//近づいていく
		scrollAffect = KuroMath::Lerp(scrollAffect, -differ, 0.1f);
	}
	else
	{
		scrollAffect = KuroMath::Lerp(scrollAffect, { 0,0 }, 0.1f);
	}

	ScrollManager::Instance()->zoom = KuroMath::Lerp(ScrollManager::Instance()->zoom, zoom, 0.1f);
}

void Camera::Focus(const Vec2<float>& TargetPos, const float& Zoom)
{
	target = TargetPos;
	zoom = Zoom;
	active = 1;
}
