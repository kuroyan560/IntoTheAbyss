
#include "Game.h"
#include"MapChipCollider.h"
#include"ShakeMgr.h"
#include"ScrollMgr.h"
//#include"BulletParticleMgr.h"
#include "StunEffect.h"
#include "SlowMgr.h"
#include"Bullet.h"
#include"Collider.h"
#include"SightCollisionStorage.h"
#include"SelectStage.h"
#include"AfterImage.h"
#include"CrashEffectMgr.h"
#include"Stamina.h"

#include"KuroFunc.h"
#include"KuroEngine.h"
#include"TexHandleMgr.h"
#include"DrawFunc.h"
#include"ParticleMgr.h"

#include"SuperiorityGauge.h"
//#include"BackGround.h"
#include"Camera.h"
#include"GameTimer.h"
#include"ScoreManager.h"
#include"FaceIcon.h"
#include"WinCounter.h"

#include"BulletCollision.h"

#include"CrashMgr.h"

#include"ResultTransfer.h"
#include "Player.h"
#include "Boss.h"

#include "CharacterInterFace.h"

#include<map>

#include"DebugParameter.h"

#include"DebugKeyManager.h"

#include"CharacterManager.h"
#include "StaminaItemMgr.h"

#include "RoundFinishEffect.h"

#include "RoundFinishParticle.h"
#include "DistanceCounter.h"

#include"ScoreKeep.h"
#include "RoundCountMgr.h"

#include "BackGroundParticle.h"
#include "GameSceneCamerMove.h"

std::vector<std::unique_ptr<MassChipData>> Game::AddData(MapChipArray MAPCHIP_DATA, const int& CHIP_NUM)
{
	MassChip checkData;
	std::vector<std::unique_ptr<MassChipData>> data;

	for (int y = 0; y < MAPCHIP_DATA.size(); ++y)
	{
		for (int x = 0; x < MAPCHIP_DATA[y].size(); ++x)
		{
			if (MAPCHIP_DATA[y][x].chipType == CHIP_NUM)
			{
				bool sucseedFlag = checkData.Check(Vec2<int>(x, y), CHIP_NUM);
				if (!sucseedFlag)
				{
					continue;
				}

				//伸びた情報をオーラに渡す
				data.push_back(std::make_unique<MassChipData>(checkData.GetLeftUpPos(), checkData.GetRightDownPos(), checkData.sideOrUpDownFlag));
			}
		}
	}

	return data;
}

void Game::DrawMapChip(MapChipArray& mapChipData, const int& stageNum, const int& roomNum)
{
	std::map<int, std::vector<ChipData>>datas;

	// 描画するチップのサイズ
	const float DRAW_MAP_CHIP_SIZE = MAP_CHIP_SIZE * ScrollMgr::Instance()->zoom;
	SizeData wallChipMemorySize = StageMgr::Instance()->GetMapChipSizeData(MAPCHIP_TYPE_STATIC_BLOCK);


	// マップチップの縦の要素数を取得。
	const int HEIGHT = mapChipData.size();
	for (int height = 0; height < HEIGHT; ++height) {

		// マップチップの横の要素数を取得。
		const int WIDTH = mapChipData[height].size();
		for (int width = 0; width < WIDTH; ++width) {

			if (mapChipData[height][width].drawData.shocked)mapChipData[height][width].drawData.shocked -= 0.02f;
			if (mapChipData[height][width].drawData.expEaseRate < 1.0f)mapChipData[height][width].drawData.expEaseRate += 0.005f;

			// ブロック以外だったら処理を飛ばす。
			bool blockFlag = (mapChipData[height][width].chipType >= wallChipMemorySize.min && mapChipData[height][width].chipType <= wallChipMemorySize.max);
			if (blockFlag)
			{
				// スクロール量から描画する位置を求める。
				const Vec2<float> drawPos = ScrollMgr::Instance()->Affect({ width * MAP_CHIP_SIZE,height * MAP_CHIP_SIZE }) + GameSceneCameraMove::Instance()->move;

				// 画面外だったら描画しない。
				if (drawPos.x < -DRAW_MAP_CHIP_SIZE || drawPos.x > WinApp::Instance()->GetWinSize().x + DRAW_MAP_CHIP_SIZE) continue;
				if (drawPos.y < -DRAW_MAP_CHIP_SIZE || drawPos.y > WinApp::Instance()->GetWinSize().y + DRAW_MAP_CHIP_SIZE) continue;


				vector<std::shared_ptr<MapChipAnimationData>>tmpAnimation = StageMgr::Instance()->animationData;
				int handle = -1;
				if (height < 0 || mapChipData.size() <= height) continue;
				if (width < 0 || mapChipData[height].size() <= width) continue;
				//アニメーションフラグが有効ならアニメーション用の情報を行う
				if (mapChipData[height][width].drawData.animationFlag)
				{
					int arrayHandle = mapChipData[height][width].drawData.handle;
					++mapChipData[height][width].drawData.interval;
					//アニメーションの間隔
					if (mapChipData[height][width].drawData.interval % tmpAnimation[arrayHandle]->maxInterval == 0)
					{
						++mapChipData[height][width].drawData.animationNum;
						mapChipData[height][width].drawData.interval = 0;
					}
					//アニメーション画像の総数に達したら最初に戻る
					if (tmpAnimation[arrayHandle]->handle.size() <= mapChipData[height][width].drawData.animationNum)
					{
						mapChipData[height][width].drawData.animationNum = 0;
					}
					//分割したアニメーションの画像から渡す
					handle = tmpAnimation[arrayHandle]->handle[mapChipData[height][width].drawData.animationNum];
				}
				else
				{
					handle = mapChipData[height][width].drawData.handle;
				}

				//mapChipDrawData[height][width].shocked = KuroMath::Lerp(mapChipDrawData[height][width].shocked, 0.0f, 0.8f);

				Vec2<float> pos = drawPos;
				pos += mapChipData[height][width].drawData.offset;
				if (0 <= handle)
				{
					ChipData chipData;
					chipData.pos = pos;
					chipData.radian = mapChipData[height][width].drawData.radian;
					chipData.shocked = mapChipData[height][width].drawData.shocked;
					chipData.expEaseRate = mapChipData[height][width].drawData.expEaseRate;
					datas[handle].emplace_back(chipData);
					//DrawFunc::DrawRotaGraph2D({ pos.x, pos.y }, 1.6f * ScrollMgr::Instance()->zoom, mapChipDrawData[height][width].radian, TexHandleMgr::GetTexBuffer(handle));
				}
			}
		}
	}

	while (drawMap.size() < datas.size())
	{
		drawMap.emplace_back();
	}

	int i = 0;
	for (auto itr = datas.begin(); itr != datas.end(); ++itr)
	{
		for (int chipIdx = 0; chipIdx < itr->second.size(); ++chipIdx)
		{
			drawMap[i].AddChip(itr->second[chipIdx]);
		}
		drawMap[i].Draw(TexHandleMgr::GetTexBuffer(itr->first));
		i++;
	}
}

const int& Game::GetChipNum(const vector<vector<int>>& MAPCHIP_DATA, const int& MAPCHIP_NUM, int* COUNT_CHIP_NUM, Vec2<float>* POS)
{
	int chipNum = 0;
	for (int y = 0; y < MAPCHIP_DATA.size(); ++y)
	{
		for (int x = 0; x < MAPCHIP_DATA[y].size(); ++x)
		{
			if (MAPCHIP_DATA[y][x] == MAPCHIP_NUM)
			{
				*COUNT_CHIP_NUM += 1;
				*POS = { x * 50.0f,y * 50.0f };
			}
		}
	}
	return chipNum;
}

#include"PlayerHand.h"
void Game::InitGame(const int& STAGE_NUM, const int& ROOM_NUM)
{
	GameSceneCameraMove::Instance()->move = {};

	isTransitionResult = false;
	trasitionTimer = 0;

	DebugParameter::Instance()->totalCombo = 0;
	DebugParameter::Instance()->timer = 0;

	CrashMgr::Instance()->Init();

	int stageNum = STAGE_NUM;
	int roomNum = ROOM_NUM;

	SuperiorityGauge::Instance()->Init();

	FaceIcon::Instance()->Init(CharacterManager::Instance()->Left()->GetCharacterName(), CharacterManager::Instance()->Right()->GetCharacterName());


	StageMgr::Instance()->SetLocalMapChipData(stageNum, roomNum);
	mapData = StageMgr::Instance()->GetLocalMap();

	MapChipArray tmp = *mapData;

	// シェイク量を設定。
	ShakeMgr::Instance()->Init();
	ParticleMgr::Instance()->Init();

	initDeadFlag = false;
	//giveDoorNumber = 0;
	debugStageData[0] = stageNum;
	debugStageData[1] = roomNum;

	drawCharaFlag = false;

	//ScrollMgr::Instance()->honraiScrollAmount = { -1060.0f,-490.0f };
	alphaValue = 0;

	restartTimer = 0.0f;

	DistanceCounter::Instance()->Init();


	{
		Vec2<float>playerLeftUpPos;
		Vec2<float>playerRightDownPos;
		Vec2<float>enemyLeftUpPos;
		Vec2<float>enemyRightDownPos;


		for (int y = 0; y < tmp.size(); ++y)
		{
			for (int x = 0; x < tmp[y].size(); ++x)
			{
				if (tmp[y][x].chipType == 33)
				{
					playerLeftUpPos = Vec2<float>(x * MAP_CHIP_SIZE, y * MAP_CHIP_SIZE);
				}
				if (tmp[y][x].chipType == 34)
				{
					playerRightDownPos = Vec2<float>(x * MAP_CHIP_SIZE, y * MAP_CHIP_SIZE);
				}
				if (tmp[y][x].chipType == 35)
				{
					enemyLeftUpPos = Vec2<float>(x * MAP_CHIP_SIZE, y * MAP_CHIP_SIZE);
				}
				if (tmp[y][x].chipType == 36)
				{
					enemyRightDownPos = Vec2<float>(x * MAP_CHIP_SIZE, y * MAP_CHIP_SIZE);
				}
			}
		}

		Vec2<float>chipPos(MAP_CHIP_HALF_SIZE, MAP_CHIP_HALF_SIZE);
		playerHomeBase.Init(playerLeftUpPos - chipPos, playerRightDownPos + chipPos, true);
		enemyHomeBase.Init(enemyLeftUpPos - chipPos, enemyRightDownPos + chipPos, false);

		{
			float size = (tmp[0].size() * MAP_CHIP_SIZE) - 400.0f;
			miniMap.Init(size);
		}


		lineCenterPos = {};
		prevLineCenterPos = {};

		isCatchMapChipBoss = false;
		isCatchMapChipPlayer = false;
	}

	StaminaItemMgr::Instance()->Init();

	Vec2<float> playerResponePos((tmp[0].size() * MAP_CHIP_SIZE) * 0.5f, (tmp.size() * MAP_CHIP_SIZE) * 0.5f);
	Vec2<float> enemyResponePos;

	//スクロールを上にずらす用
	//responePos.x -= 100;
	//responePos.y += 50;
	lineCenterPos = playerResponePos - cameraBasePos;

	Vec2<float>plPos(StageMgr::Instance()->GetPlayerResponePos(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum()));
	Vec2<float>enPos(StageMgr::Instance()->GetBossResponePos(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum()));

	CharacterManager::Instance()->CharactersInit(plPos, enPos, !practiceMode);

	//miniMap.CalucurateCurrentPos(lineCenterPos);
	// 二人の距離を求める。
	float charaLength = Vec2<float>(CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Right()->pos).Length();
	// 紐を伸ばす量を求める。
	float addLength = charaLength - (CharacterInterFace::LINE_LENGTH * 2.0f);
	// 紐を伸ばす。
	CharacterManager::Instance()->Right()->addLineLength = addLength;

	Camera::Instance()->Init();
	Vec2<float>distance = (CharacterManager::Instance()->Right()->pos - CharacterManager::Instance()->Left()->pos) / 2.0f;
	Vec2<float>cPos = CharacterManager::Instance()->Left()->pos + distance / 2.0f;

	ScrollMgr::Instance()->Init(CharacterManager::Instance()->Left()->pos + distance, Vec2<float>(tmp[0].size() * MAP_CHIP_SIZE, tmp.size() * MAP_CHIP_SIZE), cameraBasePos);
	initCentralPos = CharacterManager::Instance()->Left()->pos + distance;


	Camera::Instance()->Zoom(CharacterManager::Instance()->Left()->pos, CharacterManager::Instance()->Right()->pos);
	ScrollMgr::Instance()->zoom = Camera::Instance()->zoom;

	gameTimer = StageMgr::Instance()->GetMaxTime(stageNum, roomNum);

	ScoreManager::Instance()->Init();
	GameTimer::Instance()->Init(gameTimer);
	GameTimer::Instance()->Start();

	firstLoadFlag = false;
	lineExtendScale = lineExtendMaxScale;

	gameStartFlag = false;

	screenEdgeEffect.Init();

	readyToStartRoundFlag = !practiceMode;

	if (practiceMode)
	{
		//ゲームスタート
		gameStartFlag = true;
		roundTimer = 0;
		CharacterManager::Instance()->Left()->SetCanMove(true);
		CharacterManager::Instance()->Right()->SetCanMove(true);
		CharacterManager::Instance()->Left()->SetHitCheck(true);
		CharacterManager::Instance()->Right()->SetHitCheck(true);
		GameTimer::Instance()->SetInterruput(false);
		roundChangeEffect.initGameFlag = true;
		roundChangeEffect.drawFightFlag = true;
	}

	mapChipGeneratorChangeMap.reset();
	mapChipGeneratorChangeMap = std::make_shared<MapChipGenerator_ChangeMap>();


	bool nonFlag = StageMgr::Instance()->GetStageInfo(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum()).generatorType != NON_GENERATE;
	InitCountBlock();

	ScoreManager::Instance()->Init();
	roundFinishFlag = false;

	// 背景パーティクルを更新
	BackGroundParticleMgr::Instance()->Init();
	BackGroundParticleMgr::Instance()->StageStartGenerate(Vec2<float>(StageMgr::Instance()->GetMapIdxSize(stageNum, roomNum).x * MAP_CHIP_SIZE, StageMgr::Instance()->GetMapIdxSize(stageNum, roomNum).y * MAP_CHIP_SIZE));




	{
		std::string bossFilePass = "resource/ChainCombat/boss/0/arm/";
		int dL = TexHandleMgr::LoadGraph(bossFilePass + "default/L.png");
		int dR = TexHandleMgr::LoadGraph(bossFilePass + "default/R.png");
		int hL = TexHandleMgr::LoadGraph(bossFilePass + "hold/L.png");
		int hR = TexHandleMgr::LoadGraph(bossFilePass + "hold/R.png");
		bossHandMgr = std::make_unique<BossHandMgr>(dL, dR, hL, hR, true);
	}

	{
		PLAYABLE_CHARACTER_NAME charaName = StageMgr::Instance()->GetStageInfo(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum()).characterName;

		std::string bossFilePass = {};
		int dL = 0;
		int dR = 0;
		int hL = 0;
		int hR = 0;

		switch (charaName)
		{
		case PLAYABLE_LUNA:
			bossFilePass = "resource/ChainCombat/player/luna/arm/";
			dL = TexHandleMgr::LoadGraph(bossFilePass + "default/L.png");
			dR = TexHandleMgr::LoadGraph(bossFilePass + "default/R.png");
			hL = TexHandleMgr::LoadGraph(bossFilePass + "hold/L.png");
			hR = TexHandleMgr::LoadGraph(bossFilePass + "hold/R.png");
			break;
		case PLAYABLE_LACY:
			bossFilePass = "resource/ChainCombat/player/lacy/arm/";
			dL = TexHandleMgr::LoadGraph(bossFilePass + "default/L.png");
			dR = TexHandleMgr::LoadGraph(bossFilePass + "default/R.png");
			hL = TexHandleMgr::LoadGraph(bossFilePass + "hold/L.png");
			hR = TexHandleMgr::LoadGraph(bossFilePass + "hold/R.png");
			break;
		case PLAYABLE_BOSS_0:
			break;
		case PLAYABLE_CHARACTER_NUM:
			break;
		default:
			break;
		}
		playerHandMgr = std::make_unique<BossHandMgr>(dL, dR, hL, hR, false);
	}

	bossHandMgr->Init(false);
	playerHandMgr->Init(false);

	stageComment.Init(SelectStage::Instance()->GetStageNum());


}

void Game::GeneratorInit()
{
	mapChipGenerator.reset();
	auto localStageInfo = StageMgr::Instance()->GetStageInfo(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum());
	MAP_CHIP_GENERATOR localGeneratorType = localStageInfo.generatorType;
	if (localGeneratorType == NON_GENERATE)
	{
		mapChipGenerator = std::make_shared<MapChipGenerator_Non>();
	}
	else if (localGeneratorType == SPLINE_ORBIT)
	{
		mapChipGenerator = std::make_shared<MapChipGenerator_SplineOrbit>();
	}
	else if (localGeneratorType == RAND_PATTERN)
	{
		mapChipGenerator = std::make_shared<MapChipGenerator_RandPattern>(localStageInfo.generatorSpan);
	}
	else if (localGeneratorType == CLOSSING)
	{
		mapChipGenerator = std::make_shared<MapChipGenerator_Crossing>(localStageInfo.generatorSpan);
	}
	else if (RISE_UP_L_TO_R <= localGeneratorType && localGeneratorType <= RISE_UP_BOTTOM_TO_TOP)
	{
		int idxDir = localGeneratorType - CLOSSING - 1;
		mapChipGenerator = std::make_shared<MapChipGenerator_RiseUp>(localStageInfo.generatorSpan, (RISE_UP_GENERATOR_DIRECTION)idxDir);
	}

}

Game::Game()
{
	GameSceneCameraMove::Instance()->move = {};
	bgm = AudioApp::Instance()->LoadAudio("resource/ChainCombat/sound/bgm_1.wav");

	addLineLengthSubAmount = 1.0f;

	playerHomeBase.Init({ 0.0f,0.0f }, { 0.0f,0.0f }, true);
	enemyHomeBase.Init({ 0.0f,0.0f }, { 800.0f,1000.0f }, false);
	//enemyHomeBase->Init({ 0.0f,0.0f }, { 0.0f,0.0f });

	cameraBasePos = { 0.0f,-40.0f };

	roundChangeEffect.Init();
	StageMgr::Instance()->SetLocalMapChipData(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum());
	mapData = StageMgr::Instance()->GetLocalMap();
	MapChipArray tmp = *mapData;


	Vec2<float> responePos((tmp[0].size() * MAP_CHIP_SIZE) * 0.5f, (tmp.size() * MAP_CHIP_SIZE) * 0.5f);
	//スクロールを上にずらす用
	responePos.x -= 25;
	responePos.y += 50;
	responeScrollPos = responePos;


	//Vec2<float>distance = (CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Right()->pos) / 2.0f;
	//ScrollMgr::Instance()->Init(CharacterManager::Instance()->Left()->pos + distance, Vec2<float>(tmp[0].size() * MAP_CHIP_SIZE, tmp.size() * MAP_CHIP_SIZE), cameraBasePos);

	Camera::Instance()->Init();
	SuperiorityGauge::Instance()->Init();

	readyToStartRoundFlag = true;
	//背景に星
	//BackGround::Instance()->Init(GetStageSize());

	mapChipGeneratorChangeMap = std::make_shared<MapChipGenerator_ChangeMap>();

}

void Game::Init(const bool& PracticeMode)
{
	GameSceneCameraMove::Instance()->move = {};
	rStickNoInputTimer = 0;

	RoundFinishEffect::Instance()->Init();

	static const int READY_EXPLOSION_SE = AudioApp::Instance()->LoadAudio("resource/ChainCombat/sound/readyExplosion.wav", 0.5f);
	AudioApp::Instance()->StopWave(READY_EXPLOSION_SE);

	isTransitionResult = false;
	trasitionTimer = 0;

	practiceMode = PracticeMode;

	WinCounter::Instance()->Reset();

	turnResultScene = false;

	CharacterManager::Instance()->CharactersGenerate();

	SelectStage::Instance()->SelectRoomNum(0);
	InitGame(SelectStage::Instance()->GetStageNum(), 0);
	ScrollMgr::Instance()->Reset();
	roundChangeEffect.Init();
	CrashEffectMgr::Instance()->Init();

	StaminaItemMgr::Instance()->SetArea(playerHomeBase.hitBox.center->x - playerHomeBase.hitBox.size.x, enemyHomeBase.hitBox.center->x + enemyHomeBase.hitBox.size.x);

	drawCharaFlag = false;
	RoundFinishEffect::Instance()->Init();

	stageRap.Init(StageMgr::Instance()->GetMaxLap(SelectStage::Instance()->GetStageNum()));

	DistanceCounter::Instance()->Init();

	GeneratorInit();

	// 背景パーティクルを更新
	//BackGroundParticleMgr::Instance()->Init();
	//BackGroundParticleMgr::Instance()->StageStartGenerate(Vec2<float>(StageMgr::Instance()->GetLocalMap()[0].size() * MAP_CHIP_SIZE, StageMgr::Instance()->GetLocalMap()->size() * MAP_CHIP_SIZE));

	int roomNum = StageMgr::Instance()->GetMaxRoomNumber(SelectStage::Instance()->GetStageNum());
	RoundCountMgr::Instance()->Init(roomNum);
}

void Game::InitRestart(const bool& PracticeMode)
{
	GameSceneCameraMove::Instance()->move = {};
	rStickNoInputTimer = 0;

	RoundFinishEffect::Instance()->Init();

	static const int READY_EXPLOSION_SE = AudioApp::Instance()->LoadAudio("resource/ChainCombat/sound/readyExplosion.wav", 0.5f);
	AudioApp::Instance()->StopWave(READY_EXPLOSION_SE);

	isTransitionResult = false;
	trasitionTimer = 0;

	practiceMode = PracticeMode;

	WinCounter::Instance()->Reset();

	turnResultScene = false;

	CharacterManager::Instance()->CharactersGenerate();

	//SelectStage::Instance()->SelectRoomNum(0);
	InitGame(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum());
	ScrollMgr::Instance()->Reset();
	roundChangeEffect.Init();
	CrashEffectMgr::Instance()->Init();

	StaminaItemMgr::Instance()->SetArea(playerHomeBase.hitBox.center->x - playerHomeBase.hitBox.size.x, enemyHomeBase.hitBox.center->x + enemyHomeBase.hitBox.size.x);

	drawCharaFlag = false;
	RoundFinishEffect::Instance()->Init();

	stageRap.Init(StageMgr::Instance()->GetMaxLap(SelectStage::Instance()->GetStageNum()));

	DistanceCounter::Instance()->Init();

	GeneratorInit();

	// 背景パーティクルを更新
	//BackGroundParticleMgr::Instance()->Init();
	//BackGroundParticleMgr::Instance()->StageStartGenerate(Vec2<float>(StageMgr::Instance()->GetLocalMap()[0].size() * MAP_CHIP_SIZE, StageMgr::Instance()->GetLocalMap()->size() * MAP_CHIP_SIZE));

	int roomNum = StageMgr::Instance()->GetMaxRoomNumber(SelectStage::Instance()->GetStageNum());
	RoundCountMgr::Instance()->Init(roomNum);
}

void Game::Update(const bool& Loop)
{

	//ScrollMgr::Instance()->zoom = ViewPort::Instance()->zoomRate;
	MapChipArray tmpMapData = *mapData;

	// ステージの切り替え
	SwitchingStage();

	addLineLengthSubAmount = DebugParameter::Instance()->addLineLengthSubAmount;


	// 陣地の判定
	//DeterminationOfThePosition();

	//ラウンド終了演出開始
	RoundFinishEffect(Loop);

	//ラウンド開始時の演出開始
	RoundStartEffect(Loop, tmpMapData);

	miniMap.CalucurateCurrentPos(lineCenterPos);

	screenEdgeEffect.CheckPos(miniMap.nowValue);


	if (roundChangeEffect.readyFlag)
	{
		{
			Vec2<float> sub = CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Right()->pos;
			bossHandMgr->Hold(-sub.GetNormal(), CharacterAIOrder::Instance()->prevSwingFlag);
			bossHandMgr->Update(CharacterManager::Instance()->Right()->pos);
		}
		{
			Vec2<float> sub = CharacterManager::Instance()->Right()->pos - CharacterManager::Instance()->Left()->pos;
			playerHandMgr->Hold(-sub.GetNormal(), true);
			playerHandMgr->Update(CharacterManager::Instance()->Left()->pos);
		}

		RoundCountMgr::Instance()->Appear();
		countBlock.Appear();
	}
	// プレイヤーの更新処理
	//if (!roundFinishFlag)
	//{
	if (roundChangeEffect.initGameFlag)
	{
		DebugParameter::Instance()->timer++;
		mapChipGeneratorChangeMap->Update();
		mapChipGenerator->Update();
	}

	// 座標を保存。
	CharacterManager::Instance()->Left()->SavePrevFramePos();
	CharacterManager::Instance()->Right()->SavePrevFramePos();

	CharacterManager::Instance()->Left()->Update(tmpMapData, lineCenterPos, readyToStartRoundFlag, roundFinishFlag, SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum());

	// ボスの更新処理
	CharacterManager::Instance()->Right()->Update(tmpMapData, lineCenterPos, readyToStartRoundFlag, roundFinishFlag, SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum());
	//}

	CharacterAIData::Instance()->nowPos = CharacterManager::Instance()->Right()->pos;
	// プレイヤーとボスの引っ張り合いの処理
	Scramble();

	// プレイヤーとボスの当たり判定処理
	CharacterManager::Instance()->Left()->CheckHit(tmpMapData, lineCenterPos, roundFinishFlag);
	CharacterManager::Instance()->Right()->CheckHit(tmpMapData, lineCenterPos, roundFinishFlag);
	CharacterAIData::Instance()->prevPos = CharacterManager::Instance()->Right()->pos;

	if (RoundFinishEffect::Instance()->addScoreFlag)
	{
		ScoreManager::Instance()->Add(countBlock.GetBlockNum());
		RoundFinishEffect::Instance()->addScoreFlag = false;
	}

	miniMap.Update();
	screenEdgeEffect.Update();

	roundChangeEffect.Update();

	GameTimer::Instance()->Update();
	ScoreManager::Instance()->Update();

	// シェイク量の更新処理
	ShakeMgr::Instance()->Update();

	SuperiorityGauge::Instance()->Update();

	// スタン演出クラスの更新処理
	StunEffect::Instance()->Update();

	//スコアを常に代入
	ScoreKeep::Instance()->AddScore(stageRap.GetRapNum() - 1, countBlock.GetBlockNum());

	// 中心点を計算。
	CalCenterPos();

	//	ScrollManager::Instance()->CalucurateScroll(prevLineCenterPos - lineCenterPos, lineCenterPos);
	ScrollMgr::Instance()->CalucurateScroll(prevLineCenterPos - lineCenterPos, lineCenterPos);

	// スクロール量の更新処理
	//ScrollManager::Instance()->Update();
	Camera::Instance()->Update();
	Vec2<float> distance = CharacterManager::Instance()->Right()->pos - CharacterManager::Instance()->Left()->pos;
	Vec2<float>cPos = CharacterManager::Instance()->Left()->pos + distance / 2.0f;


	bool disappearFlag = CharacterManager::Instance()->Left()->CheckDisappear() && CharacterManager::Instance()->Right()->CheckDisappear();
	//スクロールを中心に戻す
	if (disappearFlag)
	{
		ScrollMgr::Instance()->Update(initCentralPos);
	}
	else
	{
		ScrollMgr::Instance()->Update(cPos);
	}

	//パーティクル更新
	ParticleMgr::Instance()->Update();

	//BackGround::Instance()->Update();
	FaceIcon::Instance()->Update();
	WinCounter::Instance()->Update();


	CrashMgr::Instance()->Update();

	// 残像を更新。
	AfterImageMgr::Instance()->Update();

	// クラッシュ時の演出の更新処理。
	CrashEffectMgr::Instance()->Update();

	countBlock.Update();
	stageRap.Update();
	stageComment.Update();

	// スタミナアイテムの更新処理
	if (!readyToStartRoundFlag) {
		StaminaItemMgr::Instance()->Update(playerHomeBase.GetRightUpPos(), enemyHomeBase.GetLeftDownPos());
	}

	// スタミナアイテムの当たり判定処理
	int healAmount = StaminaItemMgr::Instance()->CheckHit(&CharacterManager::Instance()->Left()->pos, 30, 70, StaminaItem::CHARA_ID::LEFT, CharacterManager::Instance()->Left()->GetPilotPosPtr());
	CharacterManager::Instance()->Left()->HealStamina(healAmount);
	healAmount = StaminaItemMgr::Instance()->CheckHit(&CharacterManager::Instance()->Right()->pos, 90, 70, StaminaItem::CHARA_ID::SCORE, CharacterManager::Instance()->Right()->GetPilotPosPtr());
	healAmount = StaminaItemMgr::Instance()->CheckHit(&CharacterManager::Instance()->Right()->pos, 90, 70, StaminaItem::CHARA_ID::RARE_SCORE, CharacterManager::Instance()->Right()->GetPilotPosPtr());
	CharacterManager::Instance()->Right()->HealStamina(healAmount);


	if (!Camera::Instance()->Active())
	{
		Camera::Instance()->Zoom(CharacterManager::Instance()->Left()->pos, CharacterManager::Instance()->Right()->pos);
	}
	else if (disappearFlag)
	{
		//Camera::Instance()->zoom = KuroMath::Lerp(ScrollMgr::Instance()->zoom, , 0.1f);
	}

	//else {

		//ScrollMgr::Instance()->lineCenterOffset = {};

	//}


	bool noGenerateFlag = StageMgr::Instance()->GetStageInfo(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum()).generatorType == NON_GENERATE;
	//敵キャラがプレイヤーにある程度近付いたら反対側に吹っ飛ばす機能。
	bool isBlockEmpty = countBlock.CheckNowNomberIsZero() && noGenerateFlag;
	bool timeUpFlag = GameTimer::Instance()->TimeUpFlag();

	bool isStuckDead = CharacterManager::Instance()->Left()->isStuckDead || CharacterManager::Instance()->Right()->isStuckDead;

	if (Vec2<float>(CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Right()->pos).Length() <= DistanceCounter::Instance()->DEAD_LINE || isBlockEmpty || timeUpFlag || isStuckDead)
	{
		// 終了演出が行われていなかったら
		if (!roundFinishFlag)
		{
			roundFinishFlag = true;

			float nowBlockCount = countBlock.countNowBlockNum;
			float maxBclokCount = countBlock.countAllBlockNum;
			float destroyRate = countBlock.GetBlockNum() / maxBclokCount;


			RoundFinishEffect::Instance()->Start(isBlockEmpty, destroyRate, Camera::Instance()->zoom);
			DistanceCounter::Instance()->isExpSmall = true;
		}
	}

	// 紐の距離を計算するクラスを更新する。
	if (!roundFinishFlag) {
		DistanceCounter::Instance()->Update();
	}

	// ラウンド数のUIを更新。
	RoundCountMgr::Instance()->Update();

	// 背景パーティクルを更新。
	BackGroundParticleMgr::Instance()->Update();

	BackGroundParticleMgr::Instance()->CheckHit(CharacterManager::Instance()->Left()->pos, CharacterManager::Instance()->Left()->size.x);
	BackGroundParticleMgr::Instance()->CheckHit(CharacterManager::Instance()->Right()->pos, CharacterManager::Instance()->Right()->size.x);

}

void Game::Draw()
{
	int stageNum = SelectStage::Instance()->GetStageNum();
	int roomNum = SelectStage::Instance()->GetRoomNum();

	/*===== 描画処理 =====*/
	//BackGround::Instance()->Draw();

	// 背景パーティクルを描画。
	BackGroundParticleMgr::Instance()->Draw();

	if (stageNum != prevDrawChipStageNum || roomNum != prevDrawChipRoomNum)
	{
	}

	prevDrawChipStageNum = stageNum;
	prevDrawChipRoomNum = roomNum;
	DrawMapChip(*mapData, stageNum, roomNum);


	stageComment.Draw();

	// ラウンド終了時のパーティクルを描画
	RoundFinishParticleMgr::Instance()->Draw();


	if (roundChangeEffect.readyFlag || drawCharaFlag)
	{
		bossHandMgr->Draw();
		playerHandMgr->Draw();
	}

	//playerHomeBase.Draw();
	//enemyHomeBase.Draw();

	// スタミナアイテムの描画処理
	StaminaItemMgr::Instance()->Draw();

	static int CENTER_CHAIN_GRAPH = TexHandleMgr::LoadGraph("resource/ChainCombat/chain.png");
	static int PLAYER_CHAIN_GRAPH = TexHandleMgr::LoadGraph("resource/ChainCombat/chain_player.png");
	static int ENEMY_CHAIN_GRAPH = TexHandleMgr::LoadGraph("resource/ChainCombat/chain_enemy.png");
	static const int CHAIN_THICKNESS = 4;
	// プレイヤーとボス間に線を描画
	if (roundChangeEffect.initGameFlag)
	{
		mapChipGeneratorChangeMap->Draw();
		mapChipGenerator->Draw();


		float disappearRate = CharacterManager::Instance()->Left()->GetAlphaRate();

		// 左のキャラ ~ 右のキャラ間に線を描画
		DrawFunc::DrawLine2DGraph(ScrollMgr::Instance()->Affect(CharacterManager::Instance()->Left()->pos) + GameSceneCameraMove::Instance()->move, ScrollMgr::Instance()->Affect(CharacterManager::Instance()->Right()->pos) + GameSceneCameraMove::Instance()->move, TexHandleMgr::GetTexBuffer(CENTER_CHAIN_GRAPH), CHAIN_THICKNESS * disappearRate);

		// 線分の中心に円を描画
		static int LINE_CENTER_GRAPH = TexHandleMgr::LoadGraph("resource/ChainCombat/line_center.png");
		//DrawFunc::DrawRotaGraph2D(ScrollMgr::Instance()->Affect(DistanceCounter::Instance()->lineCenterPos) + GameSceneCameraMove::Instance()->move, Vec2<float>(1.0f, 1.0f) * disappearRate, 0.0f, TexHandleMgr::GetTexBuffer(LINE_CENTER_GRAPH));


		if (!roundFinishFlag)
		{
			DistanceCounter::Instance()->Draw();
		}
	}

	if (roundChangeEffect.initGameFlag || drawCharaFlag)
	{
		// 残像を描画
		AfterImageMgr::Instance()->Draw();
		CharacterManager::Instance()->Left()->Draw(readyToStartRoundFlag);
		CharacterManager::Instance()->Right()->Draw(readyToStartRoundFlag);
	}

	roundChangeEffect.Draw();

	// クラッシュ時の演出の描画処理。
	CrashEffectMgr::Instance()->Draw();

	ParticleMgr::Instance()->Draw();

	CrashMgr::Instance()->Draw();

	//screenEdgeEffect.Draw();

	//右スティックチュートリアル
	static bool TUTORIAL_GRAPH_LOAD = false;
	static int TUTORIAL_FRAME;
	static Anim R_STICK_ANIM_MEM;
	static const int R_STICK_NOT_INPUT_TIME = 300;
	if (!TUTORIAL_GRAPH_LOAD)
	{
		R_STICK_ANIM_MEM.graph.resize(3);
		TUTORIAL_FRAME = TexHandleMgr::LoadGraph("resource/ChainCombat/tutorial/icon_frame.png");
		TexHandleMgr::LoadDivGraph("resource/ChainCombat/tutorial/r_stick.png", 3, { 3,1 }, R_STICK_ANIM_MEM.graph.data());
		R_STICK_ANIM_MEM.interval = 15;
		R_STICK_ANIM_MEM.loop = true;
		TUTORIAL_GRAPH_LOAD = true;
	}
	static PlayerAnimation R_STICK_ANIM({ R_STICK_ANIM_MEM });

	if (roundChangeEffect.initGameFlag && !roundFinishFlag)
	{
		CharacterManager::Instance()->Left()->DrawUI();
		CharacterManager::Instance()->Right()->DrawUI();

		float rStickInputRate = UsersInput::Instance()->GetRightStickVec(0, { 0.5f,0.5f }).Length();
		if (0.5f <= rStickInputRate)
		{
			rStickNoInputTimer = 0;
		}
		rStickNoInputTimer++;
		if (stageNum == 0 || R_STICK_NOT_INPUT_TIME <= rStickNoInputTimer)
		{
			R_STICK_ANIM.Update();
			Vec2<float>drawPos = CharacterManager::Instance()->Right()->pos + Vec2<float>(0, -160);
			DrawFunc::DrawRotaGraph2D(ScrollMgr::Instance()->Affect(drawPos + Vec2<float>(3, -12.5f)) + GameSceneCameraMove::Instance()->move,
				{ 1.0f,1.0f }, 0.0f, TexHandleMgr::GetTexBuffer(R_STICK_ANIM.GetGraphHandle()));
			DrawFunc::DrawRotaGraph2D(ScrollMgr::Instance()->Affect(drawPos) + GameSceneCameraMove::Instance()->move,
				{ 1.0f,1.0f }, 0.0f, TexHandleMgr::GetTexBuffer(TUTORIAL_FRAME));
		}
	}


	GameTimer::Instance()->Draw();
	countBlock.Draw();
	ScoreManager::Instance()->Draw();

	stageRap.Draw();

	WinCounter::Instance()->Draw();

	StunEffect::Instance()->Draw();

	{
		Vec2<float>leftUpPos = *CharacterManager::Instance()->Right()->GetAreaHitBox().center - CharacterManager::Instance()->Right()->GetAreaHitBox().size / 2.0f;
		Vec2<float>rightDownPos = *CharacterManager::Instance()->Right()->GetAreaHitBox().center + CharacterManager::Instance()->Right()->GetAreaHitBox().size / 2.0f;
		//DrawFunc::DrawBox2D(ScrollMgr::Instance()->Affect(leftUpPos), ScrollMgr::Instance()->Affect(rightDownPos), Color(255, 255, 255, 255), DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	{
		Vec2<float>leftUpPos = *CharacterManager::Instance()->Left()->GetAreaHitBox().center - CharacterManager::Instance()->Left()->GetAreaHitBox().size / 2.0f;
		Vec2<float>rightDownPos = *CharacterManager::Instance()->Left()->GetAreaHitBox().center + CharacterManager::Instance()->Left()->GetAreaHitBox().size / 2.0f;
		//DrawFunc::DrawBox2D(ScrollMgr::Instance()->Affect(leftUpPos), ScrollMgr::Instance()->Affect(rightDownPos), playerHitColor, DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	{
		Vec2<float>leftUpPos = *enemyHomeBase.hitBox.center - enemyHomeBase.hitBox.size / 2.0f;
		Vec2<float>rightDownPos = *enemyHomeBase.hitBox.center + enemyHomeBase.hitBox.size / 2.0f;
		//DrawFunc::DrawBox2D(ScrollMgr::Instance()->Affect(leftUpPos), ScrollMgr::Instance()->Affect(rightDownPos), areaHitColor, DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	// ラウンド終了時の演出を描画。
	RoundFinishEffect::Instance()->Draw();

	// ラウンド数のUIを描画
	RoundCountMgr::Instance()->Draw();

}

void Game::Scramble()
{
	/*===== 引っ張り合いの処理 =====*/
	// 前フレームの線の中心座標を保存
	prevLineCenterPos = lineCenterPos;

	//どちらも動けないとき何もしない
	if (!(CharacterManager::Instance()->Left()->GetCanMove() || CharacterManager::Instance()->Right()->GetCanMove()))return;

	Vec2<float> leftVelGauge;
	Vec2<float> rightVelGauge;

	// 右側のキャラが吹っ飛ぶマップチップに当たった際の移動量の処理を行う。
	CharacterManager::Instance()->Right()->pos += CharacterManager::Instance()->Right()->bounceVel;
	CharacterManager::Instance()->Right()->addLineLength += CharacterManager::Instance()->Right()->bounceVel.Length();

	// 移動量を取得。 優勢ゲージはここで更新。
	double leftVel = CharacterManager::Instance()->Left()->vel.Length() * SlowMgr::Instance()->slowAmount;
	leftVelGauge = CharacterManager::Instance()->Left()->vel * SlowMgr::Instance()->slowAmount;
	double rightVel = CharacterManager::Instance()->Right()->vel.Length() * SlowMgr::Instance()->slowAmount;
	rightVelGauge = CharacterManager::Instance()->Right()->vel * SlowMgr::Instance()->slowAmount;
	double subVel = fabs(fabs(leftVel) - fabs(rightVel));

	// [スタン演出中] は移動させない。 踏ん張り中の場合は、どちらにせよ移動量が限りなく0に近いので移動させても問題がない。
	if (!(StunEffect::Instance()->isActive)) {
		// 振り回され中じゃなかったら移動させる。
		if (!CharacterManager::Instance()->Right()->GetNowSwing()) {
			CharacterManager::Instance()->Left()->pos += leftVelGauge;
		}
		// 振り回され中じゃなかったら移動させる。
		if (!CharacterManager::Instance()->Left()->GetNowSwing() && !CharacterManager::Instance()->Left()->isStopPartner) {
			CharacterManager::Instance()->Right()->pos += rightVelGauge;
		}
		else if (CharacterManager::Instance()->Left()->isStopPartner) {
			CharacterManager::Instance()->Right()->pos += leftVelGauge;
		}
	}

	// 線分の長さ
	float charaLength = 0;
	float LINE = CharacterInterFace::LINE_LENGTH * 2 + (CharacterManager::Instance()->Left()->addLineLength + CharacterManager::Instance()->Right()->addLineLength);

	// どちらかが踏ん張っているか。
	bool isHoldNow = CharacterManager::Instance()->Left()->isHold || CharacterManager::Instance()->Right()->isHold;

	// どちらかが振り回しているか。
	bool isSwingNow = CharacterManager::Instance()->Left()->GetNowSwing() || CharacterManager::Instance()->Right()->GetNowSwing();

	// 距離を求める。
	charaLength = Vec2<float>(CharacterManager::Instance()->Left()->pos).Distance(CharacterManager::Instance()->Right()->pos);

	// ボスをプレイヤーの方に移動させる。
	if (LINE < charaLength) {

		// 押し戻し量
		float moveLength = charaLength - LINE;

		// 押し戻し方向
		Vec2<float> moveDir = Vec2<float>(CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Right()->pos);
		moveDir.Normalize();

		// 押し戻す。
		CharacterManager::Instance()->Right()->pos += moveDir * Vec2<float>(moveLength, moveLength);


		// 引っかかり判定だったら
		if (CharacterManager::Instance()->Right()->GetStackFlag()) {

			CharacterManager::Instance()->Right()->addLineLength += moveLength;
			//CharacterManager::Instance()->Left()->pos += -moveDir * Vec2<float>(moveLength, moveLength);

			// 引っかかっている場合は更に押し戻す。(壁ズリを表現するため)
			if (0 < CharacterManager::Instance()->Right()->addLineLength) {
				CharacterManager::Instance()->Right()->pos += moveDir * Vec2<float>(moveLength, moveLength);
			}

		}
	}

	const float ADD_LINE_LENGTH_SUB_AMOUNT = addLineLengthSubAmount;

	// どちらのキャラも引っかかっているか
	bool isBothStuck = CharacterManager::Instance()->Right()->GetStackFlag() && CharacterManager::Instance()->Left()->GetStackFlag();
	// 引っかかり判定じゃなかったらだんだん短くする。
	Vec2<float> movedVel = (CharacterManager::Instance()->Right()->pos - CharacterManager::Instance()->Right()->prevPos);
	movedVel += (CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Left()->prevPos);
	// 右側の紐の処理
	if (0 < CharacterManager::Instance()->Right()->addLineLength) {

		// 引く量 0未満にならないようにするため。
		float subAmount = ADD_LINE_LENGTH_SUB_AMOUNT;
		if (CharacterManager::Instance()->Right()->addLineLength < ADD_LINE_LENGTH_SUB_AMOUNT) {
			subAmount = CharacterManager::Instance()->Right()->addLineLength;
		}

		// 移動中は引かない。
		if (ADD_LINE_LENGTH_SUB_AMOUNT < CharacterManager::Instance()->Right()->vel.Length()) {
			subAmount = 0;
		}

		// 紐加算量をへらす。
		CharacterManager::Instance()->Right()->addLineLength -= subAmount;

		// キャラクターを紐加算量が減った方向に移動させる。
		Vec2<float> charaDir = (CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Right()->pos).GetNormal();
		CharacterManager::Instance()->Right()->vel += charaDir * subAmount / 10.0f;

		// 引いた分移動させる。
		float charaLength = (CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Right()->pos).Length();
		// [今の長さ] が [初期長さ * 2] + [左の紐の長さ] 以上だったら処理を行う。
		if (CharacterInterFace::LINE_LENGTH * 2.0f + CharacterManager::Instance()->Left()->addLineLength < charaLength) {

			// 右側が引っかかっていたら。
			if (CharacterManager::Instance()->Right()->GetStackFlag()) {
				// 右側が引っかかっているときは代わりに左側を動かす。
				CharacterManager::Instance()->Left()->pos += (CharacterManager::Instance()->Right()->pos - CharacterManager::Instance()->Left()->pos).GetNormal() * subAmount;
			}
			else {
				CharacterManager::Instance()->Right()->pos += (CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Right()->pos).GetNormal() * subAmount;
			}
		}

	}

	if (CharacterManager::Instance()->Right()->addLineLength < 0) CharacterManager::Instance()->Right()->addLineLength = 0;


	// 左側の紐の処理
	if (!isBothStuck && 0 < CharacterManager::Instance()->Left()->addLineLength && !isSwingNow) {

		// 引く量 0未満にならないようにするため。
		float subAmount = ADD_LINE_LENGTH_SUB_AMOUNT;
		if (CharacterManager::Instance()->Left()->addLineLength < ADD_LINE_LENGTH_SUB_AMOUNT) {
			subAmount = CharacterManager::Instance()->Left()->addLineLength;
		}

		// 移動中は引かない。
		if (ADD_LINE_LENGTH_SUB_AMOUNT < CharacterManager::Instance()->Left()->vel.Length()) {
			subAmount = 0;
		}


		// 紐加算量をへらす。
		CharacterManager::Instance()->Left()->addLineLength -= subAmount;

		// キャラクターを紐加算量が減った方向に移動させる。
		Vec2<float> charaDir = (CharacterManager::Instance()->Right()->pos - CharacterManager::Instance()->Left()->pos).GetNormal();
		CharacterManager::Instance()->Left()->vel += charaDir * subAmount / 10.0f;

		// 引いた分移動させる。
		float charaLength = (CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Right()->pos).Length();
		// [今の長さ] が [初期長さ * 2] + [左の紐の長さ] 以上だったら処理を行う。
		if (CharacterInterFace::LINE_LENGTH * 2.0f + CharacterManager::Instance()->Right()->addLineLength < charaLength) {

			// 動いていたら
			if (1.0f < movedVel.Length()) {
			}
			// 左側が引っかかっていたら。
			else if (CharacterManager::Instance()->Left()->GetStackFlag()) {
				// 左側が引っかかっているときは代わりに右側を動かす。
				CharacterManager::Instance()->Right()->pos += (CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Right()->pos).GetNormal() * subAmount;
			}
			else {
				CharacterManager::Instance()->Left()->pos += (CharacterManager::Instance()->Right()->pos - CharacterManager::Instance()->Left()->pos).GetNormal() * subAmount;
			}
		}

	}

	if (CharacterManager::Instance()->Right()->addLineLength < 0) CharacterManager::Instance()->Right()->addLineLength = 0;
	if (CharacterManager::Instance()->Left()->addLineLength < 0) CharacterManager::Instance()->Left()->addLineLength = 0;

	isCatchMapChipBoss = false;
	isCatchMapChipPlayer = false;

}

void Game::CalCenterPos()
{
	/*===== 中心点を求める処理 =====*/

	// 本当はScrambleの一番うしろに入れていた処理なんですが、押し戻しをした後に呼ぶ必要が出てきたので関数で分けました。

	auto& left = CharacterManager::Instance()->Left();
	auto& right = CharacterManager::Instance()->Right();

	// 移動量に応じて本来あるべき長さにする。
	Vec2<float> prevSubPos = CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Left()->prevPos;
	float horaiAddLineLength = (prevSubPos.Length() / CharacterManager::Instance()->Left()->MOVE_SPEED_PLAYER);
	horaiAddLineLength *= CharacterManager::Instance()->Left()->ADD_LINE_LENGTH_VEL;
	if (CharacterManager::Instance()->Left()->addLineLength < horaiAddLineLength && 1.0f < CharacterManager::Instance()->Left()->vel.Length() && 1.0f < prevSubPos.Length()) {
		CharacterManager::Instance()->Left()->addLineLength = horaiAddLineLength;
	}
	prevSubPos = CharacterManager::Instance()->Right()->pos - CharacterManager::Instance()->Right()->prevPos;
	horaiAddLineLength = (prevSubPos.Length() / CharacterManager::Instance()->Right()->MOVE_SPEED_PLAYER);
	horaiAddLineLength *= CharacterManager::Instance()->Right()->ADD_LINE_LENGTH_VEL;
	if (CharacterManager::Instance()->Right()->addLineLength < horaiAddLineLength && 1.0f < CharacterManager::Instance()->Right()->vel.Length() && 1.0f < prevSubPos.Length()) {
		CharacterManager::Instance()->Right()->addLineLength = horaiAddLineLength;
	}

	// 現在の距離が加算されている紐の長さより短かったら、addLineValueを短くする。
	float nowLength = (left->pos - right->pos).Length();
	if (nowLength < CharacterInterFace::LINE_LENGTH * 2.0f + left->addLineLength + right->addLineLength) {

		// オーバーしている量を求める。
		float overLength = (CharacterInterFace::LINE_LENGTH * 2.0f + left->addLineLength + right->addLineLength) - nowLength;
		// オーバーしている量の半分を求める。
		float halfOverLength = overLength / 2.0f;

		// まずは左側から短くする。

		left->addLineLength -= halfOverLength;

		// 0以下になったら、その量を保存しておく。
		float leftOverLength = 0;
		if (left->addLineLength < 0) {

			leftOverLength = fabs(left->addLineLength);

			// 紐の長さが0未満にならないようにする。
			left->addLineLength = 0;

		}

		// 次に右側を短くする。

		right->addLineLength -= halfOverLength + leftOverLength;

		// 紐の長さが0未満にならないようにする。
		if (right->addLineLength < 0) {

			right->addLineLength = 0;

		}
	}

	// 紐の中心点を計算
	{
		float distance = (CharacterManager::Instance()->Right()->pos - CharacterManager::Instance()->Left()->pos).Length();
		Vec2<float> bossDir = CharacterManager::Instance()->Right()->pos - CharacterManager::Instance()->Left()->pos;
		bossDir.Normalize();

		// ボスとプレイヤー間の距離が規定値以下だったら
		//if (distance < CharacterInterFace::LINE_LENGTH + CharacterInterFace::LINE_LENGTH) {
		//	// 既定値以下だったら団子化減少を防ぐために、二点間の真ん中の座標にする。
		//	lineCenterPos = CharacterManager::Instance()->Left()->pos + bossDir * Vec2<float>(distance / 2.0f, distance / 2.0f);
		//}
		//else {
			// 規定値以上だったら普通に場所を求める。

		auto& right = CharacterManager::Instance()->Right();
		auto& left = CharacterManager::Instance()->Left();

		Vec2<float> rightPos = right->pos;
		rightPos += (left->pos - right->pos).GetNormal() * right->addLineLength;

		Vec2<float> leftPos = left->pos;
		leftPos += (right->pos - left->pos).GetNormal() * left->addLineLength;

		float length = (leftPos - rightPos).Length();
		length /= 2.0f;
		Vec2<float> dir = (leftPos - rightPos).GetNormal();

		lineCenterPos = rightPos + dir * length;

		//float playerLineLength = CharacterManager::Instance()->Left()->LINE_LENGTH + CharacterManager::Instance()->Left()->addLineLength;
		//lineCenterPos = CharacterManager::Instance()->Left()->pos + bossDir * Vec2<float>(playerLineLength, playerLineLength);
	//}
	}

}

Vec2<float> Game::GetStageSize()
{
	static const float CHIP_SIZE = 64;
	int sizeX = mapData[0].size();
	int sizeY = mapData->size();
	return Vec2<float>(CHIP_SIZE * sizeX, CHIP_SIZE * sizeY);
}

void Game::SwitchingStage()
{
	const bool enableToSelectStageFlag = 0 < debugStageData[0];
	const bool enableToSelectStageFlag2 = debugStageData[0] < StageMgr::Instance()->GetMaxStageNumber() - 1;
	//マップの切り替え
	const bool left = UsersInput::Instance()->KeyOnTrigger(DIK_LEFT) || UsersInput::Instance()->ControllerOnTrigger(0, DPAD_LEFT);
	const bool right = UsersInput::Instance()->KeyOnTrigger(DIK_RIGHT) || UsersInput::Instance()->ControllerOnTrigger(0, DPAD_RIGHT);

	int maxStageNum = StageMgr::Instance()->GetEnableToUseStageNumber();


	//部屋か番号に切り替え
	if (left && 0 < nowSelectNum)
	{
		--nowSelectNum;
		--debugStageData[0];
	}
	if (right && nowSelectNum < maxStageNum)
	{
		++nowSelectNum;
		++debugStageData[0];
	}

	if (debugStageData[0] <= 0)
	{
		debugStageData[0] = 0;
		nowSelectNum = 0;
	}
	if (maxStageNum <= debugStageData[0])
	{
		debugStageData[0] = maxStageNum - 1;
		nowSelectNum = maxStageNum - 1;
	}


	//const bool done = /*UsersInput::Instance()->KeyOnTrigger(DIK_RETURN)
	//	||*/ (UsersInput::Instance()->ControllerInput(0, A) && UsersInput::Instance()->ControllerOnTrigger(0, B))
	//	|| (UsersInput::Instance()->ControllerOnTrigger(0, A) && UsersInput::Instance()->ControllerInput(0, B));
	//if (done)
	//{
	//	SelectStage::Instance()->SelectStageNum(debugStageData[0]);
	//	SelectStage::Instance()->SelectRoomNum(debugStageData[1]);

	//	InitGame(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum());
	//}

}

void Game::DeterminationOfThePosition()
{

	//プレイヤー陣地と敵の判定
	if (playerHomeBase.Collision(CharacterManager::Instance()->Right()->GetAreaHitBox()) && !roundFinishFlag && !readyToStartRoundFlag)
	{
		//プレイヤー勝利
		WinCounter::Instance()->RoundFinish(lineCenterPos, true, CharacterManager::Instance()->Right()->pos);
		CharacterManager::Instance()->Right()->OnKnockOut();
		roundFinishFlag = true;
		playerOrEnemeyWinFlag = true;
		gameStartFlag = false;

		screenEdgeEffect.LeftPlayerWin(120);

		// 両キャラの予測線を消す。
		CharacterManager::Instance()->Right()->InitSwingLineSegmetn();
		CharacterManager::Instance()->Left()->InitSwingLineSegmetn();

	}

	//敵陣地とプレイヤーの判定
	if (enemyHomeBase.Collision(CharacterManager::Instance()->Left()->GetAreaHitBox()) && !roundFinishFlag && !readyToStartRoundFlag)
	{
		//敵勝利
		WinCounter::Instance()->RoundFinish(lineCenterPos, false, CharacterManager::Instance()->Left()->pos);
		CharacterManager::Instance()->Left()->OnKnockOut();
		roundFinishFlag = true;
		playerOrEnemeyWinFlag = false;
		gameStartFlag = false;

		areaHitColor = Color(255, 0, 0, 255);
		playerHitColor = Color(255, 0, 0, 255);

		screenEdgeEffect.RightPlayerWin(120);

		// 両キャラの予測線を消す。
		CharacterManager::Instance()->Right()->InitSwingLineSegmetn();
		CharacterManager::Instance()->Left()->InitSwingLineSegmetn();
	}

}

void Game::RoundStartEffect(const bool& Loop, const MapChipArray& tmpMapData)
{

	//ラウンド開始時の演出開始
	if (readyToStartRoundFlag)
	{
		roundChangeEffect.Start(WinCounter::Instance()->GetNowRound(), playerOrEnemeyWinFlag);
		Vec2<float>winSize;
		winSize.x = static_cast<float>(WinApp::Instance()->GetWinSize().x);
		winSize.y = static_cast<float>(WinApp::Instance()->GetWinSize().y);

		//Vec2<float> responePos = Vec2<float>(((mapData[0].size() * MAP_CHIP_SIZE) / 2.0f) - 100.0f, (mapData.size() * MAP_CHIP_SIZE) / 2.0f);
		//responePos.y -= -cameraBasePos.y * 2.0f;
		ScrollMgr::Instance()->Warp(responeScrollPos);

		//プレイヤーと敵の座標初期化
		if (roundChangeEffect.readyToInitFlag && !roundChangeEffect.initGameFlag)
		{
			roundChangeEffect.initGameFlag = true;
			if (!AudioApp::Instance()->NowPlay(bgm))
			{
				AudioApp::Instance()->ChangeVolume(bgm, 0.1f);
				AudioApp::Instance()->PlayWave(bgm, true);
			}
		}

		//登場演出
		if (roundChangeEffect.initGameFlag)
		{
			// シェイクを発生させる。
			UsersInput::Instance()->ShakeController(0, 1.0f, 6);

			CharacterManager::Instance()->Left()->Appear();
			CharacterManager::Instance()->Right()->Appear();
			if (CharacterManager::Instance()->Left()->CompleteAppear() && CharacterManager::Instance()->Right()->CompleteAppear())	//どちらのキャラも登場演出完了
			{
				//ゲームスタート
				readyToStartRoundFlag = false;
				gameStartFlag = true;
				roundTimer = 0;
				CharacterManager::Instance()->Left()->SetCanMove(true);
				CharacterManager::Instance()->Right()->SetCanMove(true);
				CharacterManager::Instance()->Left()->SetHitCheck(true);
				CharacterManager::Instance()->Right()->SetHitCheck(true);
				GameTimer::Instance()->SetInterruput(false);
			}
		}
		//gameStartFlag = true;
		//SelectStage::Instance()->resetStageFlag = true;
		//readyToStartRoundFlag = false;
		float size = (tmpMapData[0].size() * MAP_CHIP_SIZE) - 400.0f;
		miniMap.Init(size);
	}

}

void Game::RoundFinishEffect(const bool& Loop)
{

	//ラウンド終了演出開始
	if (roundFinishFlag && !readyToStartRoundFlag)
	{
		//動けなくする
		CharacterManager::Instance()->Left()->SetCanMove(false);
		CharacterManager::Instance()->Right()->SetCanMove(false);
		CharacterManager::Instance()->Left()->SetHitCheck(false);
		CharacterManager::Instance()->Right()->SetHitCheck(false);

		//時間計測ストップ
		GameTimer::Instance()->SetInterruput(true);

		const int nowRoomNum = SelectStage::Instance()->GetRoomNum();

		bool isFinalRound = false;

		if (!SelectStage::Instance()->HaveNextLap() && RoundFinishEffect::Instance()->changeMap)
		{
			ResultTransfer::Instance()->resultScore = ScoreManager::Instance()->GetScore();
			if (WinCounter::Instance()->Winner() == LEFT_TEAM)ResultTransfer::Instance()->winner = CharacterManager::Instance()->Left()->GetCharacterName();
			else ResultTransfer::Instance()->winner = CharacterManager::Instance()->Right()->GetCharacterName();

			isFinalRound = true;

		}

		isFinalRound = !SelectStage::Instance()->HaveNextLap();
		if (isFinalRound)mapChipGeneratorChangeMap->SetActive(false);

		// リザルトシーンへ移行できる状態だったら。
		if (RoundFinishEffect::Instance()->isEndResultScene && isFinalRound) {

			// 上方向に移動させる。
			++trasitionTimer;
			if (TRANSITION_TIMER <= trasitionTimer) {

				// リザルトシーンへ遷移させる。
				turnResultScene = true;

				// リザルトシーンではDistanceCounterのPosを中心に星を生成するので、演出用でずらした値を入れる。
				DistanceCounter::Instance()->lineCenterPos.y += TRANSITION_MOVE_Y;

				// 各変数を初期化
				trasitionTimer = 0;

			}

			// 上に移動させる。
			GameSceneCameraMove::Instance()->move.y += (-TRANSITION_MOVE_Y - GameSceneCameraMove::Instance()->move.y) / 30.0f;

		}

		// ラウンド終了時演出の更新処理
		RoundFinishEffect::Instance()->Update(DistanceCounter::Instance()->lineCenterPos);

		// ラウンド終了時演出が終わったかどうか
		bool isEnd = RoundFinishEffect::Instance()->isEnd;

		if (isEnd) {

			//次のラウンドへ
				//練習モード
			if (Loop)WinCounter::Instance()->Reset();

			++roundTimer;


			if (60 <= roundTimer)
			{

				// 最終ラウンドじゃなかったら
				if (!isFinalRound) {

					SelectStage::Instance()->SelectRoomNum(nowRoomNum + 1);
					StageMgr::Instance()->SetLocalMapChipData(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum());
					mapData = StageMgr::Instance()->GetLocalMap();
					mapChipGeneratorChangeMap->RegisterMap();
					RoundFinishEffect::Instance()->changeMap = false;
					readyToStartRoundFlag = true;
					roundFinishFlag = false;

				}

				InitCountBlock();

				stageRap.Increment();
				DistanceCounter::Instance()->isExpSmall = false;

				// ランド終了時に初期化したい変数を初期化する。
				CharacterManager::Instance()->Left()->InitRoundFinish();
				CharacterManager::Instance()->Right()->InitRoundFinish();

				//InitGame(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum());
				gameTimer = StageMgr::Instance()->GetMaxTime(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum());
				GameTimer::Instance()->Init(gameTimer);
				GameTimer::Instance()->Start();

				GeneratorInit();

				rStickNoInputTimer = 0;

				// 背景パーティクルを更新
				//BackGroundParticleMgr::Instance()->Init();
				//int stageNum = SelectStage::Instance()->GetStageNum();
				//int roomNum = SelectStage::Instance()->GetRoomNum();
				//BackGroundParticleMgr::Instance()->StageStartGenerate(Vec2<float>(StageMgr::Instance()->GetMapIdxSize(stageNum, roomNum).x * MAP_CHIP_SIZE, StageMgr::Instance()->GetMapIdxSize(stageNum, roomNum).y * MAP_CHIP_SIZE));
			}

			drawCharaFlag = true;


			// AddLineLengthを伸ばす。
			CharacterManager::Instance()->Right()->addLineLength = CharacterManager::Instance()->Right()->pos.Distance(CharacterManager::Instance()->Left()->pos) - CharacterInterFace::LINE_LENGTH * 2.0f;




		}

		//勝利数カウント演出
		//if (!WinCounter::Instance()->GetNowAnimation())
		//{
		//	//どちらかが３勝とったらゲーム終了
		//	if (WinCounter::Instance()->GetGameFinish() && !Loop)
		//	{
		//		ResultTransfer::Instance()->resultScore = ScoreManager::Instance()->GetScore();
		//		if (WinCounter::Instance()->Winner() == LEFT_TEAM)ResultTransfer::Instance()->winner = CharacterManager::Instance()->Left()->GetCharacterName();
		//		else ResultTransfer::Instance()->winner = CharacterManager::Instance()->Right()->GetCharacterName();
		//		turnResultScene = true;
		//	}
		//	//次のラウンドへ
		//	else
		//	{
		//		//練習モード
		//		if (Loop)WinCounter::Instance()->Reset();

		//		++roundTimer;

		//		if (60 <= roundTimer)
		//		{
		//			readyToStartRoundFlag = true;
		//			roundFinishFlag = false;
		//			InitGame(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum());
		//		}
		//	}
		//}
	}

}

void Game::InitCountBlock()
{
	int countBlockNum = COUNT_BLOCK_MAX;
	switch (SelectStage::Instance()->GetStageNum())
	{
	case 4:
		countBlockNum = 2930;
		break;
	case 6:
		countBlockNum = 4300;
		break;
	case 9:
		countBlockNum = 5885;
		break;
	case 11:
		countBlockNum = 1769;
		break;
	case 14:
		countBlockNum = 1833;
		break;
	default:
		break;
	}


	bool nonFlag = StageMgr::Instance()->GetStageInfo(SelectStage::Instance()->GetStageNum(), SelectStage::Instance()->GetRoomNum()).generatorType != NON_GENERATE;

	int stageNum = SelectStage::Instance()->GetStageNum();
	int roomNum = SelectStage::Instance()->GetRoomNum();
	bool firstStageFlag = roomNum == 0;
	if (nonFlag)
	{
		countBlock.Init(countBlockNum, false, firstStageFlag);
		ScoreKeep::Instance()->Init(StageMgr::Instance()->GetMaxLap(stageNum), countBlockNum);
	}
	else
	{
		countBlock.Init(StageMgr::Instance()->GetAllLocalWallBlocksNum(), true, firstStageFlag);
		ScoreKeep::Instance()->Init(StageMgr::Instance()->GetMaxLap(stageNum), StageMgr::Instance()->GetAllRoomWallBlocksNum(stageNum));
	}
}
