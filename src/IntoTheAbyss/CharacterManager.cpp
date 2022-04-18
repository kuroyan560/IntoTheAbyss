#include "CharacterManager.h"
#include"Player.h"
#include"Boss.h"

std::shared_ptr<CharacterInterFace> CharacterManager::CreateCharacter(const  PLAYABLE_CHARACTER_NAME& CharacterName, const int& ControllerIdx)
{
	switch (CharacterName)
	{
	case PLAYABLE_LUNA:
		return std::make_shared<Player>(CharacterName, ControllerIdx);
	case PLAYABLE_LACY:
		return std::make_shared<Player>(CharacterName, ControllerIdx);
	case PLAYABLE_BOSS_0:
		return std::make_shared<Boss>();
	}
	assert(0);
}

void CharacterManager::CharactersSelectInit()
{
	nowSelectTeam = LEFT_TEAM;
}

void CharacterManager::CharactersSelectUpdate()
{
	//���I��
	if (nowSelectTeam == LEFT_TEAM)
	{
		if (UsersInput::Instance()->ControllerOnTrigger(0, L_DOWN))
		{
			nowSelectTeam = RIGHT_TEAM;
		}
	}
	//�E�I��
	else
	{
		if (UsersInput::Instance()->ControllerOnTrigger(0, L_UP))
		{
			nowSelectTeam = LEFT_TEAM;
		}
	}

	//�L�����N�^�[�I��
	if (UsersInput::Instance()->ControllerOnTrigger(0, L_LEFT) && 0 < characterName[nowSelectTeam])
	{
		characterName[nowSelectTeam] = (PLAYABLE_CHARACTER_NAME)(characterName[nowSelectTeam] - 1);
	}
	else if (UsersInput::Instance()->ControllerOnTrigger(0, L_RIGHT) && characterName[nowSelectTeam] < PLAYABLE_CHARACTER_NUM - 1)
	{
		characterName[nowSelectTeam] = (PLAYABLE_CHARACTER_NAME)(characterName[nowSelectTeam] + 1);
	}
}

void CharacterManager::CharactersSelectDraw()
{
	ImGui::Begin("CharacterSelect");

	ImGui::Text("Stick Up   / Down  : Select Left/Right");
	ImGui::Text("Stick Left / Right : Select Character");
	ImGui::Text("Bbutton : Return To StageSelect");
	ImGui::Text("Abutton : GameStart");

	ImGui::Separator();

	static const std::string NOW_SELECT = "  - Now Select -";
	static const std::string NAME[PLAYABLE_CHARACTER_NUM] =
	{
		"Luca",
		"Lacy",
		"Boss_0"
	};
	static const std::string CHARACTER[TEAM_NUM] =
	{
		"Left Character",
		"RightCharacter"
	};

	for (int i = 0; i < TEAM_NUM; ++i)
	{
		std::string text = CHARACTER[i] + NAME[characterName[i]] + (nowSelectTeam == i ? NOW_SELECT : "");
		ImGui::Text("%s", text.c_str());
	}

	ImGui::End();
}

void CharacterManager::CharactersGenerate()
{
	//�L�����N�^�[����
	for (int i = 0; i < TEAM_NUM; ++i)
	{
		characters[i] = CreateCharacter(characterName[i], i);
	}
	//�ǂ���̃L�����N�^�[�������������Ă���łȂ��ƁA���o�^�o���Ȃ�
	for (int i = 0; i < TEAM_NUM; ++i)
	{
		characters[i]->RegisterCharacterInfo(characters[TEAM_NUM - i - 1], (WHICH_TEAM)i);
	}
}

void CharacterManager::CharactersInit(const Vec2<float>& RespawnPos)
{
	const auto offset = Vec2<float>(CharacterInterFace::LINE_LENGTH, 0.0f);
	characters[LEFT_TEAM]->Init(RespawnPos - offset);
	characters[RIGHT_TEAM]->Init(RespawnPos + offset);
}