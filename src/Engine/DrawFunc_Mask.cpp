#include "DrawFunc_Mask.h"
#include"KuroEngine.h"

//DrawExtendGraph
int DrawFunc_Mask::DRAW_EXTEND_GRAPH_COUNT = 0;

//DrawRotaGraph
int DrawFunc_Mask::DRAW_ROTA_GRAPH_COUNT = 0;

int DrawFunc_Mask::DRAW_BY_MASK_GRAPH_COUNT = 0;

static std::vector<RootParam>ROOT_PARAMETER =
{
	RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "平行投影行列定数バッファ"),
	RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "テクスチャリソース"),
};

void DrawFunc_Mask::DrawGraph(const Vec2<float>& LeftUpPos, const std::shared_ptr<TextureBuffer>& Tex, const Vec2<float>& MaskLeftUpPos, const Vec2<float>& MaskRightBottomPos, const Vec2<bool>& Miror, const float& MaskAlpha)
{
	DrawExtendGraph2D(LeftUpPos, LeftUpPos + Tex->GetGraphSize().Float(), Tex, MaskLeftUpPos, MaskRightBottomPos, Miror, MaskAlpha);
}

void DrawFunc_Mask::DrawExtendGraph2D(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const std::shared_ptr<TextureBuffer>& Tex, const Vec2<float>& MaskLeftUpPos, const Vec2<float>& MaskRightBottomPos, const Vec2<bool>& Miror, const float& MaskAlpha)
{
	class ExtendGraphVertex
	{
	public:
		Vec2<float>leftUpPos;
		Vec2<float>rightBottomPos;
		Vec2<float>maskLeftUpPos;
		Vec2<float>maskRightBottomPos;
		Vec2<int> miror;
		float maskAlpha;	//範囲外の描画アルファ値
		ExtendGraphVertex(const Vec2<float>& LeftUpPos, const Vec2<float>& RightBottomPos, const Vec2<float>& MaskLeftUpPos, const Vec2<float>& MaskRightBottomPos, const Vec2<bool>& Miror, const float& MaskAlpha)
			:leftUpPos(LeftUpPos), rightBottomPos(RightBottomPos), maskLeftUpPos(MaskLeftUpPos), maskRightBottomPos(MaskRightBottomPos), miror({ Miror.x ? 1 : 0 ,Miror.y ? 1 : 0 }), maskAlpha(MaskAlpha) {}
	};

	static std::shared_ptr<GraphicsPipeline>EXTEND_GRAPH_PIPELINE;
	static std::vector<std::shared_ptr<VertexBuffer>>EXTEND_GRAPH_VERTEX_BUFF;

	//パイプライン未生成
	if (!EXTEND_GRAPH_PIPELINE)
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		PIPELINE_OPTION.depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.vs = D3D12App::Instance()->CompileShader("resource/HLSL/Engine/DrawExtendGraph_Mask.hlsl", "VSmain", "vs_5_0");
		SHADERS.gs = D3D12App::Instance()->CompileShader("resource/HLSL/Engine/DrawExtendGraph_Mask.hlsl", "GSmain", "gs_5_0");
		SHADERS.ps = D3D12App::Instance()->CompileShader("resource/HLSL/Engine/DrawExtendGraph_Mask.hlsl", "PSmain", "ps_5_0");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("POSITION_L_U",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("POSITION_R_B",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("MASK_POS_LU",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("MASK_POS_RB",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("MIROR",DXGI_FORMAT_R32G32_SINT),
			InputLayoutParam("MASK_ALPHA",DXGI_FORMAT_R32_FLOAT),
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans) };
		//パイプライン生成
		EXTEND_GRAPH_PIPELINE = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, WrappedSampler(true, false));
	}

	KuroEngine::Instance().Graphics().SetPipeline(EXTEND_GRAPH_PIPELINE);

	if (EXTEND_GRAPH_VERTEX_BUFF.size() < (DRAW_EXTEND_GRAPH_COUNT + 1))
	{
		EXTEND_GRAPH_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(ExtendGraphVertex), 1, nullptr, ("DrawExtendGraph_Mask -" + std::to_string(DRAW_EXTEND_GRAPH_COUNT)).c_str()));
	}

	ExtendGraphVertex vertex(LeftUpPos, RightBottomPos, MaskLeftUpPos, MaskRightBottomPos, Miror, MaskAlpha);
	EXTEND_GRAPH_VERTEX_BUFF[DRAW_EXTEND_GRAPH_COUNT]->Mapping(&vertex);

	KuroEngine::Instance().Graphics().ObjectRender(EXTEND_GRAPH_VERTEX_BUFF[DRAW_EXTEND_GRAPH_COUNT],
		{ KuroEngine::Instance().GetParallelMatProjBuff(),Tex }, { CBV,SRV }, 0.0f, true);

	DRAW_EXTEND_GRAPH_COUNT++;
}

void DrawFunc_Mask::DrawRotaGraph2D(const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian, const std::shared_ptr<TextureBuffer>& Tex, const Vec2<float>& MaskCenterPos, const Vec2<float>& MaskSize, const Vec2<float>& RotaCenterUV, const Vec2<bool>& Miror, const float& MaskAlpha)
{
	//DrawRotaGraph専用頂点
	class RotaGraphVertex
	{
	public:
		Vec2<float>center;
		Vec2<float> extRate;
		float radian;
		Vec2<float>maskCenterPos;
		Vec2<float>maskSize;
		Vec2<float>rotaCenterUV;
		Vec2<int> miror;
		float maskAlpha;	//範囲外の描画アルファ値
		RotaGraphVertex(const Vec2<float>& Center, const Vec2<float>& ExtRate, const float& Radian,
			const Vec2<float>& MaskCenterPos, const Vec2<float>& MaskSize,
			const Vec2<float>& RotaCenterUV, const Vec2<bool>& Miror, const float& MaskAlpha)
			:center(Center), extRate(ExtRate), radian(Radian),
			maskCenterPos(MaskCenterPos), maskSize(MaskSize),
			rotaCenterUV(RotaCenterUV), miror({ Miror.x ? 1 : 0,Miror.y ? 1 : 0 }), maskAlpha(MaskAlpha) {}
	};

	static std::shared_ptr<GraphicsPipeline>ROTA_GRAPH_PIPELINE;
	static std::vector<std::shared_ptr<VertexBuffer>>ROTA_GRAPH_VERTEX_BUFF;

	//パイプライン未生成
	if (!ROTA_GRAPH_PIPELINE)
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		PIPELINE_OPTION.depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.vs = D3D12App::Instance()->CompileShader("resource/HLSL/Engine/DrawRotaGraph_Mask.hlsl", "VSmain", "vs_5_0");
		SHADERS.gs = D3D12App::Instance()->CompileShader("resource/HLSL/Engine/DrawRotaGraph_Mask.hlsl", "GSmain", "gs_5_0");
		SHADERS.ps = D3D12App::Instance()->CompileShader("resource/HLSL/Engine/DrawRotaGraph_Mask.hlsl", "PSmain", "ps_5_0");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("CENTER",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("EXT_RATE",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("RADIAN",DXGI_FORMAT_R32_FLOAT),
			InputLayoutParam("MASK_CENTER",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("MASK_SIZE",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("ROTA_CENTER_UV",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("MIROR",DXGI_FORMAT_R32G32_SINT),
			InputLayoutParam("MASK_ALPHA",DXGI_FORMAT_R32_FLOAT),
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans) };
		//パイプライン生成
		ROTA_GRAPH_PIPELINE = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, WrappedSampler(true, false));
	}

	KuroEngine::Instance().Graphics().SetPipeline(ROTA_GRAPH_PIPELINE);

	if (ROTA_GRAPH_VERTEX_BUFF.size() < (DRAW_ROTA_GRAPH_COUNT + 1))
	{
		ROTA_GRAPH_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(RotaGraphVertex), 1, nullptr, ("DrawRotaGraph_Mask -" + std::to_string(DRAW_ROTA_GRAPH_COUNT)).c_str()));
	}

	RotaGraphVertex vertex(Center, ExtRate, Radian, MaskCenterPos, MaskSize, RotaCenterUV, Miror, MaskAlpha);
	ROTA_GRAPH_VERTEX_BUFF[DRAW_ROTA_GRAPH_COUNT]->Mapping(&vertex);

	KuroEngine::Instance().Graphics().ObjectRender(ROTA_GRAPH_VERTEX_BUFF[DRAW_ROTA_GRAPH_COUNT],
		{ KuroEngine::Instance().GetParallelMatProjBuff(),Tex }, { CBV,SRV }, 0.0f, true);

	DRAW_ROTA_GRAPH_COUNT++;
}

#include"KuroMath.h"
void DrawFunc_Mask::DrawLine2DGraph(const Vec2<float>& FromPos, const Vec2<float>& ToPos, const std::shared_ptr<TextureBuffer>& Tex, const int& Thickness, const Vec2<float>& MaskLeftUpPos, const Vec2<float>& MaskRightBottomPos, const Vec2<bool>& Mirror)
{
	float distance = FromPos.Distance(ToPos);
	Vec2<float> vec = (ToPos - FromPos).GetNormal();

	auto graphSize = Tex->GetGraphSize().Float();
	Vec2<float>expRate = { distance / graphSize.x,Thickness / graphSize.y };
	Vec2<float>centerPos = FromPos + vec * distance / 2;

	Vec2<float>maskSize = MaskRightBottomPos - MaskLeftUpPos;
	Vec2<float>maskCenterPos = KuroMath::Liner(MaskLeftUpPos, MaskRightBottomPos, 0.5f);

	DrawRotaGraph2D(centerPos, expRate, KuroFunc::GetAngle(vec), Tex, maskCenterPos,maskSize, { 0.5f,0.5f }, Mirror);
}

void DrawFunc_Mask::DrawGraphByMaskGraph(const Vec2<float>& Center, const std::shared_ptr<TextureBuffer>& Tex, const Vec2<float>& MaskCenter, const std::shared_ptr<TextureBuffer>& MaskTex, const Vec2<float>& MaskExp, const Vec2<bool>& Mirror)
{
	//DrawGraphByMaskGraph専用頂点
	class Vertex
	{
	public:
		Vec2<float>center;
		Vec2<float>maskCenterPos;
		Vec2<float>maskExp;
		Vec2<int>mirror;
		Vertex(const Vec2<float>& Center, const Vec2<float>& MaskCenterPos, const Vec2<float>& MaskExp, const Vec2<bool>& Mirror)
			:center(Center), maskCenterPos(MaskCenterPos), maskExp(MaskExp), mirror({ Mirror.x ? 1 : 0,Mirror.y ? 1 : 0 }) {}
	};

	static std::shared_ptr<GraphicsPipeline>ROTA_GRAPH_PIPELINE;
	static std::vector<std::shared_ptr<VertexBuffer>>ROTA_GRAPH_VERTEX_BUFF;
	static std::vector<RootParam>ROOT_PARAMETER =
	{
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, "平行投影行列定数バッファ"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "テクスチャリソース"),
		RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, "マスクテクスチャリソース"),
	};


	//パイプライン未生成
	if (!ROTA_GRAPH_PIPELINE)
	{
		//パイプライン設定
		static PipelineInitializeOption PIPELINE_OPTION(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		PIPELINE_OPTION.depthTest = false;

		//シェーダー情報
		static Shaders SHADERS;
		SHADERS.vs = D3D12App::Instance()->CompileShader("resource/HLSL/Engine/DrawGraphByMaskGraph.hlsl", "VSmain", "vs_5_0");
		SHADERS.gs = D3D12App::Instance()->CompileShader("resource/HLSL/Engine/DrawGraphByMaskGraph.hlsl", "GSmain", "gs_5_0");
		SHADERS.ps = D3D12App::Instance()->CompileShader("resource/HLSL/Engine/DrawGraphByMaskGraph.hlsl", "PSmain", "ps_5_0");

		//インプットレイアウト
		static std::vector<InputLayoutParam>INPUT_LAYOUT =
		{
			InputLayoutParam("CENTER",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("MASK_CENTER",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("MASK_EXP",DXGI_FORMAT_R32G32_FLOAT),
			InputLayoutParam("MIRROR",DXGI_FORMAT_R32G32_SINT),
		};

		//レンダーターゲット描画先情報
		std::vector<RenderTargetInfo>RENDER_TARGET_INFO = { RenderTargetInfo(D3D12App::Instance()->GetBackBuffFormat(), AlphaBlendMode_Trans) };
		//パイプライン生成
		ROTA_GRAPH_PIPELINE = D3D12App::Instance()->GenerateGraphicsPipeline(PIPELINE_OPTION, SHADERS, INPUT_LAYOUT, ROOT_PARAMETER, RENDER_TARGET_INFO, WrappedSampler(false, false));
	}

	KuroEngine::Instance().Graphics().SetPipeline(ROTA_GRAPH_PIPELINE);

	if (ROTA_GRAPH_VERTEX_BUFF.size() < (DRAW_BY_MASK_GRAPH_COUNT + 1))
	{
		ROTA_GRAPH_VERTEX_BUFF.emplace_back(D3D12App::Instance()->GenerateVertexBuffer(sizeof(Vertex), 1, nullptr, ("DrawGraphByMaskGraph -" + std::to_string(DRAW_BY_MASK_GRAPH_COUNT)).c_str()));
	}

	Vertex vertex(Center, MaskCenter, MaskExp, Mirror);
	ROTA_GRAPH_VERTEX_BUFF[DRAW_BY_MASK_GRAPH_COUNT]->Mapping(&vertex);

	KuroEngine::Instance().Graphics().ObjectRender(ROTA_GRAPH_VERTEX_BUFF[DRAW_BY_MASK_GRAPH_COUNT],
		{ KuroEngine::Instance().GetParallelMatProjBuff(),Tex,MaskTex }, { CBV,SRV,SRV }, 0.0f, true);

	DRAW_BY_MASK_GRAPH_COUNT++;
}
