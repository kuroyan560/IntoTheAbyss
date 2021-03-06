#include "MapChipCollider.h"
#include "DrawFunc.h"
#include "StageMgr.h"

bool MapChipCollider::IsIntersected(const Vec2<float>& posA1, const Vec2<float>& posA2, const Vec2<float>& posB1, const Vec2<float>& posB2)
{
	/*--線分の外積を計算して交差判定を行う--*/
	//第一回 線分Aから見たBの交差判定
	Vec2<float> buffA = Vec2<float>(posA2.x - posA1.x, posA2.y - posA1.y);
	buffA.Normalize();
	Vec2<float> buffB = Vec2<float>(posB1.x - posA1.x, posB1.y - posA1.y);
	buffB.Normalize();
	Vec2<float> buffC = Vec2<float>(posA2.x - posA1.x, posA2.y - posA1.y);
	buffC.Normalize();
	Vec2<float> buffD = Vec2<float>(posB2.x - posA1.x, posB2.y - posA1.y);
	buffD.Normalize();
	float buffE = buffA.Cross(buffB);
	float buffF = buffC.Cross(buffD);
	float result1 = buffE * buffF;
	bool zero1 = false;
	if (buffE * buffF == 0) zero1 = true;
	//第二回 線分Bから見たAの交差判定
	buffA = Vec2<float>(posB2.x - posB1.x, posB2.y - posB1.y);
	buffA.Normalize();
	buffB = Vec2<float>(posA1.x - posB1.x, posA1.y - posB1.y);
	buffB.Normalize();
	buffC = Vec2<float>(posB2.x - posB1.x, posB2.y - posB1.y);
	buffC.Normalize();
	buffD = Vec2<float>(posA2.x - posB1.x, posA2.y - posB1.y);
	buffD.Normalize();
	buffE = buffA.Cross(buffB);
	buffF = buffC.Cross(buffD);
	float result2 = buffE * buffF;
	bool zero2 = false;
	if (buffE * buffF == 0) zero2 = true;

	//線分が交差している時は、線分から見て交差判定したい線分の端点2つが両側にある時。
	//外積で左右判定をすると、交差している時は値の結果が+と-になる。
	//つまり両方の外積を掛けて結果が-になった場合のみ交差している。
	//線分AからみてBを判定、線分BからみてAを判定と二通り判定を行う。
	//この2つの判定結果を掛けた時に-、若しくは0の時2つの線分は交差している。
	if ((result1 <= 0 && result2 <= 0) ||
		(result1 <= 0 && zero2) ||
		(zero1 && result2 <= 0)) {
		return true;
	}
	return false;
}

Vec2<float> MapChipCollider::CalIntersectPoint(Vec2<float> posA1, Vec2<float> posA2, Vec2<float> posB1, Vec2<float> posB2)
{
	//交点を求める この式は資料そのまま
	Vec2<float> buff = Vec2<float>(posB2.x - posB1.x, posB2.y - posB1.y);
	double d1 = abs(buff.Cross(Vec2<float>(posA1.x - posB1.x, posA1.y - posB1.y)));
	double d2 = abs(buff.Cross(Vec2<float>(posA2.x - posB1.x, posA2.y - posB1.y)));
	double t = d1 / (d1 + d2);

	return Vec2<float>(posA1.x + (posA2.x - posA1.x) * t, posA1.y + (posA2.y - posA1.y) * t);
}

INTERSECTED_LINE MapChipCollider::CheckHitMapChipBasedOnTheVel(Vec2<float>& pos, const Vec2<float>& prevFramePos, const Vec2<float>& vel, const Vec2<float>& size, const MapChipArray& mapChipData, Vec2<int>& hitChipIndex, bool OnlyUnBrokenBlock)
{
	/*===== マップチップとプレイヤーの当たり判定 =====*/

	// プレイヤーの移動量を取得
	Vec2<float> lineStartPos = prevFramePos;
	Vec2<float> lineEndPos = pos + vel;

	// 交点保存用
	vector<HitData> intersectPos;

	const int HEIGHT = mapChipData.size();
	SizeData mapChipSizeData = StageMgr::Instance()->GetMapChipSizeData(MAPCHIP_TYPE_STATIC_BLOCK);
	for (int height = 0; height < HEIGHT; ++height) {

		// マップの横
		const int WIDTH = mapChipData[height].size();
		for (int width = 0; width < WIDTH; ++width) {

			//壊れないブロックとのみ当たり判定を取る
			if (OnlyUnBrokenBlock)
			{
				if (mapChipData[height][width].chipType != MAPCHIP_TYPE_STATIC_UNBROKEN_BLOCK)continue;
			}

			// マップIDが0だったら処理を飛ばす。
			if (!(mapChipSizeData.min <= mapChipData[height][width].chipType && mapChipData[height][width].chipType <= mapChipSizeData.max)) continue;

			// このマップの中心座標を求める。
			const float centerX = width * MAP_CHIP_SIZE;
			const float centerY = height * MAP_CHIP_SIZE;

			// プレイヤーとの距離が一定以上離れていたら処理を行わない。
			if (MAP_CHIP_SIZE * 10.0f <= Vec2<float>(centerX - pos.x, centerY - pos.y).Length()) {
				continue;
			}

			// マップチップをプレイヤー分拡張して4頂点を求める。
			const Vec2<float> rightTop = { centerX + MAP_CHIP_HALF_SIZE , centerY - MAP_CHIP_HALF_SIZE };
			const Vec2<float> rightBottom = { centerX + MAP_CHIP_HALF_SIZE , centerY + MAP_CHIP_HALF_SIZE };
			const Vec2<float> leftTop = { centerX - MAP_CHIP_HALF_SIZE , centerY - MAP_CHIP_HALF_SIZE };
			const Vec2<float> leftBottom = { centerX - MAP_CHIP_HALF_SIZE , centerY + MAP_CHIP_HALF_SIZE };

			// 全ての線分との当たり判定を行う。

			// 上辺
			if (IsIntersected(rightTop, leftTop, lineStartPos, lineEndPos)) {

				// 当たっていたら交点を計算して保存
				HitData buff;
				buff.hitPos = CalIntersectPoint(rightTop, leftTop, lineStartPos, lineEndPos);

				buff.hitLine = INTERSECTED_TOP;
				buff.hitChipIndex = Vec2<int>(width, height);
				intersectPos.push_back(buff);

			}

			// 右辺
			bool isHitRight = false;
			isHitRight = IsIntersected(rightTop, rightBottom, lineStartPos, lineEndPos);
			if (isHitRight) {

				// 当たっていたら交点を計算して保存
				HitData buff;
				buff.hitPos = CalIntersectPoint(rightTop, rightBottom, lineStartPos, lineEndPos);

				buff.hitLine = INTERSECTED_RIGHT;
				buff.hitChipIndex = Vec2<int>(width, height);
				intersectPos.push_back(buff);

			}

			// 下辺
			if (IsIntersected(leftBottom, rightBottom, lineStartPos, lineEndPos)) {

				// 当たっていたら交点を計算して保存
				HitData buff;
				buff.hitPos = CalIntersectPoint(leftBottom, rightBottom, lineStartPos, lineEndPos);

				buff.hitLine = INTERSECTED_BOTTOM;
				buff.hitChipIndex = Vec2<int>(width, height);
				intersectPos.push_back(buff);

			}

			// 左辺
			bool isIntersectedLeft = false;
			isIntersectedLeft = IsIntersected(leftBottom, leftTop, lineStartPos, lineEndPos);
			if (isIntersectedLeft) {

				// 当たっていたら交点を計算して保存
				HitData buff;
				buff.hitPos = CalIntersectPoint(leftBottom, leftTop, lineStartPos, lineEndPos);

				buff.hitLine = INTERSECTED_LEFT;
				buff.hitChipIndex = Vec2<int>(width, height);
				intersectPos.push_back(buff);

			}

		}

	}



	// 交点が0個だったら次へ
	if (0 < intersectPos.size()) {

		// 全ての交点の中からプレイヤーとの距離が最小のものを求める。
		HitData miniIntersectedPoint;
		miniIntersectedPoint.hitPos = { 100000,100000 };
		for (int index = 0; index < intersectPos.size(); ++index) {

			// 二点間の距離が保存されているものよりも小さかったら、その座標を保存する。
			if (Vec2<float>(intersectPos[index].hitPos - prevFramePos).Length() < Vec2<float>(miniIntersectedPoint.hitPos - prevFramePos).Length()) {

				miniIntersectedPoint = intersectPos[index];

			}

		}

		// 押し戻してから当たった辺を返す。


		float offset = 1.0f;

		// 最小の交点の種類によって処理を分ける。
		if (miniIntersectedPoint.hitLine == INTERSECTED_TOP) {

			// 押し戻す。
			pos.y = miniIntersectedPoint.hitPos.y - size.y - offset;


		}
		else if (miniIntersectedPoint.hitLine == INTERSECTED_RIGHT) {

			// 押し戻す。
			pos.x = miniIntersectedPoint.hitPos.x + size.x + offset;

		}
		else if (miniIntersectedPoint.hitLine == INTERSECTED_BOTTOM) {

			// 押し戻す。
			pos.y = miniIntersectedPoint.hitPos.y + size.y + offset;


		}
		else if (miniIntersectedPoint.hitLine == INTERSECTED_LEFT) {

			// 押し戻す。
			pos.x = miniIntersectedPoint.hitPos.x - size.x - offset;

		}

		hitChipIndex = miniIntersectedPoint.hitChipIndex;

		return miniIntersectedPoint.hitLine;

	}

	hitChipIndex = { -1,-1 };

	return INTERSECTED_NONE;
}

INTERSECTED_LINE MapChipCollider::CheckHitMapChipBasedOnTheScale(Vec2<float>& pos, const Vec2<float>& prevFramePos, const Vec2<float>& size, const MapChipArray& mapChipData, const INTERSECTED_LINE& direction, Vec2<int>& hitChipIndex, bool OnlyUnBrokenBlock)
{
	/*===== マップチップとプレイヤーの当たり判定 =====*/


	// 当たり判定に使用する線分を生成。
	Vec2<float> checkHitDirection = {};
	Vec2<float> checkHitPos = {};

	switch (direction) {

	case INTERSECTED_TOP:

		// 上方向の線分を生成。
		checkHitDirection = { pos.x, pos.y - size.y };
		checkHitPos = { pos.x, pos.y + size.y };

		break;

	case INTERSECTED_BOTTOM:

		// 下方向の線分を生成。
		checkHitDirection = { pos.x, pos.y + size.y };
		checkHitPos = { pos.x, pos.y - size.y };

		break;

	case INTERSECTED_LEFT:

		// 左方向の線分を生成。
		checkHitDirection = { pos.x - size.x, pos.y };
		checkHitPos = { pos.x + size.x, pos.y };

		break;

	case INTERSECTED_RIGHT:

		// 右方向の線分を生成。
		checkHitDirection = { pos.x + size.x, pos.y };
		checkHitPos = { pos.x - size.x, pos.y };

		break;

	default:

		break;

	}

	// 交点保存用
	vector<HitData> intersectPos;

	const int HEIGHT = mapChipData.size();
	SizeData mapChipSizeData = StageMgr::Instance()->GetMapChipSizeData(MAPCHIP_TYPE_STATIC_BLOCK);
	for (int height = 0; height < HEIGHT; ++height) {

		// マップの横
		const int WIDTH = mapChipData[height].size();
		for (int width = 0; width < WIDTH; ++width) {

			//壊れないブロックとのみ当たり判定を取る
			if (OnlyUnBrokenBlock)
			{
				if (mapChipData[height][width].chipType != MAPCHIP_TYPE_STATIC_UNBROKEN_BLOCK)continue;
			}

			// マップIDが0だったら処理を飛ばす。
			if (!(mapChipSizeData.min <= mapChipData[height][width].chipType && mapChipData[height][width].chipType <= mapChipSizeData.max)) continue;

			// このマップの中心座標を求める。
			const float centerX = width * MAP_CHIP_SIZE;
			const float centerY = height * MAP_CHIP_SIZE;

			// プレイヤーとの距離が一定以上離れていたら処理を行わない。
			if (size.x * 3.0f <= fabs(centerX - pos.x)) {
				continue;
			}
			if (size.y * 3.0f <= fabs(centerY - pos.y)) {
				continue;
			}

			// マップチップをプレイヤー分拡張して4頂点を求める。
			const Vec2<float> rightTop = { centerX + MAP_CHIP_HALF_SIZE , centerY - MAP_CHIP_HALF_SIZE };
			const Vec2<float> rightBottom = { centerX + MAP_CHIP_HALF_SIZE , centerY + MAP_CHIP_HALF_SIZE };
			const Vec2<float> leftTop = { centerX - MAP_CHIP_HALF_SIZE , centerY - MAP_CHIP_HALF_SIZE };
			const Vec2<float> leftBottom = { centerX - MAP_CHIP_HALF_SIZE , centerY + MAP_CHIP_HALF_SIZE };


			// 全ての線分との当たり判定を行う。

			// 上辺
			if (IsIntersected(rightTop, leftTop, checkHitPos, checkHitDirection)) {

				// 当たっていたら交点を計算して保存
				HitData buff;
				buff.hitPos = CalIntersectPoint(rightTop, leftTop, checkHitPos, checkHitDirection);

				buff.hitLine = INTERSECTED_TOP;
				buff.hitChipIndex = Vec2<int>(width, height);
				intersectPos.push_back(buff);

			}

			// 右辺
			if (IsIntersected(rightTop, rightBottom, checkHitPos, checkHitDirection)) {

				// 当たっていたら交点を計算して保存
				HitData buff;
				buff.hitPos = CalIntersectPoint(rightTop, rightBottom, checkHitPos, checkHitDirection);

				buff.hitLine = INTERSECTED_RIGHT;
				buff.hitChipIndex = Vec2<int>(width, height);
				intersectPos.push_back(buff);

			}

			// 下辺
			if (IsIntersected(leftBottom, rightBottom, checkHitPos, checkHitDirection)) {

				// 当たっていたら交点を計算して保存
				HitData buff;
				buff.hitPos = CalIntersectPoint(leftBottom, rightBottom, checkHitPos, checkHitDirection);

				buff.hitLine = INTERSECTED_BOTTOM;
				buff.hitChipIndex = Vec2<int>(width, height);
				intersectPos.push_back(buff);

			}

			// 左辺
			if (IsIntersected(leftBottom, leftTop, checkHitPos, checkHitDirection)) {

				// 当たっていたら交点を計算して保存
				HitData buff;
				buff.hitPos = CalIntersectPoint(leftBottom, leftTop, checkHitPos, checkHitDirection);

				buff.hitLine = INTERSECTED_LEFT;
				buff.hitChipIndex = Vec2<int>(width, height);
				intersectPos.push_back(buff);


			}


		}

	}


	// 交点が0個だったら次へ
	if (0 < intersectPos.size()) {

		// 全ての交点の中からプレイヤーとの距離が最小のものを求める。
		HitData miniIntersectedPoint;
		miniIntersectedPoint.hitPos = { 100000,100000 };
		for (int index = 0; index < intersectPos.size(); ++index) {

			// 二点間の距離が保存されているものよりも小さかったら、その座標を保存する。
			if (Vec2<float>(intersectPos[index].hitPos - pos).Length() < Vec2<float>(miniIntersectedPoint.hitPos - pos).Length()) {

				miniIntersectedPoint = intersectPos[index];

			}

		}

		// ピッタリ押し戻したら不都合なことが起こるので、多少オフセット値を設ける。
		float pushBackOffset = 0.0f;

		// 最小の交点の種類によって処理を分ける。
		if (miniIntersectedPoint.hitLine == INTERSECTED_TOP) {

			// 押し戻す。
			pos.y = miniIntersectedPoint.hitPos.y - size.y;


		}
		else if (miniIntersectedPoint.hitLine == INTERSECTED_RIGHT) {

			// 押し戻す。
			pos.x = miniIntersectedPoint.hitPos.x + size.x + pushBackOffset;

		}
		else if (miniIntersectedPoint.hitLine == INTERSECTED_BOTTOM) {

			// 押し戻す。
			pos.y = miniIntersectedPoint.hitPos.y + size.y;


		}
		else if (miniIntersectedPoint.hitLine == INTERSECTED_LEFT) {

			// 押し戻す。
			pos.x = miniIntersectedPoint.hitPos.x - size.x - pushBackOffset;

		}

		hitChipIndex = miniIntersectedPoint.hitChipIndex;

		return miniIntersectedPoint.hitLine;

	}


	hitChipIndex = { -1,-1 };

	return INTERSECTED_NONE;
}
