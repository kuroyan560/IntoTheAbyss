#pragma once
#include "Vec.h"
#include <vector>
using namespace std;
#include"WinApp.h"

class LightManager;
#include"AfterImg.h"

#include"Light.h"

// プレイヤーの手クラス
class PlayerHand {

public:

	/*-- メンバ変数 --*/

	Vec2<float> handPos;			// 腕の描画座標
	//Vec2<float> vel;				// 弾を打った時の移動量
	static const int OFFSET_RADIUS_TIME = 30;
	int offsetRadiusTimer;
	Vec2<float> sightPos;			// 照準座標
	Vec2<float> drawPos;			// Draw You
	Vec2<float> muzzlePos;			// 銃口の座標
	float armDistance;		// プレイヤーの中心からの距離 右手と左手で変えるため
	float inputAngle;		// 入力された角度

	AfterImg afterImg;

	const int handGraphHandle;
	const int aimGraphHandle;
	//照準を光らせる
	Light::Point ptLight;

	/*-- 定数 --*/

public:

	const float ARM_RANGE_OF_MOTION = 15.0f;	// 手の可動域
	const float SHOT_VEL = 15.0f;				// 弾を撃った時の反動
	const float FIRST_SHOT_VEL = 20.0f;			// 最初の一発目を撃った時の反動
	const float SIGHT_SIZE = 5.0f;				// 照準のサイズ
	const int PIKE_COOL_TIME = 60;				// ビーコンのクールタイム


	/*-- メンバ関数 --*/

public:

	// コンストラクタ
	PlayerHand(const int& HandGraph, const int& AimGraphHandle);

	// 初期化処理
	void Init(const float& armDistance);

	// 更新処理
	void Update(const Vec2<float>& playerCenterPos);

	// 描画処理
	void Draw(const float& ExtRate, const float& InitAngle, const Vec2<float>& RotaCenterUV, const bool &DRAW_CURSOR);

	// 弾を打った時の処理
	void Shot(const Vec2<float>& forwardVec, const bool& isFirstShot);

	// 当たり判定関数
	void CheckHit(const vector<vector<int>>& mapData);

	// 照準を描画する最短距離を求める。
	void CheckShortestPoint(const vector<vector<int>>& mapData);

	bool IsIntersected(const Vec2<float>& posA1, const Vec2<float>& posA2, const Vec2<float>& posB1, const Vec2<float>& posB2);

	Vec2<float> CalIntersectPoint(Vec2<float> posA1, Vec2<float> posA2, Vec2<float> posB1, Vec2<float> posB2);

	// Angle wo Tikadukeru
	inline void PutCloseAngle(const float& defAngle) { inputAngle += (defAngle - inputAngle) / 10.0f; }

	// 角度のセッタ
	inline void SetAngle(const float& angle) { inputAngle = angle; }
	// 角度のゲッタ
	inline const float& GetAngle() { return inputAngle; }
	// 座標のゲッタ
	inline const Vec2<float>& GetPos() { return handPos; }

	//残像を出す
	void EmitAfterImg(const Vec2<float>& TeleAmount, const int& GraphHandle, const Vec2<float>& GraphSize, const Vec2<bool>& Miror);

private:

	// 切り上げ
	int RoundUp(int size, int align) {
		return UINT(size + align - 1) & ~(align - 1);
	}
};