#include "DrawMapChipForSceneChange.h"
#include "TexHandleMgr.h"
#include "StageMgr.h"
#include "CharacterManager.h"

DrawMapChipForSceneChange::DrawMapChipForSceneChange()
{
	auto backBuff = D3D12App::Instance()->GetBackBuffRenderTarget();
	mapBuffer = D3D12App::Instance()->GenerateRenderTarget(backBuff->GetDesc().Format, Color(56, 22, 74, 255), backBuff->GetGraphSize(), L"SceneChangeMapSS");
}

void DrawMapChipForSceneChange::Init(int STAGE_NUM)
{
	StageMgr::Instance()->SetLocalMapChipData(STAGE_NUM, 0);
	StageMgr::Instance()->SetLocalMapChipDrawBlock(STAGE_NUM, 0);
	stageNum = STAGE_NUM;
	isSS = true;

	//RoomMapChipArray tmp = *StageMgr::Instance()->GetLocalMap();
	//Vec2<float>distance = (CharacterManager::Instance()->Left()->pos - CharacterManager::Instance()->Right()->pos) / 2.0f;
	//ScrollMgr::Instance()->Init(CharacterManager::Instance()->Left()->pos + distance, Vec2<float>(tmp[0].size() * MAP_CHIP_SIZE, tmp.size() * MAP_CHIP_SIZE), { 0.0f,-40.0f });
}

void DrawMapChipForSceneChange::Finalize()
{
	isSS = false;
}

void DrawMapChipForSceneChange::Draw()
{
	if (isSS)
	{
		KuroEngine::Instance().Graphics().SetRenderTargets({ mapBuffer });
		KuroEngine::Instance().Graphics().ClearRenderTarget({ mapBuffer });
		DrawMapChip(*StageMgr::Instance()->GetLocalMap(), *StageMgr::Instance()->GetLocalDrawMap(), stageNum, 0);
	}
}

void DrawMapChipForSceneChange::DrawMapChip(const vector<vector<int>> &mapChipData, vector<vector<MapChipDrawData>> &mapChipDrawData, const int &stageNum, const int &roomNum)
{
	std::map<int, std::vector<ChipData>>datas;

	// �`�悷��`�b�v�̃T�C�Y
	const float DRAW_MAP_CHIP_SIZE = MAP_CHIP_SIZE * 1.0f;
	SizeData wallChipMemorySize = StageMgr::Instance()->GetMapChipSizeData(MAPCHIP_TYPE_STATIC_BLOCK);


	// �}�b�v�`�b�v�̏c�̗v�f�����擾�B
	const int HEIGHT = mapChipData.size();
	for (int height = 0; height < HEIGHT; ++height) {

		// �}�b�v�`�b�v�̉��̗v�f�����擾�B
		const int WIDTH = mapChipData[height].size();
		for (int width = 0; width < WIDTH; ++width) {

			if (mapChipDrawData[height][width].shocked)mapChipDrawData[height][width].shocked -= 0.02f;
			if (mapChipDrawData[height][width].expEaseRate < 1.0f)mapChipDrawData[height][width].expEaseRate += 0.005f;

			// �u���b�N�ȊO�������珈�����΂��B
			bool blockFlag = (mapChipData[height][width] >= wallChipMemorySize.min && mapChipData[height][width] <= wallChipMemorySize.max);
			if (blockFlag)
			{
				// �X�N���[���ʂ���`�悷��ʒu�����߂�B
				const Vec2<float> drawPos = ScrollMgr::Instance()->Affect({ width * MAP_CHIP_SIZE,height * MAP_CHIP_SIZE });

				// ��ʊO��������`�悵�Ȃ��B
				if (drawPos.x < -DRAW_MAP_CHIP_SIZE || drawPos.x > WinApp::Instance()->GetWinSize().x + DRAW_MAP_CHIP_SIZE) continue;
				if (drawPos.y < -DRAW_MAP_CHIP_SIZE || drawPos.y > WinApp::Instance()->GetWinSize().y + DRAW_MAP_CHIP_SIZE) continue;


				vector<std::shared_ptr<MapChipAnimationData>>tmpAnimation = StageMgr::Instance()->animationData;
				int handle = -1;
				if (height < 0 || mapChipDrawData.size() <= height) continue;
				if (width < 0 || mapChipDrawData[height].size() <= width) continue;
				//�A�j���[�V�����t���O���L���Ȃ�A�j���[�V�����p�̏����s��
				if (mapChipDrawData[height][width].animationFlag)
				{
					int arrayHandle = mapChipDrawData[height][width].handle;
					++mapChipDrawData[height][width].interval;
					//�A�j���[�V�����̊Ԋu
					if (mapChipDrawData[height][width].interval % tmpAnimation[arrayHandle]->maxInterval == 0)
					{
						++mapChipDrawData[height][width].animationNum;
						mapChipDrawData[height][width].interval = 0;
					}
					//�A�j���[�V�����摜�̑����ɒB������ŏ��ɖ߂�
					if (tmpAnimation[arrayHandle]->handle.size() <= mapChipDrawData[height][width].animationNum)
					{
						mapChipDrawData[height][width].animationNum = 0;
					}
					//���������A�j���[�V�����̉摜����n��
					handle = tmpAnimation[arrayHandle]->handle[mapChipDrawData[height][width].animationNum];
				}
				else
				{
					handle = mapChipDrawData[height][width].handle;
				}

				//mapChipDrawData[height][width].shocked = KuroMath::Lerp(mapChipDrawData[height][width].shocked, 0.0f, 0.8f);

				Vec2<float> pos = drawPos;
				pos += mapChipDrawData[height][width].offset;
				if (0 <= handle)
				{
					ChipData chipData;
					chipData.pos = pos;
					chipData.radian = mapChipDrawData[height][width].radian;
					chipData.shocked = mapChipDrawData[height][width].shocked;
					chipData.expEaseRate = mapChipDrawData[height][width].expEaseRate;
					datas[handle].emplace_back(chipData);
					//DrawFunc::DrawRotaGraph2D({ pos.x, pos.y }, 1.6f * ScrollMgr::Instance()->zoom, mapChipDrawData[height][width].radian, TexHandleMgr::GetTexBuffer(handle));
				}
			}
		}
	}

	while (drawMap.size() < datas.size())
	{
		drawMap.emplace_back();
	}

	int i = 0;
	for (auto itr = datas.begin(); itr != datas.end(); ++itr)
	{
		for (int chipIdx = 0; chipIdx < itr->second.size(); ++chipIdx)
		{
			drawMap[i].AddChip(itr->second[chipIdx]);
		}
		drawMap[i].Draw(TexHandleMgr::GetTexBuffer(itr->first));
		i++;
	}

}