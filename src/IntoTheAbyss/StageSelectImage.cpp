#include "StageSelectImage.h"
#include"../Engine/DrawFunc.h"
#include"../IntoTheAbyss/TexHandleMgr.h"
#include"IntoTheAbyss/StageSelectOffsetPosDebug.h"

StageSelectImage::StageSelectImage()
{
	backGroundHandle = TexHandleMgr::LoadGraph("resource/ChainCombat/select_scene/frame.png");

	backGroundLerpData.Init(Vec2<float>(640, 900), Vec2<float>(2.8f, 2.5f), Vec2<float>(1280.0f / 2.0f, 720.0f / 2.0f), Vec2<float>(1.0f, 1.0f));
	zoomOutFlag = false;
}

void StageSelectImage::Init(bool MOVE_FROM_GAME_FLAG)
{
	if (MOVE_FROM_GAME_FLAG)
	{
		startStageSelectPos = { 1280.0f / 2.0f,1000.0f };
		startStageSelectSize = { 3.8f,3.0f };
		backGroundLerpData.timer = 0.0f;
	}
	else
	{
		startStageSelectPos = { 640,900 };
		startStageSelectSize = { 2.8f,2.5f };
		backGroundLerpData.timer = 1.0f;
	}

	backGroundLerpData.Init(Vec2<float>(640, 900), Vec2<float>(2.8f, 2.5f), Vec2<float>(1280.0f / 2.0f, 720.0f / 2.0f), Vec2<float>(1.0f, 1.0f));
	expData.Init(Vec2<float>(0, 0), Vec2<float>(0, 0), Vec2<float>(0, 0), Vec2<float>(0, 0));
	zoomOutFlag = false;
}

void StageSelectImage::Update()
{

	if (zoomOutFlag)
	{
		backGroundLerpData.lerpPos = { 1280.0f / 2.0f,1000.0f };
		backGroundLerpData.lerpSize = { 3.8f,3.0f };
		backGroundLerpData.startPos = { 640,900 };
		backGroundLerpData.startSize = { 2.8f,2.5f };
	}
	else
	{
		backGroundLerpData.lerpPos = { 640,900 };
		backGroundLerpData.lerpSize = { 2.8f,2.5f };
		backGroundLerpData.startPos = startStageSelectPos;
		backGroundLerpData.startSize = startStageSelectSize;
	}
	backGroundLerpData.EaseInOut();

	// 拡縮のための大きさを0に戻す。
	expData.lerpPos = Vec2<float>(0, 0);
	expData.lerpSize = Vec2<float>(0, 0);
	expData.Lerp();
}

void StageSelectImage::Draw()
{
	Vec2<float> debugPos = StageSelectOffsetPosDebug::Instance()->pos;
	DrawFunc::DrawRotaGraph2D(backGroundLerpData.pos + expData.pos + debugPos, backGroundLerpData.size + expData.size, 0.0f, TexHandleMgr::GetTexBuffer(backGroundHandle));
}

void StageSelectImage::ImGuiDraw()
{
	ImGui::Begin("StageSelectImage2");
	ImGui::InputFloat("PosX", &backGroundLerpData.pos.x);
	ImGui::InputFloat("PosY", &backGroundLerpData.pos.y);
	ImGui::InputFloat("SizeX", &backGroundLerpData.size.x);
	ImGui::InputFloat("SizeY", &backGroundLerpData.size.y);
	ImGui::Checkbox("ZoomOut", &zoomOutFlag);
	ImGui::End();
}

void StageSelectImage::SetZoomFlag(const bool& Zoom)
{
	zoomOutFlag = Zoom;
	backGroundLerpData.timer = 0.0f;
}
