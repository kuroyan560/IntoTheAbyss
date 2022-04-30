#include "OperatingLayer.h"

OperateMove::OperateMove(const std::shared_ptr<Vec2<float>> &POS)
{
	pos = POS;
	oldPos = *pos;
}

void OperateMove::Update(const Vec2<float> &VELOCITY)
{
	oldPos = *pos;
	*pos += VELOCITY;
}

AiResult OperateMove::CurrentProgress()
{
	//座標が動いていたら成功、止まっていたら失敗
	if (*pos != oldPos)
	{
		return AiResult::OPERATE_SUCCESS;
	}
	else
	{
		return AiResult::OPERATE_FAIL;
	}
}