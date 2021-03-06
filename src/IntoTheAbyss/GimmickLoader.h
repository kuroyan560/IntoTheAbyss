#pragma once
#include"Vec.h"
#include "Singleton.h"
#include<vector>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include<string>
#include<array>
#include<memory>
#include<windows.h>

enum E_GIMMICK
{
	GIMMICK_NONE = -1,
	GIMMICK_DOSSN_OFF,
	GIMMICK_DOSSN_ON_LOW,
	GIMMICK_DOSSN_ON_HIGH,
};

struct ThownpeData
{
	Vec2<float> startPos;		//開始位置
	Vec2<float> endPos;		//終了位置
	Vec2<float> size;			//画像の大きさ
	E_GIMMICK type;		//ギミックの種類

	ThownpeData(const Vec2<float> &SIZE = Vec2<float>(-1.0f, -1.0f), const E_GIMMICK &TYPE = GIMMICK_NONE) :size(SIZE), type(TYPE), startPos(Vec2<float>(-1.0f, -1.0f)), endPos(Vec2<float>(-1.0f, -1.0f))
	{
	};
};

struct BubbleData
{
	Vec2<float> pos;	//生成座標
	BubbleData(const Vec2<float> &POS = Vec2<float>(-1.0f, -1.0f)) :pos(POS)
	{
	};
};

struct ThownData
{
	Vec2<float> adjValue;	//当たり判定調整用の値
	ThownData(const Vec2<float> &VALUE = Vec2<float>(-1.0f, -1.0f)) :adjValue(VALUE)
	{
	};
};

class GimmickLoader :public Singleton<GimmickLoader>
{
public:
	GimmickLoader();
	void LoadData(const int &STAGE_NUM, const int &ROOM_NUM, const std::string &FILE_PASS);
	void LoadData(const std::string &FILE_PASS);

	std::vector< std::shared_ptr<ThownpeData>> GetThowpeData(const int &STAGE_NUM, const int &ROOM_NUM);
	std::vector< std::shared_ptr<BubbleData>> GetBubbleData(const int &STAGE_NUM, const int &ROOM_NUM);


	void SetThwompStartPos(const int &STAGE_NUM, const int &ROOM_NUM, const int &GIMMICK_NUMBER, const Vec2<float> &POS);
	void SetThwompEndPos(const int &STAGE_NUM, const int &ROOM_NUM, const int &GIMMICK_NUMBER, const Vec2<float> &POS);
	void PushBubbleData(const int &STAGE_NUM, const int &ROOM_NUM, const int &GIMMICK_NUMBER, const Vec2<float> &POS);

	const bool &CanLoadData(const int &STAGE_NUM, const int &ROOM_NUM, const int &GIMMICK_NUMBER);

	void ErrorCheck();

	std::shared_ptr<ThownData> thownData;
private:
	std::vector<std::vector<std::vector<std::shared_ptr<ThownpeData>>>> allThowmpeData;//ドッスンに関する情報
	std::vector<std::vector<std::vector<std::shared_ptr<BubbleData>>>> allBubbleData;//ドッスンに関する情報


	enum GimmickName
	{
		GIMMCK_NAME_NONE = -1,
		GIMMCK_NAME_THOWMPE,
		GIMMCK_NAME_THOWN,
		GIMMCK_NAME_MAX,
	};

	enum GimmickCommon
	{
		GIMMCK_COMMON_DATA_NAME,
		GIMMCK_COMMON_DATA_MAX
	};

	enum GimmickThompeData
	{
		GIMMCK_THOWMPE_DATA_TYPE,
		GIMMCK_THOWMPE_DATA_SIZE,
		GIMMCK_THOWMPE_DATA_MAX
	};

	enum GimmickThowmpeType
	{
		GIMMCK_THOWMPE_TYPE_OFF,
		GIMMCK_THOWMPE_TYPE_ON_LOW,
		GIMMCK_THOWMPE_TYPE_ON_HIGH,
		GIMMCK_THOWMPE_TYPE_MAX,
	};

	std::array<std::string, GIMMCK_NAME_MAX>gimmickName;					//ギミック名を読み込む際に無効な文字が入ってないか確認する為の比較対象
	std::array<std::string, GIMMCK_COMMON_DATA_MAX>gimmickCommonName;		//Key値を読み込む際に無効な文字が入ってないか確認する為の比較対象。全てのギミック共通に使うKey値が格納される
	std::array<std::string, GIMMCK_THOWMPE_DATA_MAX>gimmickThowmpeDataName;	//Key値を読み込む際に無効な文字が入ってないか確認する為の比較対象。特定のギミックに使うKey値が格納される
	std::array<std::string, GIMMCK_THOWMPE_TYPE_MAX>gimmickThowmpeType;		//ギミックの項目を読み込む際に無効な文字が入ってないか確認する為の比較対象

	GimmickName CheckName(const std::string &KEY_NAME, std::istringstream *LINE_STREAM);

	void LoadThowmpeData(const std::string &KEY, std::istringstream *LINE_STREAM, std::shared_ptr<ThownpeData> THOWNPE_DATA);
	void LoadThownData(const std::string &KEY, std::istringstream *LINE_STREAM, std::shared_ptr<ThownData> THOWN_DATA);

	void AddErrorStirng(const int &STAGE_NUM, const int ROOM_NUM, const std::string &NAME);

	//エラー文表示
	std::vector<std::vector<std::vector<std::string>>> errorStrings;
	std::vector<int>errorCountData;
	int oldStageNum;
	int oldRoomNum;
};