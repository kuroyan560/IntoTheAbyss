#include "GameScene.h"
#include"KuroEngine.h"
#include"Sprite.h"
#include"GaussianBlur.h"
#include"IntoTheAbyss/StageMgr.h"
#include"IntoTheAbyss/SelectStage.h"
#include"IntoTheAbyss/DebugParameter.h"
#include"IntoTheAbyss/SuperiorityGauge.h"
#include"IntoTheAbyss/GameTimer.h"
#include"IntoTheAbyss/ScoreManager.h"
#include"IntoTheAbyss/WinCounter.h"
#include"IntoTheAbyss/ResultSceneBackGround.h"

GameScene::GameScene()
{
	static const float BACK_GROUND_DEPTH = 7.0f;
	auto backColor = D3D12App::Instance()->GenerateTextureBuffer(Color(56, 22, 74, 255));
	backGround = std::make_shared<Sprite>(backColor, "BackGround");
	backGround->mesh.SetZLayer(BACK_GROUND_DEPTH);
	backGround->mesh.SetSize(WinApp::Instance()->GetExpandWinSize());

	gaussianBlur = std::make_shared<GaussianBlur>(WinApp::Instance()->GetWinSize(), DXGI_FORMAT_R32G32B32A32_FLOAT);

	emissiveMap = D3D12App::Instance()->GenerateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT, Color(0.0f, 0.0f, 0.0f, 1.0f),
		WinApp::Instance()->GetWinSize(), L"EmissiveMap");

	sceneChange = new SceneCange();

	addValue = 10.0f;

	isSS = false;

	ResultSceneBackGround::Instance()->Init();
}

void GameScene::OnInitialize()
{
	ResultSceneBackGround::Instance()->Init();
}

void GameScene::OnUpdate()
{
	DebugParameter::Instance()->Update();
	game.Update();


	if (UsersInput::Instance()->OnTrigger(DIK_Q))
	{
		SuperiorityGauge::Instance()->AddPlayerGauge(addValue);
	}
	if (UsersInput::Instance()->OnTrigger(DIK_W))
	{
		SuperiorityGauge::Instance()->AddEnemyGauge(addValue);
	}
	if (UsersInput::Instance()->Input(DIK_S))
	{
		isSS = true;
	}
	else {
		isSS = false;
	}

	// リザルト画面へ飛ばす
	if (UsersInput::Instance()->OnTrigger(DIK_R)) {
		KuroEngine::Instance().ChangeScene(3, sceneChange);
	}

	bool changeInput = UsersInput::Instance()->OnTrigger(DIK_B) || UsersInput::Instance()->OnTrigger(START);
	if (changeInput)
	{
		KuroEngine::Instance().ChangeScene(1, sceneChange);
	}
}

void GameScene::OnDraw()
{
	emissiveMap->Clear(D3D12App::Instance()->GetCmdList());

	KuroEngine::Instance().Graphics().SetRenderTargets({ D3D12App::Instance()->GetBackBuffRenderTarget(),emissiveMap });
	backGround->Draw();
	game.Draw(emissiveMap);

	gaussianBlur->Register(emissiveMap);
	gaussianBlur->DrawResult(AlphaBlendMode_Add);

	// スクショを保存。
	if (isSS) {
		KuroEngine::Instance().Graphics().CopyTexture(ResultSceneBackGround::Instance()->backGround, D3D12App::Instance()->GetBackBuffRenderTarget());
	}

}

void GameScene::OnImguiDebug()
{
	//ImGui::Begin("StageInfo");
	//ImGui::Text("---Now---");
	//ImGui::Text("StageNumber:%d", SelectStage::Instance()->GetStageNum());
	//ImGui::Text("RoomNumber:%d\n\n", SelectStage::Instance()->GetRoomNum());
	//ImGui::Text("---Select---");

	//ImGui::Text("StageNumber:%d", game.debugStageData[0]);
	////選択できないステージ番号なら表示する
	//if (!StageMgr::Instance()->CheckStageNum(game.debugStageData[0]))
	//{
	//	ImGui::SameLine();
	//	ImGui::Text("Don't use");
	//}
	//if (game.nowSelectNum == 0)
	//{
	//	ImGui::SameLine();
	//	ImGui::Text("NowSelecting");
	//}


	//ImGui::Text("RoomNumber:%d", game.debugStageData[1]);
	////選択できない部屋番号なら表示する
	//if (!StageMgr::Instance()->CheckRoomNum(game.debugStageData[0], game.debugStageData[1]))
	//{
	//	ImGui::SameLine();
	//	ImGui::Text("Don't use");
	//}
	//if (game.nowSelectNum == 1)
	//{
	//	ImGui::SameLine();
	//	ImGui::Text("NowSelecting");
	//}

	//ImGui::Text("Up Down:SelectNumber");
	//ImGui::Text("Left Right:SelectStageOrRoom");
	//ImGui::Text("B Key / Home Button :StageSelectScene");
	//ImGui::Text("Return / A Button:Done");
	//ImGui::Text("Space / Back Button:Reset");
	//ImGui::End();


	/*ImGui::Begin("Timer");
	ImGui::Text("GravityMode");
	if (game.player.isZeroGravity)
	{
		ImGui::SameLine();
		ImGui::Text("  Now!!!");
	}
	ImGui::Text("NoGravityMode");
	if (!game.player.isZeroGravity)
	{
		ImGui::SameLine();
		ImGui::Text("Now!!!");
	}
	ImGui::Text("MaxTimer%d", game.player.CHANGE_GRAVITY_TIMER);
	ImGui::Text("NowTimer%d", game.player.changeGravityTimer);
	ImGui::End();*/


	SuperiorityGauge::Instance()->DebugValue(&addValue);

	//game.playerHomeBase->Debug();
	//game.enemyHomeBase->Debug();

	ImGui::Begin("Round");
	ImGui::Text("RoundNum%d", WinCounter::Instance()->GetNowRound());
	ImGui::Text("PlayerWin:%d,EnemyWin:%d", WinCounter::Instance()->GetLeftWin(), WinCounter::Instance()->GetRightWin());
	ImGui::End();

	DebugParameter::Instance()->DrawImGui();


	//GameTimer::Instance()->Debug();
	//ScoreManager::Instance()->Debug();
}

void GameScene::OnFinalize()
{
}
