#include "Barrages.h"

#pragma region WhirlpoolBarrage

void WhirlpoolBarrage::Start()
{

	/*===== ���������� =====*/

	isEnd = false;
	timer = 0;
	angle = 0;
	roundCounter = 0;

}

void WhirlpoolBarrage::Init()
{

	isEnd = true;
	timer = 0;
	angle = 0;
	roundCounter = 0;

}

bool WhirlpoolBarrage::Update(BulletMgrBase& BulletMgr, const Vec2<float>& Pos, const Vec2<float>& TargetPos, const int& GraphHandle)
{

	/*===== �X�V���� =====*/

	if (isEnd) return true;

	// �^�C�}�[���X�V���āA�e�����������X�V����B
	++timer;
	if (SHOOT_DELAY < timer) {

		timer = 0;

		// �e�𐶐��B
		BulletMgr.Generate(GraphHandle, Pos, angle, SPEED);

		// �p�x���X�V�B
		angle += ANGLE;

		// �p�x������𒴂�����������������s���B
		if (DirectX::XM_2PI <= angle) {

			angle = 0;
			++roundCounter;

		}

	}

	// ���ȏ�������珈�����I����B
	if (MAX_ROUND <= roundCounter) {

		isEnd = true;

	}

	return isEnd;
}

#pragma endregion


#pragma region Whirlpool2WayBarrage

void Whirlpool2WayBarrage::Start()
{

	/*===== ���������� =====*/

	isEnd = false;
	timer = 0;
	angle = 0;
	roundCounter = 0;

}

void Whirlpool2WayBarrage::Init()
{

	isEnd = true;
	timer = 0;
	angle = 0;
	roundCounter = 0;

}

bool Whirlpool2WayBarrage::Update(BulletMgrBase& BulletMgr, const Vec2<float>& Pos, const Vec2<float>& TargetPos, const int& GraphHandle)
{

	/*===== �X�V���� =====*/

	if (isEnd) return true;

	// �^�C�}�[���X�V���āA�e�����������X�V����B
	++timer;
	if (SHOOT_DELAY < timer) {

		timer = 0;

		// �e�𐶐��B
		BulletMgr.Generate(GraphHandle, Pos, angle, SPEED);
		BulletMgr.Generate(GraphHandle, Pos, -angle, SPEED);

		// �p�x���X�V�B
		angle += ANGLE;

		// �p�x������𒴂�����������������s���B
		if (DirectX::XM_2PI <= angle) {

			angle = 0;
			++roundCounter;

		}

	}

	// ���ȏ�������珈�����I����B
	if (MAX_ROUND <= roundCounter) {

		isEnd = true;

	}

	return isEnd;
}

#pragma endregion


#pragma region CircularBarrage

void CircularBarrage::Start()
{

	/*===== �J�n =====*/

	isEnd = false;

}

void CircularBarrage::Init()
{

	/*===== �����I�� =====*/

	isEnd = true;

}

bool CircularBarrage::Update(BulletMgrBase& BulletMgr, const Vec2<float>& Pos, const Vec2<float>& TargetPos, const int& GraphHandle)
{

	/*===== �X�V���� =====*/

	if (isEnd) return true;

	float shootAngle = 0;

	// ����ɒe�𐶐�����B
	for (int index = 0; index < SHOOT_COUNT_PER_ROUND; ++index) {

		// �e�𐶐����Ċp�x���X�V����B
		BulletMgr.Generate(GraphHandle, Pos, shootAngle, SPEED);

		shootAngle += ANGLE;

	}

	// ��񌂂�����I���B
	isEnd = true;

	return isEnd;
}

#pragma endregion


#pragma region TargetShotBarrage

void TargetShotBarrage::Start()
{

	/*===== �J�n =====*/

	isEnd = false;
	timer = 0;
	shotCounter = 0;

}

void TargetShotBarrage::Init()
{

	/*===== �����I�� =====*/

	isEnd = true;

}

bool TargetShotBarrage::Update(BulletMgrBase& BulletMgr, const Vec2<float>& Pos, const Vec2<float>& TargetPos, const int& GraphHandle)
{

	/*===== �X�V���� =====*/

	if (isEnd) return true;

	// �^�C�}�[���X�V���Ēe�����B
	++timer;
	if (SHOOT_DELAY <= timer) {

		timer = 0;

		++shotCounter;

		BulletMgr.Generate(GraphHandle, Pos, atan2f(TargetPos.y - Pos.y, TargetPos.x - Pos.x), SPEED);

	}

	// ��萔��������I���B
	if (SHOOT_COUNT <= shotCounter) isEnd = true;

	return isEnd;
}

#pragma endregion


#pragma region TargetShot3WayBarrage

void TargetShot3WayBarrage::Start()
{

	/*===== �J�n =====*/

	isEnd = false;
	timer = 0;
	shotCounter = 0;

}

void TargetShot3WayBarrage::Init()
{

	/*===== �����I�� =====*/

	isEnd = true;

}

bool TargetShot3WayBarrage::Update(BulletMgrBase& BulletMgr, const Vec2<float>& Pos, const Vec2<float>& TargetPos, const int& GraphHandle)
{

	/*===== �X�V���� =====*/

	if (isEnd) return true;

	// �^�C�}�[���X�V���Ēe�����B
	++timer;
	if (SHOOT_DELAY <= timer) {

		timer = 0;

		++shotCounter;

		BulletMgr.Generate(GraphHandle, Pos, atan2f(TargetPos.y - Pos.y, TargetPos.x - Pos.x), SPEED);
		BulletMgr.Generate(GraphHandle, Pos, atan2f(TargetPos.y - Pos.y, TargetPos.x - Pos.x) - AngleDispersion, SPEED);
		BulletMgr.Generate(GraphHandle, Pos, atan2f(TargetPos.y - Pos.y, TargetPos.x - Pos.x) + AngleDispersion, SPEED);

	}

	// ��萔��������I���B
	if (SHOOT_COUNT <= shotCounter) isEnd = true;

	return isEnd;
}

#pragma endregion


#pragma region ShotGunBarrage

void ShotGunBarrage::Start()
{

	/*===== �J�n =====*/

	isEnd = false;

}

void ShotGunBarrage::Init()
{

	/*===== �����I�� =====*/

	isEnd = true;

}

bool ShotGunBarrage::Update(BulletMgrBase& BulletMgr, const Vec2<float>& Pos, const Vec2<float>& TargetPos, const int& GraphHandle)
{

	/*===== �X�V���� =====*/

	if (isEnd) return true;

	// �L�����Ԃ̊p�x�B
	float baseAngle = atan2f(TargetPos.y - Pos.y, TargetPos.x - Pos.x);

	// �w��̐����e�����B
	for (int index = 0; index < SHOOT_COUNT; ++index) {

		// �U��΂点�邽�߂Ɋp�x�𑽏����炷�B
		float shootAngle = baseAngle + (KuroFunc::GetRand(AngleDispersion * 2.0f) - AngleDispersion);

		// �ړ����x�������_���ŋ��߂�B
		float speed = KuroFunc::GetRand(MAX_SPEED - MIN_SPEED) + MIN_SPEED;

		// �e�𐶐�����B
		BulletMgr.Generate(GraphHandle, Pos, shootAngle, speed);

	}

	// ��񌂂�����I���B
	isEnd = true;

	return isEnd;

}

#pragma endregion


#pragma region WaveBarrage

void WaveBarrage::Start()
{

	/*===== �J�n =====*/

	isEnd = false;
	timer = 0;
	sinTimer = 0;

}

void WaveBarrage::Init()
{

	/*===== �����I�� =====*/

	isEnd = true;

}

bool WaveBarrage::Update(BulletMgrBase& BulletMgr, const Vec2<float>& Pos, const Vec2<float>& TargetPos, const int& GraphHandle)
{

	/*===== �X�V���� =====*/

	if (isEnd) return true;

	// �T�C���g���X�V����B
	sinTimer += FRAME_TIMER;

	float charaAngle = atan2f(TargetPos.y - Pos.y, TargetPos.x - Pos.x);
	float shotAngle = sinf(sinTimer) * AngleDispersion;

	// �e�𐶐�����B
	if (timer % SHOOT_DELAY == 0)
	{
		BulletMgr.Generate(GraphHandle, Pos, charaAngle + shotAngle, SPEED);
	}

	// �^�C�}�[���X�V���A�^�C�}�[���K��l�ɒB�����珈�����I����B
	++timer;
	if (TIMER <= timer) isEnd = true;

	return isEnd;
}

#pragma endregion