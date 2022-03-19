#pragma once
#include"Vec.h"
#include<vector>
#include "StageMgr.h"
using namespace std;
#include "Player.h"
#include "Enemy.h"
#include"AuraBlock.h"
#include "DossunBlock.h"
#include "Bubble.h"
#include"GimmickLoader.h"

#include<memory>
class TextureBuffer;

#include"DrawMap.h"

//元ソリューションのmain処理をまとめたもの
class Game
{
	bool CheckUsedData(vector<Vec2<float>> DATA, Vec2<float> DATA2);
	void DrawMapChip(const vector<vector<int>>& mapChipData, vector<vector<MapChipDrawData>>& mapChipDrawData, const int& mapBlockGraph, const int& stageNum, const int& roomNum);
	Vec2<float> GetPlayerResponePos(const int& STAGE_NUMBER, const int& ROOM_NUMBER, const int& DOOR_NUMBER, Vec2<float> DOOR_MAPCHIP_POS);
	Vec2<float> GetPlayerPos(const int& STAGE_NUMBER, int* ROOM_NUMBER, const int& DOOR_NUMBER, const SizeData& SIZE_DATA, vector<vector<int>>* MAPCHIP_DATA);

	int mapBlockGraph;
	// 動的ブロックの画像。
	int movingBlockGraph;

	// プレイヤー
	Player player;

	// 敵
	Enemy enemy;

	// 時間停止の短槍の挙動をテストするようのブロック。
	TimeStopTestBlock testBlock;

	// マップチップのデータ
	vector<vector<int>> mapData;

	vector<DossunBlock> dossunBlock;
	vector<Bubble> bubbleBlock;

	int nowSelectNum = 0;


	int oldStageNum = -1;
	int oldRoomNum = -1;

	vector<std::unique_ptr<AuraBlock>> auraBlock;

	vector<vector<MapChipDrawData>> mapChipDrawData;

	int countStopNum = 0;
	int countHitNum = 0;

	std::vector<DrawMap>drawMap;

public:
	int stageNum = 0;//ステージ番号
	int roomNum = 0; //部屋番号

	array<int, 2> debugStageData = { 0,0 };//デバック用のステージと部屋番号


	Game();
	void Update();
	void Draw();
};

