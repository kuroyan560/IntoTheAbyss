#pragma once
#include"../KuroEngine.h"

class ThornBlock
{
public:
	ThornBlock();
	void Init(const Vec2<float> &LEFT_UP_POS,const Vec2<float>&RIGHT_DOWN_POS);
	void Finalize();
	bool HitBox(Vec2<float> &PLAYER_POS, const Vec2<float> &SIZE, Vec2<float> &PLAYER_VEL, Vec2<float> &PLAYER_PREV_POS);
	void Draw();

	static Vec2<float>adjValue;//当たり判定調整用
private:
	Vec2<float> leftUpPos;
	Vec2<float> size;
	Color color;
};

