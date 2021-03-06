#pragma once
#include"IStageSelectImage.h"
#include"KazHelper.h"
#include<array>

class StageSelectScreenShot
{
public:
	StageSelectScreenShot(int* SelectNum);
	void Init(bool MOVE_FROM_GAME_FLAG);
	void Update();
	void Draw();

	void ImGuiDraw();

	void SetZoomFlag(const bool& Zoom);

	// 演出用の拡縮をセット。
	inline void SetExp(const Vec2<float>& expPos, const Vec2<float>& expRate) {
		expData.pos = expPos;
		expData.size = expRate;
	};

	int GetCanMaxSelectNum() 
	{
		//return min(screenShotHandle.size(), stageNumberHandle.size()); 
		return screenShotHandle.size();
	}

	float GetZoomChangeRate();

	std::shared_ptr<RenderTarget>screenShot;

	int stageNum;
private:
	std::vector<int>screenShotHandle;
	std::vector<int> stageNumberHandle;
	LerpData screenShotLerpData;
	LerpData stageNumberData;

	// 選択された時に拡縮するための変数
	LerpData expData;
	LerpData stageNumberExpData;

	bool zoomOutFlag;

	float timer;		// リサージュ曲線に使用するタイマー

	int* selectNumPtr;

	int stageTagHandle;
	int stageNumHandle;

	Vec2<float>startStageSelectSize;//ゲーム画面からステージ選択画面に入る際のズーム処理

};

