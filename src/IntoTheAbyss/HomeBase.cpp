#include "HomeBase.h"
#include"../Engine/ImguiApp.h"
#include"../Engine/DrawFunc.h"
#include"ScrollMgr.h"
#include"ShakeMgr.h"

int HomeBase::AREA_NUM = 0;

HomeBase::HomeBase()
{
	if (AREA_NUM == 0)
	{
		name = "PlayerHomeBase";
	}
	else
	{
		name = "EnemyHomeBase";
	}
	++AREA_NUM;
}

void HomeBase::Init(const Vec2<float> &LEFT_UP_POS, const Vec2<float> &RIGHT_DOWN_POS, const bool& LeftPlayer)
{
	leftUpPos = LEFT_UP_POS;
	rightDownPos = RIGHT_DOWN_POS;

	hitBox.size = rightDownPos - leftUpPos;
	centerPos = leftUpPos + (hitBox.size / 2.0f);
	hitBox.center = &centerPos;

	leftPlayer = LeftPlayer;
}

bool HomeBase::Collision(const Square &OBJ_A)
{
	return AreaCollider::Instance()->CheckHitArea(OBJ_A, hitBox);
}

void HomeBase::Draw()
{
	Vec2<float>drawLeftUpPos = ScrollMgr::Instance()->Affect(leftUpPos);
	Vec2<float>drawRightDownPos = ScrollMgr::Instance()->Affect(rightDownPos);
	
	static const int AREA_ALPHA = 100;
	static Color PLAYER_COLOR = Color(47, 255, 139, AREA_ALPHA);
	static Color ENEMY_COLOR = Color(239, 1, 144, AREA_ALPHA);

	//DrawFunc::DrawBox2D(drawLeftUpPos, drawRightDownPos, Color(255, 255, 255, 255), DXGI_FORMAT_R8G8B8A8_UNORM);
	DrawFunc::DrawBox2D(drawLeftUpPos, drawRightDownPos, leftPlayer ? PLAYER_COLOR : ENEMY_COLOR, DXGI_FORMAT_R8G8B8A8_UNORM, true, AlphaBlendMode_Trans);
}

void HomeBase::Debug()
{
	ImGui::Begin(name.c_str());
	ImGui::InputFloat("leftUpPosX", &leftUpPos.x);
	ImGui::InputFloat("leftUpPosY", &leftUpPos.y);
	ImGui::InputFloat("rightDownPosX", &rightDownPos.x);
	ImGui::InputFloat("rightDownPosY", &rightDownPos.y);
	ImGui::End();
}
