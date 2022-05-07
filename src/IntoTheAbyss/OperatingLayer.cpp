#include "OperatingLayer.h"
#include"CharacterManager.h"

OperateMove::OperateMove()
{
	initFlag = false;
}

void OperateMove::Update(const Vec2<float> &VELOCITY)
{
	//移動
	CharacterAIOrder::Instance()->vel = VELOCITY;

}

AiResult OperateMove::CurrentProgress()
{
	bool succeedFlag = false;
	//座標が動いていたら成功、止まっていたら失敗
	if (CharacterAIData::Instance()->nowPos != CharacterAIData::Instance()->prevPos)
	{
		succeedFlag = true;
	}
	else
	{
		succeedFlag = false;
	}

	if (succeedFlag)
	{
		return AiResult::OPERATE_SUCCESS;
	}
	else
	{
		return AiResult::OPERATE_FAIL;
	}
}