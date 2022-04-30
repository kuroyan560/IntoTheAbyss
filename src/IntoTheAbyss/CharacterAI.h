#pragma once
#include"../KuroEngine.h"
#include"../IntoTheAbyss/TacticalLayer.h"
#include"../IntoTheAbyss/OperatingLayer.h"
#include<memory>

/// <summary>
/// キャラクターAIの意思決定
/// 階層型ゴール指向プランニングに基づいて設計する
/// </summary>
class CharacterAI
{
public:
	void Init();
	void Update();
	void Draw();
	std::unique_ptr<FollowPath> moveToGoal;
	std::shared_ptr<MovingBetweenTwoPoints> betweenPoints;
	std::shared_ptr<OperateMove>move;

	std::shared_ptr<Vec2<float>>pos;
	std::vector<WayPointData> shortestData;

	bool initFlag = false;
};

