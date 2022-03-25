#include "EventBlock.h"

EventBlock::EventBlock() :initFlag(false)
{
}

void EventBlock::Init(const Vec2<float> &POS)
{
	pos = POS;
	initFlag = true;
}

void EventBlock::Finalize()
{
	initFlag = false;
}

bool EventBlock::HitBox(const Vec2<float> &PLAYER_POS)
{
	//初期化していないブロックは当たり判定は返さない
	if (initFlag)
	{
		return false;
	}


	return false;
}
