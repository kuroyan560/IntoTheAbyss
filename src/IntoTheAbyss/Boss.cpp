#include "Boss.h"
#include "ShakeMgr.h"
#include "ScrollMgr.h"
#include "DrawFunc.h"
#include "MapChipCollider.h"
#include "WinApp.h"
#include"UsersInput.h"
#include"SuperiorityGauge.h"

#include"TexHandleMgr.h"
#include"BossPatternNormalMove.h"
#include"BossPatternAttack.h"

#include"BossBulletManager.h"

Boss::Boss()
{

	pos = {};
	scale = {};
	vel = {};

	graphHandle[FRONT] = TexHandleMgr::LoadGraph("resource/ChainCombat/enemy.png");
	graphHandle[BACK] = TexHandleMgr::LoadGraph("resource/ChainCombat/enemy_back.png");

	areaHitBox.center = &pos;


	bossPattern[0] = std::make_unique<BossPatternNormalMove>();
	bossPattern[1] = std::make_unique<BossPatternAttack>();


	//パターンに渡すデータの初期化
	patternData.bossPos = &vel;
	patternData.stuckWindowTimer = &stuckWindowTimer;

	bossPatternNow = BOSS_PATTERN_NORMALMOVE;
	patternTimer = 0;

}

void Boss::Init()
{

	pos = {};
	scale = {};
	vel = {};

}

void Boss::Generate(const Vec2<float> &generatePos)
{

	/*===== 生成処理 =====*/

	pos = generatePos;
	scale = SCALE;
	vel = { OFFSET_VEL,0 };
	prevIntersectedLine = INTERSECTED_NONE;
	stuckWindowTimer = 0;
	areaHitBox.size = scale;
}

#include"Camera.h"
void Boss::Update()
{

	/*===== 更新処理 =====*/

	// 前フレームの座標を保存
	prevPos = pos;

	for (int i = 0; i < patternData.bulltData.size(); ++i)
	{
		if (patternData.bulltData[i].initFlag)
		{
			patternData.bulltData[i].Reset();
		}
	}


	//ボスのAI-----------------------
	++patternTimer;
	if (120 <= patternTimer)
	{
		if (atackModeFlag)
		{
			bossPatternNow = BOSS_PATTERN_ATTACK;
		}
		else
		{
			bossPatternNow = BOSS_PATTERN_NORMALMOVE;
		}
		atackModeFlag = !atackModeFlag;
		patternTimer = 0;
	}
	//ボスのAI-----------------------


	

	//ボスの挙動
	if (bossPatternNow != oldBossPattern)
	{
		bossPattern[bossPatternNow]->Init();
	}
	bossPattern[bossPatternNow]->Update(&patternData);
	oldBossPattern = bossPatternNow;

	//ボスの弾
	for (int i = 0; i < patternData.bulltData.size(); ++i)
	{
		if (patternData.bulltData[i].initFlag)
		{
			BossBulletManager::Instance()->Generate(pos, patternData.bulltData[i].dir, patternData.bulltData[i].speed);
		}
	}
	BossBulletManager::Instance()->Update();

}

void Boss::Draw()
{
	/*===== 描画処理 =====*/
	BossBulletManager::Instance()->Draw();
	//DrawFunc::DrawBox2D(pos - scale - scrollShakeAmount, pos + scale - scrollShakeAmount, Color(230, 38, 113, 255), DXGI_FORMAT_R8G8B8A8_UNORM, true);
	DIR dir = FRONT;
	if (vel.y < 0)dir = BACK;
	DrawFunc::DrawExtendGraph2D(ScrollMgr::Instance()->Affect(pos - scale), ScrollMgr::Instance()->Affect(pos + scale), TexHandleMgr::GetTexBuffer(graphHandle[dir]), AlphaBlendMode_Trans);
}

void Boss::CheckHit(const vector<vector<int>>& mapData, bool& isHitMapChip, const Vec2<float>& playerPos, const Vec2<float>& lineCenterPos)
{

	/*===== 当たり判定 =====*/

	// ボスがマップチップと当たっているかのフラグ
	bool isHitMapChipBoss = false;
	INTERSECTED_LINE intersectedBuff = {};

	// マップチップ目線でどこに当たったか
	bool isHitTop = false;
	bool isHitRight = false;
	bool isHitLeft = false;
	bool isHitBottom = false;

	bool onGround = false;
	// 移動量を元にしたマップチップとの当たり判定を行う。
	Vec2<float> moveDir = pos - prevPos;
	moveDir.Normalize();
	INTERSECTED_LINE intersectedLine = MapChipCollider::Instance()->CheckHitMapChipBasedOnTheVel(pos, prevPos, moveDir * scale, scale, onGround, mapData, false);
	isHitTop = intersectedLine == INTERSECTED_TOP;
	isHitRight = intersectedLine == INTERSECTED_RIGHT;
	isHitLeft = intersectedLine == INTERSECTED_LEFT;
	isHitBottom = intersectedLine == INTERSECTED_BOTTOM;

	// 当たり判定を保存。
	isHitMapChipBoss = intersectedLine != INTERSECTED_NONE;

	// スケールを元にしたマップチップとの当たり判定を行う。
	float offset = 1.0f;
	intersectedLine = MapChipCollider::Instance()->CheckHitMapChipBasedOnTheScale(pos, scale, mapData, INTERSECTED_TOP);
	if (!isHitMapChipBoss && intersectedLine != INTERSECTED_NONE)isHitMapChipBoss = true;
	if (!isHitTop && intersectedLine == INTERSECTED_TOP) isHitTop = true;
	intersectedLine = MapChipCollider::Instance()->CheckHitMapChipBasedOnTheScale(pos, scale, mapData, INTERSECTED_TOP);
	if (!isHitMapChipBoss && intersectedLine != INTERSECTED_NONE)isHitMapChipBoss = true;
	if (!isHitTop && intersectedLine == INTERSECTED_TOP) isHitTop = true;

	intersectedLine = MapChipCollider::Instance()->CheckHitMapChipBasedOnTheScale(pos, scale, mapData, INTERSECTED_BOTTOM);
	if (!isHitMapChipBoss && intersectedLine != INTERSECTED_NONE)isHitMapChipBoss = true;
	if (!isHitBottom && intersectedLine == INTERSECTED_BOTTOM) isHitBottom = true;
	intersectedLine = MapChipCollider::Instance()->CheckHitMapChipBasedOnTheScale(pos, scale, mapData, INTERSECTED_BOTTOM);
	if (!isHitMapChipBoss && intersectedLine != INTERSECTED_NONE)isHitMapChipBoss = true;
	if (!isHitBottom && intersectedLine == INTERSECTED_BOTTOM) isHitBottom = true;

	intersectedLine = MapChipCollider::Instance()->CheckHitMapChipBasedOnTheScale(pos, scale, mapData, INTERSECTED_RIGHT);
	if (!isHitMapChipBoss && intersectedLine != INTERSECTED_NONE)isHitMapChipBoss = true;
	if (!isHitRight && intersectedLine == INTERSECTED_RIGHT) isHitRight = true;
	intersectedLine = MapChipCollider::Instance()->CheckHitMapChipBasedOnTheScale(pos, scale, mapData, INTERSECTED_RIGHT);
	if (!isHitMapChipBoss && intersectedLine != INTERSECTED_NONE)isHitMapChipBoss = true;
	if (!isHitRight && intersectedLine == INTERSECTED_RIGHT) isHitRight = true;

	intersectedLine = MapChipCollider::Instance()->CheckHitMapChipBasedOnTheScale(pos, scale, mapData, INTERSECTED_LEFT);
	if (!isHitMapChipBoss && intersectedLine != INTERSECTED_NONE)isHitMapChipBoss = true;
	if (!isHitLeft && intersectedLine == INTERSECTED_LEFT) isHitLeft = true;
	intersectedLine = MapChipCollider::Instance()->CheckHitMapChipBasedOnTheScale(pos, scale, mapData, INTERSECTED_LEFT);
	if (!isHitMapChipBoss && intersectedLine != INTERSECTED_NONE)isHitMapChipBoss = true;
	if (!isHitLeft && intersectedLine == INTERSECTED_LEFT) isHitLeft = true;


	// マップチップと当たっていたら
	if (isHitMapChipBoss) {

		// マップチップと当たっている場所によって、引っかかっているかどうかを調べる。

		// プレイヤーとの角度
		float playerAngle = atan2(playerPos.y - pos.y, playerPos.x - pos.x);
		playerAngle = fabs(playerAngle);

		if (isHitTop) {

			// マップチップの上にあたっていたということは、プレイヤーが下方向にいればOK！
			// 下方向の具体的な値は
			const float MIN_ANGLE = 0.785398f;
			const float MAX_ANGLE = 2.35619f;

			// 角度がこの値の間だったら
			if (MIN_ANGLE <= playerAngle && playerAngle <= MAX_ANGLE) {

				// 引っかかっている。
				isHitMapChip = true;
				intersectedBuff = INTERSECTED_BOTTOM;

			}

		}

		if (isHitBottom) {

			// マップチップの下にあたっていたということは、プレイヤーが上方向にいればOK！
			// 上方向の具体的な値は
			const float MIN_ANGLE = 3.92699f;
			const float MAX_ANGLE = 5.49779f;

			// 角度がこの値の間だったら
			if (MIN_ANGLE <= playerAngle && playerAngle <= MAX_ANGLE) {

				// 引っかかっている。
				isHitMapChip = true;
				intersectedBuff = INTERSECTED_TOP;

			}

		}

		if (isHitRight) {

			// マップチップの右にあたっていたということは、プレイヤーが左方向にいればOK！
			// 左方向の具体的な値は
			const float MIN_ANGLE = 2.35619f;
			const float MAX_ANGLE = 3.92699f;

			// 角度がこの値の間だったら
			if (MIN_ANGLE <= playerAngle && playerAngle <= MAX_ANGLE) {

				// 引っかかっている。
				isHitMapChip = true;
				intersectedBuff = INTERSECTED_RIGHT;

			}

		}

		if (isHitLeft) {

			// マップチップの左にあたっていたということは、プレイヤーが右方向にいればOK！
			// 右方向の具体的な値は
			const float MIN_ANGLE = 0.785398f;
			const float MAX_ANGLE = 5.49779f;

			// 角度がこの値の間だったら
			if (MAX_ANGLE <= playerAngle && playerAngle <= MIN_ANGLE) {

				// 引っかかっている。
				isHitMapChip = true;
				intersectedBuff = INTERSECTED_LEFT;

			}

		}

	}

	// マップチップにあたっている状態で画面外に出たら
	if (isHitMapChip) {

		Vec2<int> windowSize = WinApp::Instance()->GetWinCenter();
		windowSize *= Vec2<int>(2, 2);

		// ボスとプレイヤーの距離
		float distanceX = fabs(lineCenterPos.x - pos.x);
		float disntaceY = fabs(lineCenterPos.y - pos.y);

		// ウィンドウ左右
		if (windowSize.x <= pos.x + scale.x - ScrollMgr::Instance()->scrollAmount.x || pos.x - scale.x - ScrollMgr::Instance()->scrollAmount.x <= 0) {

			stuckWindowTimer = STRUCK_WINDOW_TIMER;
			ShakeMgr::Instance()->SetShake(20);
			SuperiorityGauge::Instance()->AddPlayerGauge(10);

		}
		// ウィンドウ上下
		if (windowSize.y <= pos.y + scale.y - ScrollMgr::Instance()->scrollAmount.y || pos.y - scale.y - ScrollMgr::Instance()->scrollAmount.y <= 0) {

			stuckWindowTimer = STRUCK_WINDOW_TIMER;
			ShakeMgr::Instance()->SetShake(20);
			SuperiorityGauge::Instance()->AddPlayerGauge(10);

		}

	}

	prevIntersectedLine = intersectedBuff;

	if (0 < stuckWindowTimer) {

		isHitMapChip = false;

	}

}
