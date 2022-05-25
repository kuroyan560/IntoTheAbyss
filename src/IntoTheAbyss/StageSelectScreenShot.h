#pragma once
#include"IStageSelectImage.h"
#include<array>

class StageSelectScreenShot
{
public:
	StageSelectScreenShot();
	void Init();
	void Update();
	void Draw();

	void Next();
	void Prev();

	void ImGuiDraw();

	void SetZoomFlag(const bool& Zoom);

	// 演出用の拡縮をセット。
	inline void SetExp(const Vec2<float>& expPos, const Vec2<float>& expRate) {
		expData.pos = expPos;
		expData.size = expRate;
	};

private:
	int selectNum;
	static const int STAGE_MAX_NUM = 10;
	std::array<int, STAGE_MAX_NUM> screenShotHandle;
	std::array<int, STAGE_MAX_NUM> stageNumberHandle;
	LerpData screenShotLerpData;
	LerpData stageNumberData;

	// 選択された時に拡縮するための変数
	LerpData expData;
	LerpData stageNumberExpData;

	bool zoomOutFlag;

	float timer;		// リサージュ曲線に使用するタイマー

};

