#pragma once
#include"Vec.h"
#include<memory>
#include<vector>
#include"StagingInterFace.h"
#include"AreaCollider.h"
#include"../IntoTheAbyss/BulletCollision.h"
#include<array>
#include "Intersected.h"

static const enum WHICH_TEAM { LEFT_TEAM, RIGHT_TEAM };

class CharacterInterFace
{
private:
	void SwingUpdate();
	void Crash(const Vec2<float>& MyVec);
	void CrashUpdate();

	//画面挟みクラッシュ判定
	const int STACK_WINDOW_TIMER = 120.0f;// ウィンドウに挟まったタイマー
	int stackWindowTimer;		// ウィンドウに挟まったタイマー

	//陣地との判定
	Square areaHitBox;

	//弾との当たり判定
	SphereCollision bulletHitSphere;

	const int AFTER_SWING_DELAY = 15;
	Vec2<float> swingInertiaVec;// 振り回しの慣性の移動方向
	float swingInertia;			// 振り回しの慣性
	int afterSwingDelay;		// 振り回しのあとにボスを少し動けない状態にするためのタイマー

	//演出などの動きの関係で動きを止める
	bool canMove;

	//左か右か
	WHICH_TEAM team;

protected:

	bool nowSwing;
	Vec2<float> swingStartVec;
	Vec2<float> swingEndVec;
	float swingEaseRate;


protected:
	static const enum HIT_DIR { LEFT, RIGHT, TOP, BOTTOM, HIT_DIR_NUM };
	std::array<int, HIT_DIR_NUM>mapChipHit;
	bool stackMapChip;

	//試合開始時に呼び出される
	CharacterInterFace(const WHICH_TEAM& Team, const Vec2<float>& HonraiSize) :team(Team), size(HonraiSize)
	{
		areaHitBox.center = &pos;
		areaHitBox.size = size;
		bulletHitSphere.center = &pos;
		bulletHitSphere.radius = size.x;
	}

	std::weak_ptr<CharacterInterFace>partner;
	StagingInterFace stagingDevice;
	Vec2<float>size;	//サイズ

	//[キャラごとに違う関数]
	virtual void OnInit() = 0;
	virtual void OnUpdate(const std::vector<std::vector<int>>& MapData) = 0;
	virtual void OnUpdateNoRelatedSwing() = 0;	//スウィング中でも通る処理
	virtual void OnDraw() = 0;
	virtual void OnCheckHit(const std::vector<std::vector<int>>& MapData, const Vec2<float>& LineCenterPos) = 0;
	virtual void OnHitMapChip(const HIT_DIR& Dir) = 0;

	//[共通関数]
	//振り回し
	void SwingPartner();
	//ゲッタ類
	const Vec2<float>& GetPartnerPos()
	{
		return partner.lock()->pos;
	}
	//振り回しによる慣性
	Vec2<float>GetSwingInertia() { return swingInertiaVec * swingInertia; }
	//振り回し直後の硬直中か
	bool GetSwingRigor() { return 0 < afterSwingDelay; }
	//左チームか右チームか
	const WHICH_TEAM& GetWhichTeam() { return team; }

	// 当たり判定情報保存。
	void SaveHitInfo(bool& isHitTop, bool& isHitBottom, bool& isHitLeft, bool& isHitRight, const INTERSECTED_LINE& intersectedLine);

public:
	//登場演出が完了したか
	virtual bool Appear() = 0;

public:
	static const int LINE_LENGTH = 150;
	float addLineLength;	//紐
	Vec2<float> pos;			// 座標
	Vec2<float>vel;
	Vec2<float> prevPos;		// 前フレームの座標

	void RegisterSetPartner(const std::shared_ptr<CharacterInterFace>Partner) { partner = Partner; }
	void Init(const Vec2<float>& GeneratePos);	//ラウンド開始時に呼び出される
	void Update(const std::vector<std::vector<int>>& MapData, const Vec2<float>& LineCenterPos);
	void Draw();
	//当たり判定
	void CheckHit(const std::vector<std::vector<int>>& MapData, const Vec2<float>& LineCenterPos);

	const Square& GetAreaHitBox() { return areaHitBox; }
	const SphereCollision& GetBulletHitSphere() { return bulletHitSphere; }

	const int& GetStackWinTimer() { return stackWindowTimer; }
	const bool& GetNowSwing() { return nowSwing; }
	const bool& GetNowStuckWin() { return 0 < stackWindowTimer; }
	const bool& GetCanMove() { return canMove; }
	const bool& GetStackFlag() { return stackMapChip; }

	void SetCanMove(const bool& Flg) { canMove = Flg; }

	inline void FinishSwing() { nowSwing = false; }
};

