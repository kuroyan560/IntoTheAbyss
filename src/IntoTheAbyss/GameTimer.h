#pragma once
#include"../Common/Vec.h"
#include<vector>
#include"../Common/Singleton.h"
#include<array>

/// <summary>
/// ゲームの全体の時間を計算します
/// </summary>
class GameTimer :public Singleton<GameTimer>
{
	bool interruput;
public:
	GameTimer();

	/// <summary>
	/// 時間の描画座標を初期化します
	/// </summary>
	/// <param name="POS">時間の描画座標</param>
	void Init(int TIME);
	void Finalize();
	void Update();
	void Draw();

	/// <summary>
	/// 時間を計ります
	/// </summary>
	void Start();

	/// <summary>
	/// フレーム単位で時間を渡します
	/// </summary>
	/// <returns>フレーム数</returns>
	int GetFlame();

	/// <summary>
	/// 秒単位で時間を渡します
	/// </summary>
	/// <returns>秒数</returns>
	int GetSeceond();

	/// <summary>
	/// 時間切れを知らせます
	/// </summary>
	/// <returns>true...時間切れ,false...時間切れじゃない</returns>
	bool TimeUpFlag();

	/// <summary>
	/// カウントダウンが終了した判定を渡します
	/// </summary>
	/// <returns>true...カウントダウン終了,false...終了していない</returns>
	bool StartGame();

	/// <summary>
	/// 時間計測の一時中断フラグセット
	/// </summary>
	void SetInterruput(const bool& Flg) { interruput = Flg; }


	bool FinishAllEffect();


	void Debug();

private:
	Vec2<float> timerPos;

	std::vector<int> number;

	int timer;
	float flame;
	bool startFlag;
	bool timeUpFlag;

	std::vector<int> minitueHandle;
	std::vector<int> timeHandle;
	std::vector<int> miriHandle;

	float countDownEndPos;
	Vec2<int> texSize;
	int countDownNum;

	bool countDownFlag;
	int countDownHandle[4], finishHandle;

	int finishTimer;

	bool counDownFinishFlag;

	std::array<int, 5> countDownSE;

	std::vector<int> CountNumber(int TIME);

	// UIのサイズのオフセット
	const float OFFSET_SIZE = 0.98;
	float timerSize;
	int timerAlpha;

	bool isLessThan5SecondsLeft;	// 残り五秒だよフラグ

	// 残り五秒以下のときに真ん中に出すカウントダウン用の変数
	float centerCountDownSize;
	int centerCoundDownAlpha;

};

