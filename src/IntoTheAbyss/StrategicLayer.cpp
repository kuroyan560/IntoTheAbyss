#include "StrategicLayer.h"
#include"DebugKeyManager.h"
#include"StaminaItemMgr.h"
#include"BulletCollision.h"
#include"../IntoTheAbyss/CharacterManager.h"
#include"Stamina.h"
#include"ScrollMgr.h"
#include"StageMgr.h"
#include"DebugParameter.h"

const float IStrategicLayer::SEARCH_RADIUS = 500.0f;
const float RestoreStamina::SUCCEED_GAIN_STAMINA_VALUE = 0.4f;
const float SwingThreeTimesCounterClockWise::SUCCEED_GAUGE_VALUE = 0.3f;

float IStrategicLayer::GetGaugeStatus()
{
	float rate = static_cast<float>(timer) / static_cast<float>(timeOver);
	if (1.0f <= rate)
	{
		rate = 1.0f;
	}
	return rate;
}

IStrategicLayer::SearchData IStrategicLayer::SearchItem(const SphereCollision &DATA)
{
	std::array<StaminaItem, 300>item = StaminaItemMgr::Instance()->GetItemArray();

	MapChipArray *mapData = StageMgr::Instance()->GetLocalMap();

	float mapX = mapData[0].size() * 50.0f;
	float position = CharacterManager::Instance()->Right()->pos.x / mapX;
	float rate = 1.0f;
	//ポジションゲージが0.5以下の場合、敵陣に近づいている事のなので座標を動かす準備をする
	if (position <= 0.5f)
	{
		float adjPosition = position - 0.3f;
		rate = adjPosition / 0.2f;
	}
	Vec2<float> adjPos = Vec2<float>((SEARCH_RADIUS / 2.0f) * (1.0 - rate), 0.0f);
	searchCollision.center = &adjPos;
	searchCollision.radius = SEARCH_RADIUS;



	std::vector<float>distance;
	std::vector<int>itemId;
	//探索範囲内にアイテムがあるのか調べる
	for (int i = 0; i < item.size(); ++i)
	{
		//アイテムを一つ以上見つけたら探索準備をする
		//そして距離を測る
		bool canGetFlag = item[i].GetIsActive() && !item[i].GetIsAcquired();
		if (canGetFlag && BulletCollision::Instance()->CheckSphereAndSphere(*item[i].GetCollisionData(), searchCollision))
		{
			distance.push_back(DATA.center->Distance(*item[i].GetCollisionData()->center));
			itemId.push_back(i);
		}
	}

	//探索範囲内から一番近いアイテムを見る
	SearchData result;
	result.distance = 10000.0f;
	result.itemIndex = -1;
	for (int i = 0; i < distance.size(); ++i)
	{
		if (distance[i] < result.distance)
		{
			result.distance = distance[i];
			result.itemIndex = itemId[i];
		}
	}

	return result;
}

RestoreStamina::RestoreStamina()
{
	initRouteFlag = true;

	seachItemFlag = true;
	searchItemIndex = -1;
	prevItemIndex = -1;
	prevStartHandle = startPoint.handle;

	searchArea.center = &CharacterManager::Instance()->Right()->pos;
	searchArea.radius = SEARCH_RADIUS;

	initFlag = true;
	getFlag = false;

	startFlag = true;
}

void RestoreStamina::Init()
{
	staminaGauge = CharacterAIData::Instance()->bossData.stamineGauge;
	timer = 0;
	timeOver = 60 * 10;

	CharacterAIOrder::Instance()->Init();
}

void RestoreStamina::Update()
{
	std::array<StaminaItem, 300>item = StaminaItemMgr::Instance()->GetItemArray();


	//アイテム探索を開始
	if (seachItemFlag)
	{
		SearchData result = SearchItem(searchArea);

		if (result.itemIndex != -1 && result.itemIndex != prevItemIndex)
		{
			searchItemIndex = result.itemIndex;
			seachItemFlag = false;
			initFlag = false;
			//ゴール地点から最も近いウェイポイントをゴール地点とする--------------------------
			searchGoalPoint.Init(*item[searchItemIndex].GetCollisionData()->center);
			endPoint = searchGoalPoint.Update();
			//ゴール地点から最も近いウェイポイントをゴール地点とする--------------------------

			prevItemIndex = result.itemIndex;
			initRouteFlag = false;
			getFlag = true;
		}
		else
		{
			seachItemFlag = false;
			getFlag = false;
		}
	}
	else
	{
		//探索範囲内に指定のアイテムがあるかどうか見る
		//無くなったら再検索をかける
		if (searchItemIndex != -1 && !BulletCollision::Instance()->CheckSphereAndSphere(*item[searchItemIndex].GetCollisionData(), searchArea))
		{
			seachItemFlag = true;
		}
	}


	//アイテムが無い場合は基本陣地に向かう
	if (!getFlag)
	{
		moveToOnwGround.route = route;
		moveToOnwGround.Update();
		startPoint = moveToOnwGround.startPoint;
		endPoint = moveToOnwGround.endPoint;
	}
	//アイテムが見つかったらその方向に向かう
	else
	{
		//ボスから最も近いウェイポイントをスタート地点とする------------------------
		if (startPoint.handle != endPoint.handle)
		{
			searchStartPoint.Init(*searchArea.center);
			startPoint = searchStartPoint.Update();
		}
		//ボスから最も近いウェイポイントをスタート地点とする------------------------

		//探索を開始し、向かう--------------------------
		if (route.size() != 0 && (!initRouteFlag || prevStartHandle != startPoint.handle))
		{
			followPath.Init(route);
			initRouteFlag = true;
		}
		prevStartHandle = startPoint.handle;

		//ウェイポイントに沿って移動する
		followPath.Update();
		//探索を開始し、向かう--------------------------


		//時間内に獲得したらまた近くのアイテムを探す。一定時間内にたどり着かなければ失敗
		if (!seachItemFlag && item[searchItemIndex].GetIsAcquired())
		{
			seachItemFlag = true;
			getFlag = false;
		}
	}

	//戦略実行からの経過時間
	++timer;
	timer += CharacterAIData::Instance()->addTimer;
}

void RestoreStamina::Draw()
{
	MapChipArray mapData = *StageMgr::Instance()->GetLocalMap();
	float mapX = mapData[0].size() * 50.0f;

	float position = CharacterManager::Instance()->Right()->pos.x / mapX;
	float rate = 1.0f;
	//ポジションゲージが0.5以下の場合、敵陣に近づいている事のなので座標を動かす準備をする
	if (position <= 0.5f)
	{
		float adjPosition = position - 0.3f;
		rate = adjPosition / 0.2f;
	}
	Vec2<float> adjPos = Vec2<float>((SEARCH_RADIUS / 2.0f) * (1.0 - rate), 0.0f);
	DrawFunc::DrawCircle2D(ScrollMgr::Instance()->Affect(CharacterManager::Instance()->Right()->pos + adjPos), ScrollMgr::Instance()->zoom * searchCollision.radius, Color(255, 255, 255, 255));
}

AiResult RestoreStamina::CurrentProgress()
{
	//一定時間内に一定量回復したら成功、出来なければ失敗
	float sub = CharacterAIData::Instance()->bossData.stamineGauge - staminaGauge;
	if (SUCCEED_GAIN_STAMINA_VALUE <= sub && timer < timeOver)
	{
		return AiResult::OPERATE_SUCCESS;
	}
	else if (timeOver <= timer)
	{
		return AiResult::OPERATE_FAIL;
	}
	else
	{
		return AiResult::OPERATE_INPROCESS;
	}
}

float RestoreStamina::EvaluationFunction()
{
	CharacterAIData *data = CharacterAIData::Instance();
	//評価値
	int evaluationValue = 0;

	//優勢ゲージ n6~7
	if (0.6f <= data->bossData.gaugeValue && data->bossData.gaugeValue <= 0.7f)
	{
		evaluationValue += 2;
	}

	const float LESS_OF_MY_OWN_STAMINA = 0.5f;
	//スタミナが少なくなった
	if (data->bossData.stamineGauge < LESS_OF_MY_OWN_STAMINA)
	{
		evaluationValue += 2;
	}


	//ウェイポイント探索
	SearchWayPoint search;

	//ボスから近場のウェイポイント探索
	search.Init(CharacterManager::Instance()->Right()->pos);
	WayPointData bossNearWayPoint = search.Update();

	const float nearDistance = 200.0f;
	//自分と壁との距離が近い
	if (!bossNearWayPoint.isWall)
	{
		bool nearTopFlag = bossNearWayPoint.wallDistanceTop < nearDistance;
		bool nearBottomFlag = bossNearWayPoint.wallDistanceBottom < nearDistance;
		bool nearLeftFlag = bossNearWayPoint.wallDistanceLeft < nearDistance;
		bool nearRightFlag = bossNearWayPoint.wallDistanceRight < nearDistance;

		if (nearTopFlag || nearBottomFlag || nearLeftFlag || nearRightFlag)
		{
			evaluationValue += 2;
		}
	}


	search.Init(CharacterManager::Instance()->Left()->pos);
	WayPointData playerNearWayPoint = search.Update();

	//敵と壁との距離が近くない
	if (!playerNearWayPoint.isWall)
	{
		bool farTopFlag = nearDistance < playerNearWayPoint.wallDistanceTop;
		bool farBottomFlag = nearDistance < playerNearWayPoint.wallDistanceBottom;
		bool farLeftFlag = nearDistance < playerNearWayPoint.wallDistanceLeft;
		bool farRightFlag = nearDistance < playerNearWayPoint.wallDistanceRight;

		if (farTopFlag || farBottomFlag || farLeftFlag || farRightFlag)
		{
			evaluationValue += 1;
		}
	}

	//自分の方がアイテムが近い
	SearchData bossResult = SearchItem(searchArea);
	if (bossResult.distance < SEARCH_RADIUS)
	{
		evaluationValue += 2;
	}

	//敵の振り回し入力があった
	if (data->swingFlag)
	{
		evaluationValue += 2;
	}

	//自分と自陣との距離が近い
	const float NEAR_LINE_RATE = 0.7f;
	bool nearFlag = NEAR_LINE_RATE < CharacterAIData::Instance()->position;
	if (nearFlag)
	{
		evaluationValue += 2;
	}

	//敵と敵陣との距離が近い
	const float FAR_LINE_RATE = 0.3f;
	bool farFlag = 1.0f - CharacterAIData::Instance()->position < FAR_LINE_RATE;
	if (farFlag)
	{
		evaluationValue += 2;
	}

	//自陣へのウェイポイント
	return static_cast<float>(evaluationValue) / static_cast<float>(data->EVALUATION_MAX_VALUE);
}


SwingClockWise::SwingClockWise()
{
}

void SwingClockWise::Init()
{
	timer = 0;
	timeOver = GAUGE_TIMER;
	startFlag = false;
	goToTheFieldFlag = true;

	CharacterAIOrder::Instance()->Init();
	finishFlag = false;
	operateSwing.Init(SWING_MAX_COOL_TIME);
}

void SwingClockWise::Update()
{
	//自分が自陣に近づく
	if (goToTheFieldFlag)
	{
		moveToOnwGround.route = route;
		moveToOnwGround.Update();
		startPoint = moveToOnwGround.startPoint;
		endPoint = moveToOnwGround.endPoint;
		startFlag = true;
	}
	//自分が敵陣に近づかない
	else
	{

	}



	bool useSwingFlag = timeOver <= timer;

	operateSwing.Update();
	if (useSwingFlag)
	{
		if (operateSwing.SwingClockWise() == AiResult::OPERATE_SUCCESS)
		{
			//連続で振り回すのを防止する為ダッシュカウントをリセットする
			CharacterAIData::Instance()->dashCount = 0;
			CharacterAIData::Instance()->dashTimer = 0;
		}
		//振り回し終わったら次の戦略に移動する
		else if (!CharacterManager::Instance()->Right()->GetNowSwing())
		{
			finishFlag = true;
			CharacterAIOrder::Instance()->prevSwingFlag = false;
		}
	}

	++timer;
	timer += CharacterAIData::Instance()->addTimer;
}

AiResult SwingClockWise::CurrentProgress()
{
	if (finishFlag)
	{
		return AiResult::OPERATE_SUCCESS;
	}
	else
	{
		return AiResult::OPERATE_INPROCESS;
	}
}

float SwingClockWise::EvaluationFunction()
{
	CharacterAIData *data = CharacterAIData::Instance();
	//評価値
	int evaluationValue = 0;

	//優勢ゲージ n6~7
	if (0.6f <= data->bossData.gaugeValue && data->bossData.gaugeValue <= 0.7f)
	{
		evaluationValue += 2;
	}

	const float MANY_OF_MY_OWN_STAMINA = 0.5f;
	//スタミナが多い
	if (MANY_OF_MY_OWN_STAMINA <= data->bossData.stamineGauge)
	{
		evaluationValue += 1;
	}

	//ウェイポイント探索
	SearchWayPoint search;

	//ボスから近場のウェイポイント探索
	search.Init(CharacterManager::Instance()->Right()->pos);
	WayPointData bossNearWayPoint = search.Update();

	const float nearDistance = 200.0f;
	//自分と壁との距離が近い
	if (!bossNearWayPoint.isWall)
	{
		bool farTopFlag = nearDistance < bossNearWayPoint.wallDistanceTop;
		bool farBottomFlag = nearDistance < bossNearWayPoint.wallDistanceBottom;
		bool farLeftFlag = nearDistance < bossNearWayPoint.wallDistanceLeft;
		bool farRightFlag = nearDistance < bossNearWayPoint.wallDistanceRight;

		if (farTopFlag || farBottomFlag || farLeftFlag || farRightFlag)
		{
			evaluationValue += 2;
		}
	}

	search.Init(CharacterManager::Instance()->Right()->pos);
	WayPointData playerNearWayPoint = search.Update();

	//敵と壁との距離が近くない
	if (!playerNearWayPoint.isWall)
	{
		bool farTopFlag = nearDistance < playerNearWayPoint.wallDistanceTop;
		bool farBottomFlag = nearDistance < playerNearWayPoint.wallDistanceBottom;
		bool farLeftFlag = nearDistance < playerNearWayPoint.wallDistanceLeft;
		bool farRightFlag = nearDistance < playerNearWayPoint.wallDistanceRight;

		if (farTopFlag || farBottomFlag || farLeftFlag || farRightFlag)
		{
			evaluationValue += 1;
		}
	}

	SphereCollision searchArea;
	searchArea.center = &CharacterManager::Instance()->Right()->pos;
	searchArea.radius = SEARCH_RADIUS;

	//敵より自分の方がアイテムが近い
	//自分側
	SearchData bossResult = SearchItem(searchArea);
	//敵側
	SphereCollision hitBox;
	hitBox.center = &CharacterManager::Instance()->Left()->pos;
	hitBox.radius = SEARCH_RADIUS;
	SearchData playerResult = SearchItem(hitBox);
	const float distance = 200.0f;
	//敵が一定以上アイテムから離れていたら、敵を移動させることを優勢できるように値を入れる
	if (distance < playerResult.distance)
	{
		evaluationValue += 2;
	}

	//自分と自陣との距離が近い
	const float NEAR_LINE_RATE = 0.7f;
	bool nearFlag = NEAR_LINE_RATE < CharacterAIData::Instance()->position;
	if (nearFlag)
	{
		evaluationValue += 3;
	}

	//敵と敵陣との距離が近い
	const float FAR_LINE_RATE = 0.3f;
	bool farFlag = CharacterAIData::Instance()->position < FAR_LINE_RATE;
	if (farFlag)
	{
		evaluationValue += 3;
	}

	//自陣へのウェイポイントが少ない


	return static_cast<float>(evaluationValue) / static_cast<float>(data->EVALUATION_MAX_VALUE);
}


SwingThreeTimesCounterClockWise::SwingThreeTimesCounterClockWise()
{
}

void SwingThreeTimesCounterClockWise::Init()
{
	nowGauge = CharacterAIData::Instance()->bossData.gaugeValue;
	timer = 0;
	timeOver = GAUGE_TIMER;

	crashEnemyFlag = false;
	dontCrashFlag = false;

	CharacterAIOrder::Instance()->Init();
	operateSwing.Init(SWING_MAX_COOL_TIME);

	finishFlag = false;
	swingingFlag = false;

	countSwingNum = 0;
}

void SwingThreeTimesCounterClockWise::Update()
{
	moveToOnwGround.route = route;
	moveToOnwGround.Update();
	startPoint = moveToOnwGround.startPoint;
	endPoint = moveToOnwGround.endPoint;
	startFlag = true;


	bool useSwingFlag = timeOver <= timer;

	operateSwing.Update();
	if (useSwingFlag)
	{
		if (3 <= countSwingNum && !CharacterManager::Instance()->Right()->GetNowSwing())
		{
			finishFlag = true;
		}
		else if (countSwingNum % 2 == 0 && !CharacterManager::Instance()->Right()->GetNowSwing())
		{
			operateSwing.SwingQuickClockWise();
			CharacterAIData::Instance()->dashCount = 0;
			CharacterAIData::Instance()->dashTimer = 0;
			++countSwingNum;
		}
		else if (countSwingNum % 2 != 0 && !CharacterManager::Instance()->Right()->GetNowSwing())
		{
			operateSwing.SwingQuickCounterClockWise();
			CharacterAIData::Instance()->dashCount = 0;
			CharacterAIData::Instance()->dashTimer = 0;
			++countSwingNum;
		}
		swingingFlag = true;
		CharacterAIOrder::Instance()->stopFlag = true;
	}

	//戦略実行中
	++timer;
	timer += CharacterAIData::Instance()->addTimer;
}

AiResult SwingThreeTimesCounterClockWise::CurrentProgress()
{
	if (finishFlag)
	{
		return AiResult::OPERATE_SUCCESS;
	}
	else
	{
		return AiResult::OPERATE_INPROCESS;
	}
}

float SwingThreeTimesCounterClockWise::EvaluationFunction()
{
	CharacterAIData *data = CharacterAIData::Instance();
	//評価値
	int evaluationValue = 0;

	//優勢ゲージ n6~7
	float sub = data->bossData.gaugeValue - data->bossData.gaugeValue;
	const float BIG_DIFFERENCE_IN_GAUGE = 0.3f;
	if (BIG_DIFFERENCE_IN_GAUGE <= fabs(sub))
	{
		evaluationValue += 3;
	}

	const float MANY_OF_MY_OWN_STAMINA = 0.5f;
	//スタミナが多い
	if (MANY_OF_MY_OWN_STAMINA <= data->bossData.stamineGauge)
	{
		evaluationValue += 2;
	}

	//ウェイポイント探索
	SearchWayPoint search;

	//ボスから近場のウェイポイント探索
	search.Init(CharacterManager::Instance()->Right()->pos);
	WayPointData playerNearWayPoint = search.Update();

	const float nearDistance = 200.0f;
	//敵と壁との距離が近い
	if (!playerNearWayPoint.isWall)
	{
		bool nearTopFlag = playerNearWayPoint.wallDistanceTop < nearDistance;
		bool nearBottomFlag = playerNearWayPoint.wallDistanceBottom < nearDistance;
		bool nearLeftFlag = playerNearWayPoint.wallDistanceLeft < nearDistance;
		bool nearRightFlag = playerNearWayPoint.wallDistanceRight < nearDistance;

		if (nearTopFlag || nearBottomFlag || nearLeftFlag || nearRightFlag)
		{
			evaluationValue += 2;
		}
	}


	SphereCollision searchArea;
	searchArea.center = &CharacterManager::Instance()->Right()->pos;
	searchArea.radius = SEARCH_RADIUS;

	//敵より自分の方がアイテムが近い
	//自分側
	SearchData bossResult = SearchItem(searchArea);
	//敵側
	SphereCollision hitBox;
	hitBox.center = &CharacterManager::Instance()->Left()->pos;
	hitBox.radius = SEARCH_RADIUS;
	SearchData playerResult = SearchItem(hitBox);
	const float distance = 200.0f;
	//敵が一定以上アイテムから離れていたら、敵を移動させることを優勢できるように値を入れる
	if (distance < playerResult.distance)
	{
		evaluationValue += 2;
	}


	{
		//自分と自陣との距離が近い&&優勢ゲージが優勢
		const float NEAR_LINE_RATE = 0.7f;
		bool nearFlag = NEAR_LINE_RATE < CharacterAIData::Instance()->position;
		const float ADVANTAGE_VALUE = 0.7f;
		bool advantageFlag = ADVANTAGE_VALUE <= data->bossData.gaugeValue;
		if (nearFlag)
		{
			evaluationValue += 3;
		}
	}

	{
		//自分と敵陣との距離が近い&&優勢ゲージが劣勢
		const float NEAR_LINE_RATE = 0.3f;
		bool nearFlag = CharacterAIData::Instance()->position < NEAR_LINE_RATE;
		const float ADVANTAGE_VALUE = 0.3f;
		bool advantageFlag = data->bossData.gaugeValue <= ADVANTAGE_VALUE;
		if (nearFlag)
		{
			evaluationValue += 3;
		}
	}

	//敵の振り回し入力があった
	if (data->swingFlag)
	{
		evaluationValue += 2;
	}

	return static_cast<float>(evaluationValue) / static_cast<float>(data->EVALUATION_MAX_VALUE);
}

Dash::Dash()
{
}

void Dash::Init()
{
	finishFlag = false;
	timer = 0;
	timeOver = GAUGE_TIMER;
	initDashFlag = false;
	CharacterAIOrder::Instance()->Init();
}

void Dash::Update()
{
	bool activeFlag = timeOver <= timer;
	if (activeFlag && !initDashFlag)
	{
		CharacterAIData::Instance()->dashFlag = true;
		initDashFlag = true;
	}

	moveToOnwGround.route = route;
	moveToOnwGround.Update();
	startPoint = moveToOnwGround.startPoint;
	endPoint = moveToOnwGround.endPoint;
	startFlag = true;

	if (CharacterAIData::Instance()->finishDashFlag)
	{
		finishFlag = true;
	}

	++timer;
	timer += CharacterAIData::Instance()->addTimer;
}

AiResult Dash::CurrentProgress()
{
	if (finishFlag)
	{
		return AiResult::OPERATE_SUCCESS;
	}
	else
	{
		return AiResult::OPERATE_INPROCESS;
	}
}

float Dash::EvaluationFunction()
{
	return KuroFunc::GetRand(1.0f);
}


SwingClockWiseThreeTimes::SwingClockWiseThreeTimes()
{

}

void SwingClockWiseThreeTimes::Init()
{
	timer = 0;
	timeOver = GAUGE_TIMER;

	crashEnemyFlag = false;
	dontCrashFlag = false;


	CharacterAIOrder::Instance()->Init();
	operateSwing.Init(SWING_MAX_COOL_TIME);

	finishFlag = false;
	swingingFlag = false;

	countSwingNum = 0;
}

void SwingClockWiseThreeTimes::Update()
{
	moveToOnwGround.route = route;
	moveToOnwGround.Update();
	startPoint = moveToOnwGround.startPoint;
	endPoint = moveToOnwGround.endPoint;
	startFlag = true;


	bool useSwingFlag = timeOver <= timer;

	operateSwing.Update();
	if (useSwingFlag)
	{
		if (3 <= countSwingNum && !CharacterManager::Instance()->Right()->GetNowSwing())
		{
			finishFlag = true;
		}
		else if (countSwingNum % 2 == 0 && !CharacterManager::Instance()->Right()->GetNowSwing())
		{
			operateSwing.SwingQuickCounterClockWise();
			CharacterAIData::Instance()->dashCount = 0;
			CharacterAIData::Instance()->dashTimer = 0;
			++countSwingNum;
		}
		else if (countSwingNum % 2 != 0 && !CharacterManager::Instance()->Right()->GetNowSwing())
		{
			operateSwing.SwingQuickClockWise();
			CharacterAIData::Instance()->dashCount = 0;
			CharacterAIData::Instance()->dashTimer = 0;
			++countSwingNum;
		}
		swingingFlag = true;
		CharacterAIOrder::Instance()->stopFlag = true;
	}

	//戦略実行中
	++timer;
	timer += CharacterAIData::Instance()->addTimer;
}

AiResult SwingClockWiseThreeTimes::CurrentProgress()
{
	if (finishFlag)
	{
		return AiResult::OPERATE_SUCCESS;
	}
	else
	{
		return AiResult::OPERATE_INPROCESS;
	}
}

float SwingClockWiseThreeTimes::EvaluationFunction()
{
	return KuroFunc::GetRand(1.0f);
}


SwingCounterClockWise::SwingCounterClockWise()
{

}

void SwingCounterClockWise::Init()
{
	timer = 0;
	timeOver = GAUGE_TIMER;

	crashEnemyFlag = false;
	dontCrashFlag = false;


	CharacterAIOrder::Instance()->Init();
	operateSwing.Init(SWING_MAX_COOL_TIME);

	finishFlag = false;
	swingingFlag = false;

	countSwingNum = 0;
}

void SwingCounterClockWise::Update()
{
	moveToOnwGround.route = route;
	moveToOnwGround.Update();
	startPoint = moveToOnwGround.startPoint;
	endPoint = moveToOnwGround.endPoint;
	startFlag = true;


	bool useSwingFlag = timeOver <= timer;
	operateSwing.Update();
	if (useSwingFlag)
	{
		if (operateSwing.SwingCounterClockWise() == AiResult::OPERATE_SUCCESS)
		{
			//連続で振り回すのを防止する為ダッシュカウントをリセットする
			CharacterAIData::Instance()->dashCount = 0;
			CharacterAIData::Instance()->dashTimer = 0;
		}
		//振り回し終わったら次の戦略に移動する
		else if (!CharacterManager::Instance()->Right()->GetNowSwing())
		{
			finishFlag = true;
			CharacterAIOrder::Instance()->prevSwingFlag = false;
		}
	}
	//戦略実行中
	++timer;
	timer += CharacterAIData::Instance()->addTimer;
}

AiResult SwingCounterClockWise::CurrentProgress()
{
	if (finishFlag)
	{
		return AiResult::OPERATE_SUCCESS;
	}
	else
	{
		return AiResult::OPERATE_INPROCESS;
	}
}

float SwingCounterClockWise::EvaluationFunction()
{
	return KuroFunc::GetRand(1.0f);
}
