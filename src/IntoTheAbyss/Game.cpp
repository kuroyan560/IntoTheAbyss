#include "Game.h"
#include"MapChipCollider.h"
#include"ShakeMgr.h"
#include "BulletMgr.h"
#include"ScrollMgr.h"
#include"BulletParticleMgr.h"
#include"AuraBlock.h"
#include"ViewPort.h"
#include"MovingBlockMgr.h"
#include"Bullet.h"
#include"Collider.h"

#include"KuroFunc.h"
#include"KuroEngine.h"
#include"TexHandleMgr.h"
#include"DrawFunc.h"

bool Game::CheckUsedData(std::vector<Vec2<float>> DATA, Vec2<float> DATA2)
{
	for (int i = 0; i < DATA.size(); ++i)
	{
		if (DATA[i] == DATA2)
		{
			return true;
		}
	}
	return false;
}

void Game::DrawMapChip(const vector<vector<int>>& mapChipData, vector<vector<MapChipDrawData>>& mapChipDrawData, const int& mapBlockGraph, const int& stageNum, const int& roomNum)
//void Game::DrawMapChip(const vector<vector<int>>& mapChipData, vector<vector<MapChipDrawData>>& mapChipDrawData, const int& mapBlockGraph, const int& stageNum, const int& roomNum)
{
	// 描画するチップのサイズ
	const float DRAW_MAP_CHIP_SIZE = MAP_CHIP_SIZE * ScrollMgr::Instance()->zoom;

	// マップチップの縦の要素数を取得。
	const int HEIGHT = mapChipData.size();
	for (int height = 0; height < HEIGHT; ++height) {

		// マップチップの横の要素数を取得。
		const int WIDTH = mapChipData[height].size();
		for (int width = 0; width < WIDTH; ++width) {

			// ブロック以外だったら処理を飛ばす。
			bool blockFlag = (mapChipData[height][width] >= 1 && mapChipData[height][width] <= 9);
			bool doorFlag = (mapChipData[height][width] >= 20 && mapChipData[height][width] <= 29);
			if (blockFlag || doorFlag)
			{
				// スクロール量から描画する位置を求める。
				const float centerX = width * DRAW_MAP_CHIP_SIZE - ScrollMgr::Instance()->scrollAmount.x * ScrollMgr::Instance()->zoom;
				const float centerY = height * DRAW_MAP_CHIP_SIZE - ScrollMgr::Instance()->scrollAmount.y * ScrollMgr::Instance()->zoom;

				// 画面外だったら描画しない。
				if (centerX < -DRAW_MAP_CHIP_SIZE || centerX > WinApp::Instance()->GetWinSize().x + DRAW_MAP_CHIP_SIZE) continue;
				if (centerY < -DRAW_MAP_CHIP_SIZE || centerY > WinApp::Instance()->GetWinSize().y + DRAW_MAP_CHIP_SIZE) continue;


				vector<MapChipAnimationData*>tmpAnimation = StageMgr::Instance()->animationData;
				int handle = -1;
				//アニメーションフラグが有効ならアニメーション用の情報を行う
				if (mapChipDrawData[height][width].animationFlag)
				{
					int arrayHandle = mapChipDrawData[height][width].handle;
					++mapChipDrawData[height][width].interval;
					//アニメーションの間隔
					if (mapChipDrawData[height][width].interval % tmpAnimation[arrayHandle]->maxInterval == 0)
					{
						++mapChipDrawData[height][width].animationNum;
						mapChipDrawData[height][width].interval = 0;
					}
					//アニメーション画像の総数に達したら最初に戻る
					if (tmpAnimation[arrayHandle]->handle.size() <= mapChipDrawData[height][width].animationNum)
					{
						mapChipDrawData[height][width].animationNum = 0;
					}
					//分割したアニメーションの画像から渡す
					handle = tmpAnimation[arrayHandle]->handle[mapChipDrawData[height][width].animationNum];
				}
				else
				{
					handle = mapChipDrawData[height][width].handle;
				}

				Vec2<float> pos(centerX + ShakeMgr::Instance()->shakeAmount.x * ScrollMgr::Instance()->zoom, centerY + ShakeMgr::Instance()->shakeAmount.y * ScrollMgr::Instance()->zoom);
				pos += mapChipDrawData[height][width].offset;
				if (0 <= handle)
				{
					DrawFunc::DrawRotaGraph2D({ pos.x, pos.y }, 1.6f * ScrollMgr::Instance()->zoom, mapChipDrawData[height][width].radian, TexHandleMgr::GetTexBuffer(handle));
				}
			}
		}
	}
}

Vec2<float> Game::GetPlayerResponePos(const int& STAGE_NUMBER, const int& ROOM_NUMBER, const int& DOOR_NUMBER, Vec2<float> DOOR_MAPCHIP_POS)
{
	Vec2<float> doorPos;
	int roopCount = 0;
	//扉の一番下の座標を探索する
	for (int y = 0; 1; ++y)
	{
		Vec2<float> chipData = { DOOR_MAPCHIP_POS.x,DOOR_MAPCHIP_POS.y + y };
		int chipNum = StageMgr::Instance()->GetMapChipBlock(STAGE_NUMBER, ROOM_NUMBER, chipData);

		//どんどん下を探索してドアじゃない場所に出たら、一個前が一番下の扉
		if (chipNum != DOOR_NUMBER)
		{
			if (1 <= y)
			{
				doorPos = { DOOR_MAPCHIP_POS.x,(DOOR_MAPCHIP_POS.y + y) - 1 };
			}
			else
			{
				doorPos = { DOOR_MAPCHIP_POS.x,(DOOR_MAPCHIP_POS.y + y) };
			}
			break;
		}
		//無限ループに入ったらアサートする
		++roopCount;
		if (1000 <= roopCount)
		{
			assert(0);
		}
	}


	//左右どちらかの扉から、もしくは奥扉からリスポーンさせる場合-----------------------
	array<Vec2<float>, 2> checkWall;
	checkWall[0] = { doorPos.x + 1,doorPos.y };
	checkWall[1] = { doorPos.x - 1,doorPos.y };
	int rightChip = StageMgr::Instance()->GetMapChipBlock(STAGE_NUMBER, ROOM_NUMBER, checkWall[0]);
	int leftChip = StageMgr::Instance()->GetMapChipBlock(STAGE_NUMBER, ROOM_NUMBER, checkWall[1]);

	//右に壁がある場合
	if ((rightChip == 1 && leftChip == 0) || (rightChip == 1 && leftChip == -1))
	{
		//左に2ブロック離れた所にリスポーンさせる
		return Vec2<float>((doorPos.x - 2) * 50.0f, doorPos.y * 50.0f);
	}
	//左に壁がある場合
	else if ((rightChip == 0 && leftChip == 1) || (rightChip == 0 && leftChip == -1))
	{
		//右に2ブロック離れた所にリスポーンさせる
		return Vec2<float>((doorPos.x + 2) * 50.0f, doorPos.y * 50.0f);
	}
	//左右どちらとも壁が無かった場合
	else if (rightChip == 0 && leftChip == 0)
	{
		//扉座標にリスポーンさせる
		return Vec2<float>(doorPos.x * 50.0f, doorPos.y * 50.0f);
	}
	//左右どちらかの扉から、もしくは奥扉からリスポーンさせる場合-----------------------


	//上下どちらかの扉からリスポーンさせる場合-----------------------
	//一番左と一番右の扉ブロックを探索する
	Vec2<float> rightDoor, leftDoor;//一番端の扉座標を探索する

	roopCount = 0;
	//一番左の扉座標探索
	for (int i = 0; 1; ++i)
	{
		Vec2<float> chipData = { DOOR_MAPCHIP_POS.x - i,DOOR_MAPCHIP_POS.y };
		int chipNum = StageMgr::Instance()->GetMapChipBlock(STAGE_NUMBER, ROOM_NUMBER, chipData);
		if (chipNum != DOOR_NUMBER)
		{
			leftDoor = { DOOR_MAPCHIP_POS.x - i + 1,DOOR_MAPCHIP_POS.y };
			break;
		}

		//無限ループに入ったらアサートする
		++roopCount;
		if (1000 <= roopCount)
		{
			assert(0);
		}
	}

	roopCount = 0;
	//一番右の扉座標探索
	for (int i = 0; 1; ++i)
	{
		Vec2<float> chipData = { DOOR_MAPCHIP_POS.x + i,DOOR_MAPCHIP_POS.y };
		int chipNum = StageMgr::Instance()->GetMapChipBlock(STAGE_NUMBER, ROOM_NUMBER, chipData);
		if (chipNum != DOOR_NUMBER)
		{
			rightDoor = { DOOR_MAPCHIP_POS.x + i - 1,DOOR_MAPCHIP_POS.y };
			break;
		}

		//無限ループに入ったらアサートする
		++roopCount;
		if (1000 <= roopCount)
		{
			assert(0);
		}
	}

	checkWall[0] = { rightDoor.x + 1,rightDoor.y };
	checkWall[1] = { leftDoor.x - 1,leftDoor.y };
	rightChip = StageMgr::Instance()->GetMapChipBlock(STAGE_NUMBER, ROOM_NUMBER, checkWall[0]);
	leftChip = StageMgr::Instance()->GetMapChipBlock(STAGE_NUMBER, ROOM_NUMBER, checkWall[1]);

	//上下の壁を調べる
	array<Vec2<float>, 2> checkTopWall;
	checkTopWall[0] = { rightDoor.x,rightDoor.y - 1 };
	checkTopWall[1] = { rightDoor.x,rightDoor.y + 1 };
	int topWall = StageMgr::Instance()->GetMapChipBlock(STAGE_NUMBER, ROOM_NUMBER, checkTopWall[0]);
	int downWall = StageMgr::Instance()->GetMapChipBlock(STAGE_NUMBER, ROOM_NUMBER, checkTopWall[1]);


	//両方とも壁があり、上に空間があるなら
	if (rightChip != 0 && leftChip != 0 && topWall == 0)
	{
		//壁が無い方にリスポーンさせる
		Vec2<float> check(leftDoor.x - 2, leftDoor.y - 1);
		Vec2<float> check2(rightDoor.x + 2, rightDoor.y - 1);

		int num = StageMgr::Instance()->GetMapChipBlock(STAGE_NUMBER, ROOM_NUMBER, check);

		//左に壁があった場合、右の壁にリスポーンさせる。
		if (num == 0)
		{
			return Vec2<float>(check.x * 50.0f, check.y * 50.0f);
		}
		else
		{
			return Vec2<float>(check2.x * 50.0f, check2.y * 50.0f);
		}
	}
	//両方とも壁があり、下に空間があるなら
	else if (rightChip != 0 && leftChip != 0 && downWall == 0)
	{
		//扉座標の一マス下にリスポーンさせる
		return Vec2<float>(rightDoor.x * 50.0f, (rightDoor.y + 1) * 50.0f);
	}
	//上下どちらかの扉からリスポーンさせる場合-----------------------


	string result = "次につながるドアが見つかりません。\nRalationファイルを確認するか、担当の大石に連絡をください";
	MessageBox(NULL, KuroFunc::GetWideStrFromStr(result).c_str(), TEXT("ドアが見つかりません"), MB_OK);
	assert(0);
	//失敗
	return Vec2<float>(-1, -1);
}

Vec2<float> Game::GetPlayerPos(const int& STAGE_NUMBER, int* ROOM_NUMBER, const int& DOOR_NUMBER, const SizeData& SIZE_DATA, vector<vector<int>>* MAPCHIP_DATA)
{
	int roomNum = StageMgr::Instance()->GetRelationData(STAGE_NUMBER, *ROOM_NUMBER, DOOR_NUMBER - SIZE_DATA.min);
	*MAPCHIP_DATA = StageMgr::Instance()->GetMapChipData(STAGE_NUMBER, roomNum);

	vector<vector<int>>tmpChipData = *MAPCHIP_DATA;

	Vec2<float> door;
	//次につながるドアを探す
	for (int y = 0; y < tmpChipData.size(); ++y)
	{
		for (int x = 0; x < tmpChipData[y].size(); ++x)
		{
			if (tmpChipData[y][x] == DOOR_NUMBER)
			{
				door = { (float)x,(float)y };
				break;
			}
		}
	}
	Vec2<float> tmp = GetPlayerResponePos(STAGE_NUMBER, roomNum, DOOR_NUMBER, door);
	*ROOM_NUMBER = roomNum;
	return tmp;
}

Game::Game()
{
	mapBlockGraph = TexHandleMgr::LoadGraph("resource/IntoTheAbyss/Block.png");
	//mapBlockGraph = D3D12App::Instance()->GenerateTextureBuffer("resource/IntoTheAbyss/Block.png");

	// 弾管理クラスを初期化。
	BulletMgr::Instance()->Setting();

	// スクロール量を設定。
	const float WIN_WIDTH_HALF = WinApp::Instance()->GetWinCenter().x;
	const float WIN_HEIGHT_HALF = WinApp::Instance()->GetWinCenter().y;
	ScrollMgr::Instance()->scrollAmount = { -WIN_WIDTH_HALF, -WIN_HEIGHT_HALF };
	ScrollMgr::Instance()->honraiScrollAmount = { -WIN_WIDTH_HALF, -WIN_HEIGHT_HALF };

	// マップチップのデータをロード
	mapData = StageMgr::Instance()->GetMapChipData(0, 0);

	movingBlockGraph = TexHandleMgr::LoadGraph("resource/IntoTheAbyss/MovingBlock.png");
	//movingBlockGraph = D3D12App::Instance()->GenerateTextureBuffer("resource/IntoTheAbyss/MovingBlock.png");

	// スクロールマネージャーを初期化。
	ScrollMgr::Instance()->Init(&mapData);

	// シェイク量を設定。
	ShakeMgr::Instance()->Init();

	// 弾パーティクルをセッティング。
	BulletParticleMgr::Instance()->Setting();

	//マップ開始時の場所にスポーンさせる
	for (int y = 0; y < mapData.size(); ++y)
	{
		for (int x = 0; x < mapData[y].size(); ++x)
		{
			if (mapData[y][x] == MAPCHIP_BLOCK_START)
			{
				player.centerPos = { (float)x * 50.0f,(float)y * 50.0f };
				break;
			}
		}
	}

	StageMgr::Instance()->loadGimmickData->GetThowpeData(0, 0);

	//オーラブロック生成
	int auraChipNum = 40;//オーラブロックのチップ番号
	vector<Vec2<float>>usedNum;	//どのマップチップ番号が使われたか
	for (int y = 0; y < mapData.size(); ++y)
	{
		for (int x = 0; x < mapData[y].size(); ++x)
		{
			if (mapData[y][x] == auraChipNum)
			{
				//重複したら処理を飛ばす-----------------------
				bool usedFlag = false;
				for (int i = 0; i < usedNum.size(); ++i)
				{
					if (usedNum[i].x == x && usedNum[i].y == y)
					{
						usedFlag = true;
					}
				}
				//重複したら処理を飛ばす-----------------------

				if (!usedFlag)
				{
#pragma region 上下左右の探索
					vector<Vec2<float>>left, right, up, down;
					usedNum.push_back(Vec2<float>(x, y));
					//上にどれくらい伸びるか
					for (int upY = 1; 1; ++upY)
					{
						int Y = StageMgr::Instance()->GetMapChipBlock(stageNum, roomNum, Vec2<float>(x, y - upY));

						if (Y != auraChipNum || CheckUsedData(usedNum, Vec2<float>(x, y - upY)))
						{
							break;
						}
						else
						{
							usedNum.push_back(Vec2<float>(x, y - upY));
							up.push_back(Vec2<float>(x, y - upY));
						}
					}
					//下にどれくらい伸びるか
					for (int downY = 1; 1; ++downY)
					{
						int Y = StageMgr::Instance()->GetMapChipBlock(stageNum, roomNum, Vec2<float>(x, y + downY));
						if (Y != auraChipNum || CheckUsedData(usedNum, Vec2<float>(x, y + downY)))
						{
							break;
						}
						else
						{
							usedNum.push_back(Vec2<float>(x, y + downY));
							down.push_back(Vec2<float>(x, y + downY));
						}
					}

					//上下に伸ばす事が出来たらここの部分を飛ばす-----------------------
					if (down.size() == 0 && up.size() == 0)
					{
						//左にどれくらい伸びるか
						for (int leftX = 1; 1; ++leftX)
						{
							int X = StageMgr::Instance()->GetMapChipBlock(stageNum, roomNum, Vec2<float>(x - leftX, y));
							if (X != auraChipNum || CheckUsedData(usedNum, Vec2<float>(x - leftX, y)))
							{
								break;
							}
							else
							{
								usedNum.push_back(Vec2<float>(x - leftX, y));
								left.push_back(Vec2<float>(x - leftX, y));
							}
						}
						//右にどれくらい伸びるか
						for (int rightX = 1; 1; ++rightX)
						{
							int X = StageMgr::Instance()->GetMapChipBlock(stageNum, roomNum, Vec2<float>(x + rightX, y));
							if (X != auraChipNum || CheckUsedData(usedNum, Vec2<float>(x + rightX, y)))
							{
								break;
							}
							else
							{
								usedNum.push_back(Vec2<float>(x + rightX, y));
								right.push_back(Vec2<float>(x + rightX, y));
							}
						}
					}
					//上下に伸ばす事が出来たらここの部分を飛ばす-----------------------
#pragma endregion

#pragma region ブロックの追加

					//どの方向に伸びたか-----------------------
					//左右
					Vec2<float> leftUp, rightDown;
					bool sideOrUpDownFlag = false;
					if (left.size() || right.size())
					{
						int leftSize = left.size() - 1;
						int rightSize = right.size() - 1;

						if (leftSize <= 0)
						{
							leftSize = 0;
						}
						if (rightSize <= 0)
						{
							rightSize = 0;
						}

						//上が0なら
						if (left.size() == 0)
						{
							leftUp = { x * 50.0f,y * 50.0f };
						}
						else
						{
							leftUp = { left[leftSize].x * 50.0f,left[leftSize].y * 50.0f };
						}
						//下が0なら
						if (right.size() == 0)
						{
							rightDown = { x * 50.0f,y * 50.0f };
						}
						else
						{
							rightDown = { right[rightSize].x * 50.0f,right[rightSize].y * 50.0f };
						}

						sideOrUpDownFlag = true;
					}
					//上下
					if (up.size() || down.size())
					{
						int upSize = up.size() - 1;
						int downSize = down.size() - 1;

						if (upSize <= 0)
						{
							upSize = 0;
						}
						if (downSize <= 0)
						{
							downSize = 0;
						}

						//上が0なら
						if (up.size() == 0)
						{
							leftUp = { x * 50.0f,y * 50.0f };
						}
						else
						{
							leftUp = { up[upSize].x * 50.0f,up[upSize].y * 50.0f };
						}
						//下が0なら
						if (down.size() == 0)
						{
							rightDown = { x * 50.0f,y * 50.0f };
						}
						else
						{
							rightDown = { down[downSize].x * 50.0f,down[downSize].y * 50.0f };
						}
						sideOrUpDownFlag = false;
					}

					//ブロックサイズに合うように調整する
					leftUp.x -= 25.0f;
					leftUp.y -= 25.0f;
					rightDown.x += 25.0f;
					rightDown.y += 25.0f;
					//どの方向に伸びたか-----------------------

					//伸びた情報をオーラに渡す
					auraBlock.push_back(std::make_unique<AuraBlock>());
					if (sideOrUpDownFlag)
					{
						auraBlock[auraBlock.size() - 1]->Init(leftUp, rightDown, AURA_DIR_LEFTORRIGHT);
					}
					else
					{
						auraBlock[auraBlock.size() - 1]->Init(leftUp, rightDown, AURA_DIR_UPORDOWN);
					}
#pragma endregion
				}
			}
		}
	}

	ViewPort::Instance()->Init(player.centerPos, { 800.0f,500.0f });

	mapChipDrawData = StageMgr::Instance()->GetMapChipDrawBlock(stageNum, roomNum);

	// シャボン玉ブロックを生成。
	bubbleBlock.Generate(player.centerPos);

}

void Game::Update()
{
	ScrollMgr::Instance()->zoom = ViewPort::Instance()->zoomRate;



#pragma region ステージの切り替え
	bool enableToSelectStageFlag = 0 < debugStageData[0];
	bool enableToSelectStageFlag2 = debugStageData[0] < StageMgr::Instance()->GetMaxStageNumber() - 1;
	//マップの切り替え
	//if (Input::isKeyTrigger(KEY_INPUT_UP) && enableToSelectStageFlag2 && nowSelectNum == 0)
	if (UsersInput::Instance()->OnTrigger(DIK_UP) && enableToSelectStageFlag2 && nowSelectNum == 0)
	{
		++debugStageData[0];
	}
	//if (Input::isKeyTrigger(KEY_INPUT_DOWN) && enableToSelectStageFlag && nowSelectNum == 0)
	if (UsersInput::Instance()->OnTrigger(DIK_DOWN) && enableToSelectStageFlag && nowSelectNum == 0)
	{
		--debugStageData[0];
	}


	bool enableToSelectRoomFlag = 0 < debugStageData[1];
	bool enableToSelectRoomFlag2 = debugStageData[1] < StageMgr::Instance()->GetMaxRoomNumber(debugStageData[0]) - 1;
	//部屋の切り替え
	//if (Input::isKeyTrigger(KEY_INPUT_UP) && enableToSelectRoomFlag2 && nowSelectNum == 1)
	if (UsersInput::Instance()->OnTrigger(DIK_UP) && enableToSelectRoomFlag2 && nowSelectNum == 1)
	{
		++debugStageData[1];
	}
	//if (Input::isKeyTrigger(KEY_INPUT_DOWN) && enableToSelectRoomFlag && nowSelectNum == 1)
	if (UsersInput::Instance()->OnTrigger(DIK_DOWN) && enableToSelectRoomFlag && nowSelectNum == 1)
	{
		--debugStageData[1];
	}

	//部屋か番号に切り替え
	//if (Input::isKeyTrigger(KEY_INPUT_LEFT) && 0 < nowSelectNum)
	if (UsersInput::Instance()->OnTrigger(DIK_LEFT) && 0 < nowSelectNum)
	{
		--nowSelectNum;
		debugStageData[1] = 0;
	}
	if (UsersInput::Instance()->OnTrigger(DIK_RIGHT) && nowSelectNum < 1)
	{
		++nowSelectNum;
		debugStageData[1] = 0;
	}

	if (UsersInput::Instance()->OnTrigger(DIK_RETURN))
	{
		stageNum = debugStageData[0];
		roomNum = debugStageData[1];
		mapData = StageMgr::Instance()->GetMapChipData(stageNum, roomNum);

		Vec2<float> door;
		//デバック用のマップチップ番号からスタートする
		for (int y = 0; y < mapData.size(); ++y)
		{
			for (int x = 0; x < mapData[y].size(); ++x)
			{
				if (mapData[y][x] == MAPCHIP_BLOCK_DEBUG_START)
				{
					door = { (float)x,(float)y };
				}
			}
		}
		Vec2<float> tmp = GetPlayerResponePos(stageNum, roomNum, 0, door);
		player.centerPos = tmp;
		ScrollMgr::Instance()->WarpScroll(player.centerPos);
	}
#pragma endregion
	//ImGui::Begin("Stage");
	//ImGui::Text("StageNumber:%d", debugStageData[0]);
	//ImGui::Text("RoomNumber:%d", debugStageData[1]);
	//ImGui::End();







	//ゴールに触れたら次のステージに向かう処理
	{
		Vec2<float> playerBlockPos(player.centerPos), block(50.0f, 50.0f);
		playerBlockPos = playerBlockPos / block;
		//ゴールに触れたら次のステージに向かう
		if (mapData[playerBlockPos.y][playerBlockPos.x] == MAPCHIP_BLOCK_GOAL)
		{
			++stageNum;
			roomNum = 0;

			mapData = StageMgr::Instance()->GetMapChipData(stageNum, roomNum);
			Vec2<float> door;
			//ゴール探索
			for (int y = 0; y < mapData.size(); ++y)
			{
				for (int x = 0; x < mapData[y].size(); ++x)
				{
					if (mapData[y][x] == MAPCHIP_BLOCK_START)
					{
						door = { (float)x,(float)y };
					}
				}
			}
			Vec2<float> tmp = { door.x * 50.0f,door.y * 50.0f };
			player.centerPos = tmp;
			ScrollMgr::Instance()->WarpScroll(player.centerPos);
		}
	}

	/*===== 更新処理 =====*/

	//ドア判定-----------------------
	SizeData chipMemorySize = StageMgr::Instance()->GetMapChipSizeData(MAPCHIP_TYPE_DOOR);
	for (int i = chipMemorySize.min; i < chipMemorySize.max; ++i)
	{
		//奥扉判定-----------------------
		Vec2<float> playerChip((player.centerPos.x + 25.0f) / 50.0f, (player.centerPos.y + 25.0f) / 50.0f);
		Vec2<float> rightCehck(playerChip.x + 1, playerChip.y);
		int rightChip = StageMgr::Instance()->GetMapChipBlock(stageNum, roomNum, rightCehck);
		Vec2<float> leftCehck(playerChip.x - 1, playerChip.y);
		int leftChip = StageMgr::Instance()->GetMapChipBlock(stageNum, roomNum, leftCehck);
		bool zDoorFlag = rightChip == 0 && leftChip == 0;

		int num = 1;
		bool hitDoorFlag;

		//奥扉判定-----------------------
		if (mapData[playerChip.y][playerChip.x] == i && !zDoorFlag)
		{
			player.centerPos = GetPlayerPos(stageNum, &roomNum, i, chipMemorySize, &mapData);
			player.vel = { 0.0f,0.0f };
			ScrollMgr::Instance()->WarpScroll(player.centerPos);
		}
		//else if (mapData[playerChip.y][playerChip.x] == i && zDoorFlag && Input::isJoyBottomTrigger(XINPUT_BUTTON_B))
		else if (mapData[playerChip.y][playerChip.x] == i && zDoorFlag && UsersInput::Instance()->OnTrigger(XBOX_BUTTON::B))
		{
			player.centerPos = GetPlayerPos(stageNum, &roomNum, i, chipMemorySize, &mapData);
			player.vel = { 0.0f,0.0f };
			ScrollMgr::Instance()->WarpScroll(player.centerPos);
		}
	}
	//ドア判定-----------------------



	//ステージ毎の切り替え判定
	if (stageNum != oldStageNum)
	{
		debugStageData[0] = stageNum;
	}
	oldStageNum = stageNum;

	//部屋の初期化
	if (roomNum != oldRoomNum)
	{
		debugStageData[1] = roomNum;
		mapChipDrawData = StageMgr::Instance()->GetMapChipDrawBlock(stageNum, roomNum);

		// ドッスンブロックを生成。
		vector<shared_ptr<ThownpeData>> dossunData;

		// ドッスンブロックデータを取得
		dossunData = GimmickLoader::Instance()->GetThowpeData(stageNum, roomNum);

		const int dossunCount = dossunData.size();

		// ドッスンブロックを初期化。
		dossunBlock.clear();

		// ドッスンを生成。
		for (int index = 0; index < dossunCount; ++index) {

			DossunBlock dossunBuff;
			dossunBuff.Generate(dossunData[index]->startPos, dossunData[index]->endPos, dossunData[index]->size, dossunData[index]->type);

			// データを追加。
			dossunBlock.push_back(dossunBuff);

		}

	}
	oldRoomNum = roomNum;




	// R or Aが押されたらプレイヤーの位置を初期化する。
	//if (Input::isKeyTrigger(KEY_INPUT_R) || Input::isJoyBottomTrigger(XINPUT_BUTTON_A))
	if (UsersInput::Instance()->OnTrigger(DIK_R) || UsersInput::Instance()->OnTrigger(XBOX_BUTTON::A))
	{
		player.centerPos = player.GetGeneratePos();
		ScrollMgr::Instance()->scrollAmount = { -3.0f, 295.0f };
		ScrollMgr::Instance()->honraiScrollAmount = { 0, 295.0f };
	}

	ScrollMgr::Instance()->DetectMapChipForScroll(player.centerPos);


	//ビューポートとオーラの判定
	//if (Input::isKey(KEY_INPUT_F))
	if (UsersInput::Instance()->OnTrigger(DIK_F))
	{
		ScrollMgr::Instance()->StopScroll(ScrollMgr::Instance()->UP);
	}
	//if (Input::isKey(KEY_INPUT_G))
	if (UsersInput::Instance()->OnTrigger(DIK_G))
	{
		ScrollMgr::Instance()->StopScroll(ScrollMgr::Instance()->DOWN);
	}
	//if (Input::isKey(KEY_INPUT_H))
	if (UsersInput::Instance()->OnTrigger(DIK_H))
	{
		ScrollMgr::Instance()->StopScroll(ScrollMgr::Instance()->LEFT);
	}
	//if (Input::isKey(KEY_INPUT_J))
	if (UsersInput::Instance()->OnTrigger(DIK_J))
	{
		ScrollMgr::Instance()->StopScroll(ScrollMgr::Instance()->RIGHT);
	}




	// プレイヤーの更新処理
	player.Update(mapData);

	//if (Input::isKey(KEY_INPUT_RIGHT)) player.centerPos.x += 1.0f;
	if (UsersInput::Instance()->OnTrigger(DIK_RIGHT)) player.centerPos.x += 1.0f;
	//if (Input::isKey(KEY_INPUT_P)) player.centerPos.x += 100.0f;
	if (UsersInput::Instance()->OnTrigger(DIK_P)) player.centerPos.x += 100.0f;
	//if (Input::isKey(KEY_INPUT_LEFT)) player.centerPos.x -= 1.0f;
	if (UsersInput::Instance()->OnTrigger(DIK_LEFT)) player.centerPos.x -= 1.0f;
	//if (Input::isKey(KEY_INPUT_O)) player.centerPos.x -= 100.0f;
	if (UsersInput::Instance()->OnTrigger(DIK_O)) player.centerPos.x -= 100.0f;

	// 敵の更新処理
	enemy.Update();

	// スクロール量の更新処理
	ScrollMgr::Instance()->Update();

	// シェイク量の更新処理
	ShakeMgr::Instance()->Update();

	// 時間停止の短槍の性能テスト用のブロックの更新処理
	testBlock.Update(player.centerPos);

	// 動的ブロックの更新処理
	MovingBlockMgr::Instance()->Update(player.centerPos);

	// 弾パーティクルの更新処理
	BulletParticleMgr::Instance()->Update();

	// ドッスンブロックの更新処理
	const int DOSSUN_COUNT = dossunBlock.size();
	for (int index = 0; index < DOSSUN_COUNT; ++index) {
		dossunBlock[index].Update();
	}

	ViewPort::Instance()->Update(player.centerPos);
	for (int i = 0; i < auraBlock.size(); ++i)
	{
		auraBlock[i]->Update();
	}

	// シャボン玉ブロックの更新処理
	bubbleBlock.Update();

	//if (Input::isKey(KEY_INPUT_UP)) ViewPort::Instance()->zoomRate += 0.01f;
	//if (UsersInput::Instance()->OnTrigger(DIK_UP))  ViewPort::Instance()->zoomRate += 0.01f;
	//if (Input::isKey(KEY_INPUT_DOWN)) ViewPort::Instance()->zoomRate -= 0.01f;
	//if (UsersInput::Instance()->OnTrigger(DIK_DOWN)) ViewPort::Instance()->zoomRate -= 0.01f;

	/*===== 当たり判定 =====*/

	// プレイヤーの当たり判定
	player.CheckHit(mapData, testBlock);

	// 動的ブロックの当たり判定
	MovingBlockMgr::Instance()->CheckHit(mapData);

	// 弾とマップチップの当たり判定
	BulletMgr::Instance()->CheckHit(mapData);

	// ビューポートをプレイヤー基準で移動させる。
	ViewPort::Instance()->SetPlayerPosX(player.centerPos.x);
	ViewPort::Instance()->SetPlayerPosY(player.centerPos.y);

	// オーラブロックのデータとビューポートの判定を行う。
	ViewPort::Instance()->SavePrevFlamePos();



	// 弾とビューポートの当たり判定
	for (int i = 0; i < BulletMgr::Instance()->bullets.size(); ++i)
	{
		int hitArea = ViewPortCollider::Instance()->CheckHitRectanglePoint
		(
			ViewPort::Instance()->centralPos,
			ViewPort::Instance()->windowSize.y / 2.0f + ViewPort::Instance()->addLineValue[ViewPort::Instance()->UP],
			ViewPort::Instance()->windowSize.x / 2.0f + ViewPort::Instance()->addLineValue[ViewPort::Instance()->RIGHT],
			ViewPort::Instance()->windowSize.y / 2.0f + ViewPort::Instance()->addLineValue[ViewPort::Instance()->DOWN],
			ViewPort::Instance()->windowSize.x / 2.0f + ViewPort::Instance()->addLineValue[ViewPort::Instance()->LEFT],
			BulletMgr::Instance()->GetBullet(i)->pos,
			BulletMgr::Instance()->GetBullet(i)->MAX_RADIUS
		);

		bool hitFlag =
			BulletMgr::Instance()->GetBullet(i)->GetIsActive() &&
			hitArea != ViewPortCollider::Instance()->HIT_AREA_NONE;

		if (hitFlag)
		{
			//ViewPort::Instance()->MoveLine(hitArea, 0.01f);

			// 弾パーティクルを生成する。
			BulletParticleMgr::Instance()->Generate(BulletMgr::Instance()->GetBullet(i)->pos, BulletMgr::Instance()->GetBullet(i)->forwardVec);
			BulletParticleMgr::Instance()->Generate(BulletMgr::Instance()->GetBullet(i)->pos, BulletMgr::Instance()->GetBullet(i)->forwardVec);

			BulletMgr::Instance()->GetBullet(i)->Init();
		}
	}

	// オーラとビューポートの当たり判定
	{
		// オーラブロックのデータとビューポートの判定を行う。
		vector<ViewPortAuraReturnData> buff;
		// 上方向
		buff = ViewPortCollider::Instance()->CheckHitRectangleAura(ViewPort::Instance()->pointData[ViewPort::LEFT_UP], ViewPort::Instance()->pointData[ViewPort::RIGHT_UP], CHECK_HIT_TOP, auraBlock);

		// 押し戻す
		if (buff.size() > 0 && !ViewPort::Instance()->noHitFlags[ViewPort::Instance()->UP])
		{

			ViewPortAuraReturnData biggestBuff = {};
			// 値が一番大きい押し戻し量を求める。
			for (int index = 0; index < buff.size(); ++index) {

				if (fabs(buff[index].pushBackAmount) < fabs(biggestBuff.pushBackAmount)) continue;

				biggestBuff = buff[index];

			}

			// ホールドされていない状態だったら-方向に押し戻されるのはおかしいので、押し戻し量を打ち消す。
			if (!biggestBuff.isHold && biggestBuff.pushBackAmount < 0)
			{
				biggestBuff.pushBackAmount = 0;
			}
			// ホールドされていなる状態だったら+方向に押し戻されるのはおかしいので、押し戻し量を打ち消す。
			else if (biggestBuff.isHold && biggestBuff.pushBackAmount > 0) {

				biggestBuff.pushBackAmount = 0;

			}

			// 上側に当たった判定を保存する。
			ViewPort::Instance()->isHitTop = true;

			// ホールド状態だったら
			if (biggestBuff.isHold) {


				ViewPort::Instance()->PushBackAuraHoldUp(biggestBuff.pushBackAmount);

			}
			else {

				// 中心座標を押し戻す。
				ViewPort::Instance()->centralPos.y += biggestBuff.pushBackAmount;

				// ビューポートの四点を押し戻す。
				ViewPort::Instance()->PushBackAura(Vec2<float>(0, biggestBuff.pushBackAmount));

			}

			if (biggestBuff.isHold) {
				ViewPort::Instance()->holdFlags[ViewPort::Instance()->UP] = true;
				//ViewPort::Instance()->PushBackLine(ViewPort::Instance()->LEFT, biggestBuff.pushBackAmount);
			}

			// スクロールを止める。
			ScrollMgr::Instance()->StopScroll(0);
			++countStopNum;

		}
		else
		{
			ViewPort::Instance()->holdFlags[ViewPort::Instance()->UP] = false;
		}


		// 右方向
		buff = ViewPortCollider::Instance()->CheckHitRectangleAura(ViewPort::Instance()->pointData[ViewPort::RIGHT_UP], ViewPort::Instance()->pointData[ViewPort::RIGHT_DOWN], CHECK_HIT_RIGHT, auraBlock);

		// 押し戻す
		if (buff.size() > 0 && !ViewPort::Instance()->noHitFlags[ViewPort::Instance()->RIGHT])
		{

			ViewPortAuraReturnData biggestBuff = {};
			// 値が一番大きい押し戻し量を求める。
			for (int index = 0; index < buff.size(); ++index) {

				if (fabs(buff[index].pushBackAmount) < fabs(biggestBuff.pushBackAmount)) continue;

				biggestBuff = buff[index];

			}

			// ホールドされていない状態だったら-方向に押し戻されるのはおかしいので、押し戻し量を打ち消す。
			if (!biggestBuff.isHold && biggestBuff.pushBackAmount > 0)
			{
				biggestBuff.pushBackAmount = 0;
			}
			// ホールドされていなる状態だったら+方向に押し戻されるのはおかしいので、押し戻し量を打ち消す。
			else if (biggestBuff.isHold && biggestBuff.pushBackAmount < 0) {

				biggestBuff.pushBackAmount = 0;

			}

			// 右側に当たった判定を保存する。
			ViewPort::Instance()->isHitRight = true;

			// ホールド状態だったら
			if (biggestBuff.isHold) {


				ViewPort::Instance()->PushBackAuraHoldRight(biggestBuff.pushBackAmount);

			}
			else {

				// 中心座標を押し戻す。
				ViewPort::Instance()->centralPos.x += biggestBuff.pushBackAmount;

				// ビューポートの四点を押し戻す。
				ViewPort::Instance()->PushBackAura(Vec2<float>(biggestBuff.pushBackAmount, 0));

			}

			if (biggestBuff.isHold) {
				ViewPort::Instance()->holdFlags[ViewPort::Instance()->RIGHT] = true;
				//ViewPort::Instance()->PushBackLine(ViewPort::Instance()->LEFT, biggestBuff.pushBackAmount);
			}

		}
		else
		{
			ViewPort::Instance()->holdFlags[ViewPort::Instance()->RIGHT] = false;
			if (countStopNum % 2 != 0)
			{
				ScrollMgr::Instance()->StopScroll(0);
				countHitNum = 0;
				countStopNum = 0;
			}
			++countHitNum;

			if (2 <= countHitNum)
			{
				countStopNum = 0;
			}

		}

		// 下方向
		buff = ViewPortCollider::Instance()->CheckHitRectangleAura(ViewPort::Instance()->pointData[ViewPort::LEFT_DOWN], ViewPort::Instance()->pointData[ViewPort::RIGHT_DOWN], CHECK_HIT_BOTTOM, auraBlock);

		// 押し戻す
		if (buff.size() > 0 && !ViewPort::Instance()->noHitFlags[ViewPort::Instance()->DOWN])
		{

			ViewPortAuraReturnData biggestBuff = {};
			// 値が一番大きい押し戻し量を求める。
			for (int index = 0; index < buff.size(); ++index) {

				if (fabs(buff[index].pushBackAmount) < fabs(biggestBuff.pushBackAmount)) continue;

				biggestBuff = buff[index];

			}

			// ホールドされていない状態だったら-方向に押し戻されるのはおかしいので、押し戻し量を打ち消す。
			if (!biggestBuff.isHold && biggestBuff.pushBackAmount > 0)
			{
				biggestBuff.pushBackAmount = 0;
			}
			// ホールドされていなる状態だったら+方向に押し戻されるのはおかしいので、押し戻し量を打ち消す。
			else if (biggestBuff.isHold && biggestBuff.pushBackAmount < 0) {

				biggestBuff.pushBackAmount = 0;

			}

			// 上側に当たった判定を保存する。
			ViewPort::Instance()->isHitBottom = true;

			// ホールド状態だったら
			if (biggestBuff.isHold) {


				ViewPort::Instance()->PushBackAuraHoldDown(biggestBuff.pushBackAmount);

			}
			else {

				// 中心座標を押し戻す。
				ViewPort::Instance()->centralPos.y += biggestBuff.pushBackAmount;

				// ビューポートの四点を押し戻す。
				ViewPort::Instance()->PushBackAura(Vec2<float>(0, biggestBuff.pushBackAmount));

			}

			if (biggestBuff.isHold) {
				ViewPort::Instance()->holdFlags[ViewPort::Instance()->DOWN] = true;
				//ViewPort::Instance()->PushBackLine(ViewPort::Instance()->LEFT, biggestBuff.pushBackAmount);
			}

			// スクロールを止める。
			ScrollMgr::Instance()->StopScroll(0);
			++countStopNum;
		}
		else
		{
			ViewPort::Instance()->holdFlags[ViewPort::Instance()->DOWN] = false;
		}


		// 左方向
		buff = ViewPortCollider::Instance()->CheckHitRectangleAura(ViewPort::Instance()->pointData[ViewPort::LEFT_UP], ViewPort::Instance()->pointData[ViewPort::LEFT_DOWN], CHECK_HIT_LEFT, auraBlock);



		// 押し戻す
		if (buff.size() > 0 && !ViewPort::Instance()->noHitFlags[ViewPort::Instance()->LEFT])
		{

			ViewPortAuraReturnData biggestBuff = {};
			// 値が一番大きい押し戻し量を求める。
			for (int index = 0; index < buff.size(); ++index) {

				if (fabs(buff[index].pushBackAmount) < fabs(biggestBuff.pushBackAmount)) continue;

				biggestBuff = buff[index];

			}

			// ホールドされていない状態だったら-方向に押し戻されるのはおかしいので、押し戻し量を打ち消す。
			if (!biggestBuff.isHold && biggestBuff.pushBackAmount < 0)
			{
				biggestBuff.pushBackAmount = 0;
			}
			// ホールドされていなる状態だったら+方向に押し戻されるのはおかしいので、押し戻し量を打ち消す。
			else if (biggestBuff.isHold && biggestBuff.pushBackAmount > 0) {

				biggestBuff.pushBackAmount = 0;

			}

			//biggestBuff.pushBackAmount /= 2.0f;

			// 左側に当たった判定を保存する。
			ViewPort::Instance()->isHitLeft = true;

			// ホールド状態だったら
			if (biggestBuff.isHold) {


				ViewPort::Instance()->PushBackAuraHoldLeft(biggestBuff.pushBackAmount);

			}
			else {

				// 中心座標を押し戻す。
				ViewPort::Instance()->centralPos.x += biggestBuff.pushBackAmount;

				// ビューポートの四点を押し戻す。
				ViewPort::Instance()->PushBackAura(Vec2<float>(biggestBuff.pushBackAmount, 0));

			}

			if (biggestBuff.isHold) {
				ViewPort::Instance()->holdFlags[ViewPort::Instance()->LEFT] = true;
				//ViewPort::Instance()->PushBackLine(ViewPort::Instance()->LEFT, biggestBuff.pushBackAmount);
			}

			// スクロールを止める。
			ScrollMgr::Instance()->StopScroll(0);
			++countStopNum;
		}
		else
		{
			ViewPort::Instance()->holdFlags[ViewPort::Instance()->LEFT] = false;
		}


	}
	ViewPort::Instance()->playerPos = player.centerPos;

	// ドッスンブロックの当たり判定
	for (int index = 0; index < DOSSUN_COUNT; ++index) {
		dossunBlock[index].CheckHit(player, mapData);
	}


}

void Game::Draw()
{
	/*===== 描画処理 =====*/

	DrawMapChip(mapData, mapChipDrawData, mapBlockGraph, 0, roomNum);

	testBlock.Draw(mapBlockGraph);

	MovingBlockMgr::Instance()->Draw(movingBlockGraph);

	// 弾パーティクルの描画処理
	BulletParticleMgr::Instance()->Draw();

	// ドッスンブロックの描画処理
	const int DOSSUN_COUNT = dossunBlock.size();
	for (int index = 0; index < DOSSUN_COUNT; ++index) {
		dossunBlock[index].Draw();
	}

	// シャボン玉ブロックの更新処理
	bubbleBlock.Draw();

	player.Draw();

	ViewPort::Instance()->Draw();


	for (int i = 0; i < auraBlock.size(); ++i)
	{
		auraBlock[i]->Draw();
	}
}
