#pragma once
#include"Singleton.h"
#include<memory>
#include"Vec.h"
#include"Color.h"
#include"WinApp.h"

class VertexBuffer;
class ComputePipeline;
class GraphicsPipeline;
class ConstantBuffer;
class TextureBuffer;
class LightManager;

static const enum PARTICLE_TYPE { DASH, BULLET, CRASH_W, CRASH_G, CRASH_R, BOUND, CHIP_SPLINE_GENERATOR };

class ParticleMgr : public Singleton<ParticleMgr>
{
	static const int MAX_NUM = 1000;

	static const enum PARTICLE_CUMPUTE_TYPE { NORMAL_SMOKE, FAST_SMOKE, EMIT_STAR, SLIME_EXPLOSION, CHIP_OVERFLOW, DEFAULT = NORMAL_SMOKE };
	struct Particle
	{
		Color mulColor;
		Vec2<float> pos;	//座標
		Vec2<float>emitPos;
		Vec2<float>emitVec;	//放出ベクトル
		float speed;	//スピード
		float emitSpeed;	//放出直後のスピード
		float scale;	//スケール
		float emitScale;	//放出直後のスケール
		float radian;	//回転角度
		float emitRadian;	//放出直後の回転角度
		float alpha;	//アルファ値
		int life = 0;	//生成されてからのタイマー
		int lifeSpan;	//寿命
		char isAlive = 0;		// 生存フラグ
		unsigned int texIdx;	//テクスチャ番号
		unsigned int type = PARTICLE_CUMPUTE_TYPE::DEFAULT;	//パーティクル動き種別

		// 生成処理
		void Generate(const Vec2<float>& GeneratePos, const Vec2<float>& EmitVec, const int& Type, const int& TexIdx, const Color& MulColor);
	};
	struct GameInfo
	{
		float zoom = 0.0f;
		Vec2<float>scroll = { 0.0f,0.0f };
		float gameSpeed = 1.0f;
	};

	friend class Singleton<ParticleMgr>;
	ParticleMgr();

	//次に生成するパーティクルのインデックス
	int idx = 0;
	//パーティクルインスタンス
	Particle particles[MAX_NUM];
	//パーティクルの情報格納バッファ、描画に利用
	std::shared_ptr<VertexBuffer>buff;
	//パーティクルの更新を行うコンピュートパイプライン
	std::shared_ptr<ComputePipeline>cPipeline;
	//描画用パイプライン
	std::shared_ptr<GraphicsPipeline>gPipeline;

	//ズームとスクロールを送信するためのバッファ
	std::shared_ptr<ConstantBuffer>zoomAndScroll;

	//テクスチャは同じサイズである必要がある
	static const int TEX_SIZE = 64;
	static const int SMOKE_NUM = 4;
	static const int CHIP_NUM = 3;
	static const enum PARTICLE_TEX { WHITE, SMOKE_0, SMOKE_1, SMOKE_2, SMOKE_3, STAR, SLIME, CHIP_0, CHIP_1, CHIP_2, TEX_NUM = 10 };
	std::shared_ptr<TextureBuffer>textures[TEX_NUM];

	void EmitParticle(const Vec2<float>& EmitPos, const Vec2<float>& EmitVec, const int& Type, const int& TexIdx, const Color& MulColor = Color());

public:
	void Init();
	void Update();
	void Draw();

	// 生成処理
	void Generate(const Vec2<float>& EmitPos, const Vec2<float>& EmitVec, const PARTICLE_TYPE& Type);
};