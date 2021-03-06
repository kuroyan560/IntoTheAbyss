#include"../KuroEngine.h"
#include"IStageSelectImage.h"

// ステージ選択画面の矢印
class StageSelectArrow {

private:

	/*===== メンバ変数 =====*/

	LerpData pos;		// 座標など
	LerpData expData;	// 演出用
	Vec2<float> defPos;	// デフォルトの座標
	float angle;		// 回転させる角度
	int arrowHandle;	// 矢の画像ハンドル
	bool isZoomOut;		// ズームしているかのフラグ

	float timer;		// リサージュ曲線に使用するタイマー


public:

	/*===== メンバ変数 =====*/

	StageSelectArrow();
	void Init(const Vec2<float>& InitPos, const Vec2<float>& Pos, const float& Angle);
	void Update(const bool& isLeft);
	void Draw();

	void SetZoomOut(const bool& Zoom);
	void SetDefPos();
	void SetExitPos(const Vec2<float>& ExitPos, const Vec2<float>& ExitSize);
	void SetExpSize(const Vec2<float>& Size);

};