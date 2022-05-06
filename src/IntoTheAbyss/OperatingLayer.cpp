#include "OperatingLayer.h"
#include"CharacterManager.h"

OperateMove::OperateMove()
{
	initFlag = false;
}

void OperateMove::Update(const Vec2<float> &VELOCITY)
{
	//移動
	if (!initFlag)
	{
		oldPos = CharacterManager::Instance()->Right()->pos;
		initFlag = true;
	}
	CharacterAIOrder::Instance()->vel = VELOCITY;
	oldPos = CharacterManager::Instance()->Right()->pos;
}

AiResult OperateMove::CurrentProgress()
{
	//座標が動いていたら成功、止まっていたら失敗
	if (CharacterManager::Instance()->Right()->pos != oldPos)
	{
		return AiResult::OPERATE_SUCCESS;
	}
	else
	{
		return AiResult::OPERATE_FAIL;
	}
}