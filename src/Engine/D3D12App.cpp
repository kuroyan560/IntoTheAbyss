#include "D3D12App.h"
#include"KuroFunc.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

D3D12App* D3D12App::INSTANCE = nullptr;
std::map<std::string, D3D12App::LoadLambda_t> D3D12App::loadLambdaTable;

void D3D12App::Initialize(const HWND& Hwnd, const Vec2<int>& ScreenSize, const bool& UseHDR, const Color& ClearValue, const bool& IsFullScreen)
{
//デバッグレイヤー
#ifdef _DEBUG
	//デバッグレイヤーをオンに
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
#endif

	HRESULT result;

	//グラフィックスボードのアダプタを列挙
	//DXGIファクトリーの生成
	result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	//アダプターの列挙用
	std::vector<ComPtr<IDXGIAdapter>>adapters;
	//ここに特定の名前を持つアダプターオブジェクトが入る
	ComPtr<IDXGIAdapter> tmpAdapter = nullptr;
	for (int i = 0; dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		result = dxgiFactory->EnumAdapters(i, &tmpAdapter);
		adapters.push_back(tmpAdapter);		//動的配列に追加する
	}
	for (int i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC adesc{};
		result = adapters[i]->GetDesc(&adesc);	//アダプターの情報を取得
		std::wstring strDesc = adesc.Description;	//アダプター名
		//Microsoft Basic Render Driver,Intel UHD Graphics を回避
		if (strDesc.find(L"Microsoft") == std::wstring::npos
			&& strDesc.find(L"Intel") == std::wstring::npos)
		{
			tmpAdapter = adapters[i];	//採用
			break;
		}
	}

	//デバイスの生成（Direct3D12の基本オブジェクト）
	//対応レベルの配列
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (int i = 0; i < _countof(levels); i++)
	{
		//採用したアダプターでデバイスを生成
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&device));
		if (result == S_OK)
		{
			//デバイスを生成できた時点でループを抜ける
			featureLevel = levels[i];
			break;
		}
	}

	descHeapCBV_SRV_UAV = std::make_unique<DescriptorHeap_CBV_SRV_UAV>(device);
	descHeapRTV = std::make_unique<DescriptorHeap_RTV>(device);
	descHeapDSV = std::make_unique<DescriptorHeap_DSV>(device);

	//コマンドアロケータを生成
	commandAllocators.resize(FRAME_BUFFER_COUNT);
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		result = device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&commandAllocators[i])
		);
		if (FAILED(result))
		{
			printf("コマンドアロケータの生成失敗\n");
			assert(0);
		}
	}
	//バッファの転送を行うためにコマンドリストを使うので準備
	commandAllocators[0]->Reset();

	//コマンドリストを生成（GPUに、まとめて命令を送るためのコマンドリストを生成する）
	result = device->CreateCommandList(
		0, 
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocators[0].Get(),
		nullptr,
		IID_PPV_ARGS(&commandList));

	//コマンドキューの生成（コマンドリストをGPUに順に実行させていく為の仕組みを生成する）
	//標準設定でコマンドキューを生成
	D3D12_COMMAND_QUEUE_DESC queueDesc{
	  D3D12_COMMAND_LIST_TYPE_DIRECT,
	  0,
	  D3D12_COMMAND_QUEUE_FLAG_NONE,
	  0
	};
	result = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

	// スワップチェインの生成
	{
		DXGI_SWAP_CHAIN_DESC1 scDesc{};
		scDesc.BufferCount = FRAME_BUFFER_COUNT;
		scDesc.Width = ScreenSize.x;
		scDesc.Height = ScreenSize.y;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		scDesc.SampleDesc.Count = 1;
		//scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;  // ディスプレイの解像度も変更する場合にはコメント解除。

		//HDR対応化
		bool useHDR = UseHDR;
		if (useHDR)
		{
			bool isDisplayHDR10 = false;
			UINT index = 0;
			ComPtr<IDXGIOutput>current;
			while (tmpAdapter->EnumOutputs(index, &current) != DXGI_ERROR_NOT_FOUND)
			{
				ComPtr<IDXGIOutput6>output6;
				current.As(&output6);

				DXGI_OUTPUT_DESC1 desc;
				output6->GetDesc1(&desc);
				isDisplayHDR10 |= desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
				++index;
			}
			if (!isDisplayHDR10)useHDR = false;
		}
		if (useHDR)scDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		else scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		backBuffFormat = scDesc.Format;	//バックバッファのフォーマットを保存しておく

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc{};
		fsDesc.Windowed = IsFullScreen ? FALSE : TRUE;
		fsDesc.RefreshRate.Denominator = 1000;
		fsDesc.RefreshRate.Numerator = 60317;
		fsDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fsDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;

		ComPtr<IDXGISwapChain1> swapchain1;
		result = dxgiFactory->CreateSwapChainForHwnd(
			commandQueue.Get(),
			Hwnd,
			&scDesc,
			&fsDesc,
			nullptr,
			&swapchain1);

		if (FAILED(result))
		{
			printf("スワップチェイン生成失敗\n");
			assert(0);
		}

		float clearValue[4] =
		{
			ClearValue.r,
			ClearValue.g,
			ClearValue.b,
			ClearValue.a
		};
		swapchain = std::make_unique<Swapchain>(device, swapchain1, *descHeapCBV_SRV_UAV, *descHeapRTV, useHDR, clearValue);
	}

	//画像ロードのラムダ式生成
	loadLambdaTable["sph"]
		= loadLambdaTable["spa"]
		= loadLambdaTable["bmp"]
		= loadLambdaTable["png"]
		= loadLambdaTable["jpg"]
		= [](const std::wstring& Path, TexMetadata* Meta, ScratchImage& Img)->HRESULT
	{
		return LoadFromWICFile(Path.c_str(), WIC_FLAGS_NONE, Meta, Img);
	};
	loadLambdaTable["tga"]
		= [](const std::wstring& Path, TexMetadata* Meta, ScratchImage& Img)->HRESULT
	{
		return LoadFromTGAFile(Path.c_str(), Meta, Img);
	};
	loadLambdaTable["dds"]
		= [](const std::wstring& Path, TexMetadata* Meta, ScratchImage& Img)->HRESULT
	{
		return LoadFromDDSFile(Path.c_str(), DDS_FLAGS_NONE, Meta, Img);
	};

	//画像を分割するパイプライン
	{
		//シェーダ
		auto cs = CompileShader("resource/HLSL/Engine/RectTexture.hlsl", "CSmain", "cs_5_0");
		//ルートパラメータ
		std::vector<RootParam>rootParams =
		{
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,"分割後の画像"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,"分割前の画像"),
			RootParam(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,"分割番号")
		};
		//パイプライン生成
		splitImgPipeline = GenerateComputePipeline(cs, rootParams, WrappedSampler(true, false));
	}
}

std::shared_ptr<VertexBuffer> D3D12App::GenerateVertexBuffer(const size_t& VertexSize, const int& VertexNum, void* InitSendData, const char* Name, const bool& RWStructuredBuff)
{
	//リソースバリア
	auto barrier = D3D12_RESOURCE_STATE_GENERIC_READ;

	//頂点バッファサイズ
	UINT sizeVB = static_cast<UINT>(VertexSize * VertexNum);

	D3D12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(sizeVB);
	if (RWStructuredBuff)
	{
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		//ヒーププロパティ
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		prop.CreationNodeMask = 1;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		prop.Type = D3D12_HEAP_TYPE_CUSTOM;
		prop.VisibleNodeMask = 1;
	}

	//頂点バッファ生成
	ComPtr<ID3D12Resource1>buff;
	auto hr = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		barrier,
		nullptr,
		IID_PPV_ARGS(&buff));

	if (FAILED(hr))ASSERT_MSG("頂点バッファの生成に失敗\n");

	//名前のセット
	if (Name != nullptr)buff->SetName(KuroFunc::GetWideStrFromStr(Name).c_str());

	//頂点バッファビュー作成
	D3D12_VERTEX_BUFFER_VIEW vbView;
	vbView.BufferLocation = buff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = VertexSize;

	//専用のクラスにまとめる
	std::shared_ptr<VertexBuffer>result;

	//読み取り専用構造化バッファを生成するか
	if (RWStructuredBuff)
	{
		//シェーダリソースビュー作成
		descHeapCBV_SRV_UAV->CreateUAV(device, buff, VertexSize, VertexNum);

		//ビューを作成した位置のディスクリプタハンドルを取得
		DescHandles handles(descHeapCBV_SRV_UAV->GetCpuHandleTail(), descHeapCBV_SRV_UAV->GetGpuHandleTail());

		result = std::make_shared<VertexBuffer>(buff, barrier, vbView, handles);
	}
	else
	{
		result = std::make_shared<VertexBuffer>(buff, barrier, vbView);
	}

	//初期化マッピング
	if (InitSendData != nullptr)result->Mapping(InitSendData);

	return result;
}

std::shared_ptr<IndexBuffer> D3D12App::GenerateIndexBuffer(const int& IndexNum, void* InitSendData, const char* Name, const DXGI_FORMAT& IndexFormat)
{
	//リソースバリア
	auto barrier = D3D12_RESOURCE_STATE_GENERIC_READ;

	size_t indexSize = 0;
	if (IndexFormat == DXGI_FORMAT_R32_UINT)indexSize = sizeof(unsigned int);
	else if (IndexFormat == DXGI_FORMAT_R16_UINT)indexSize = sizeof(unsigned short);
	else ASSERT_MSG("インデックスバッファを生成しようとしましたが、対応していないフォーマットです\n");

	//インデックスバッファサイズ
	UINT sizeIB = static_cast<UINT>(indexSize * IndexNum);

	//頂点バッファ生成
	ComPtr<ID3D12Resource1>buff;
	auto hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB),
		barrier,
		nullptr,
		IID_PPV_ARGS(&buff));

	if (FAILED(hr))ASSERT_MSG("インデックスバッファの生成に失敗\n");

	//名前のセット
	if (Name != nullptr)buff->SetName(KuroFunc::GetWideStrFromStr(Name).c_str());

	//インデックスバッファビュー作成
	D3D12_INDEX_BUFFER_VIEW ibView;
	ibView.BufferLocation = buff->GetGPUVirtualAddress();
	ibView.Format = IndexFormat;
	ibView.SizeInBytes = sizeIB;

	//専用のクラスにまとめる
	auto result = std::make_shared<IndexBuffer>(buff, barrier, ibView, indexSize);

	//初期化マッピング
	if (InitSendData != nullptr)result->Mapping(InitSendData);

	return result;
}

std::shared_ptr<ConstantBuffer> D3D12App::GenerateConstantBuffer(const size_t& DataSize, const int& ElementNum, void* InitSendData, const char* Name)
{
	//アライメントしたサイズ
	size_t alignmentSize = (static_cast<UINT>(DataSize * ElementNum) + 0xff) & ~0xff;

	//リソースバリア
	auto barrier = D3D12_RESOURCE_STATE_GENERIC_READ;

	//定数バッファ生成
	ComPtr<ID3D12Resource1>buff;
	auto hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(alignmentSize),
		barrier,
		nullptr,
		IID_PPV_ARGS(&buff));

	if (FAILED(hr))ASSERT_MSG("定数バッファの生成に失敗\n");

	//名前のセット
	if (Name != nullptr)buff->SetName(KuroFunc::GetWideStrFromStr(Name).c_str());

	//定数バッファビュー作成
	descHeapCBV_SRV_UAV->CreateCBV(device, buff->GetGPUVirtualAddress(), alignmentSize);

	//ビューを作成した位置のディスクリプタハンドルを取得
	DescHandles handles(descHeapCBV_SRV_UAV->GetCpuHandleTail(), descHeapCBV_SRV_UAV->GetGpuHandleTail());

	//専用の定数バッファクラスにまとめる
	std::shared_ptr<ConstantBuffer>result;
	result = std::make_shared<ConstantBuffer>(buff, barrier, handles, DataSize, ElementNum);

	//初期値マッピング
	if (InitSendData != nullptr)result->Mapping(InitSendData);

	return result;
}

std::shared_ptr<StructuredBuffer> D3D12App::GenerateStructuredBuffer(const size_t& DataSize, const int& ElementNum, void* InitSendData, const char* Name)
{
	//リソースバリア
	auto barrier = D3D12_RESOURCE_STATE_GENERIC_READ;

	//定数バッファ生成
	ComPtr<ID3D12Resource1>buff;
	auto hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(DataSize * ElementNum),
		barrier,
		nullptr,
		IID_PPV_ARGS(&buff));

	if (FAILED(hr))ASSERT_MSG("構造化バッファの生成に失敗\n");

	//名前のセット
	if (Name != nullptr)buff->SetName(KuroFunc::GetWideStrFromStr(Name).c_str());

	//シェーダリソースビュー作成
	descHeapCBV_SRV_UAV->CreateSRV(device, buff, DataSize, ElementNum);

	//ビューを作成した位置のディスクリプタハンドルを取得
	DescHandles handles(descHeapCBV_SRV_UAV->GetCpuHandleTail(), descHeapCBV_SRV_UAV->GetGpuHandleTail());

	//専用の構造化バッファクラスにまとめる
	std::shared_ptr<StructuredBuffer>result;
	result = std::make_shared<StructuredBuffer>(buff, barrier, handles, DataSize, ElementNum);

	//初期値マッピング
	if (InitSendData != nullptr)result->Mapping(InitSendData);

	return result;
}

std::shared_ptr<RWStructuredBuffer> D3D12App::GenerateRWStructuredBuffer(const size_t& DataSize, const int& ElementNum, const char* Name)
{
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(DataSize * ElementNum);
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	//リソースバリア
	auto barrier = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	//ヒーププロパティ
	D3D12_HEAP_PROPERTIES prop{};
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	prop.CreationNodeMask = 1;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	prop.Type = D3D12_HEAP_TYPE_CUSTOM;
	prop.VisibleNodeMask = 1;

	//定数バッファ生成
	ComPtr<ID3D12Resource1>buff;
	auto hr = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		barrier,
		nullptr,
		IID_PPV_ARGS(&buff));

	if (FAILED(hr))ASSERT_MSG("出力用バッファの生成に失敗\n");

	//名前のセット
	if (Name != nullptr)buff->SetName(KuroFunc::GetWideStrFromStr(Name).c_str());

	//シェーダリソースビュー作成
	descHeapCBV_SRV_UAV->CreateUAV(device, buff, DataSize, ElementNum);

	//ビューを作成した位置のディスクリプタハンドルを取得
	DescHandles handles(descHeapCBV_SRV_UAV->GetCpuHandleTail(), descHeapCBV_SRV_UAV->GetGpuHandleTail());

	//専用の構造化バッファクラスにまとめる
	std::shared_ptr<RWStructuredBuffer>result;
	result = std::make_shared<RWStructuredBuffer>(buff, barrier, handles, DataSize, ElementNum);

	return result;
}

std::shared_ptr<TextureBuffer> D3D12App::GenerateTextureBuffer(const Color& Color, const DXGI_FORMAT& Format, const int& Width)
{
	//既にあるか確認
	for (auto itr = colorTextures.begin(); itr != colorTextures.end(); ++itr)
	{
		if (itr->color == Color && itr->width == Width)
		{
			return itr->tex;
		}
	}

	//なかったので生成する
	const int texDataCount = Width * Width;

	//テクスチャデータ配列
	XMFLOAT4* texturedata = new XMFLOAT4[texDataCount];

	//全ピクセルの色を初期化
	for (int i = 0; i < texDataCount; ++i)
	{
		texturedata[i].x = Color.r;	//R
		texturedata[i].y = Color.g;	//G
		texturedata[i].z = Color.b;	//B
		texturedata[i].w = Color.a;	//A
	}

	//テクスチャヒープ設定
	D3D12_HEAP_PROPERTIES texHeapProp{};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	//テクスチャリソース設定
	D3D12_RESOURCE_DESC texDesc{};	//リソース設定
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	//2Dテクスチャ用
	texDesc.Format = Format;	//RGBAフォーマット
	texDesc.Width = Width;
	texDesc.Height = Width;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = 1;

	//リソースバリア
	auto barrier = D3D12_RESOURCE_STATE_GENERIC_READ;

	//テクスチャ用リソースバッファの生成
	ComPtr<ID3D12Resource1>buff;
	auto hr = device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		barrier,
		nullptr,
		IID_PPV_ARGS(&buff));

	if (FAILED(hr))ASSERT_MSG("単色塗りつぶしテクスチャバッファの生成に失敗\n");

	//バッファに名前セット
	std::wstring name = L"ColorTexture - ";
	name += std::to_wstring(Color.r) + L" , ";
	name += std::to_wstring(Color.g) + L" , ";
	name += std::to_wstring(Color.b) + L" , ";
	name += std::to_wstring(Color.a);
	buff->SetName(name.c_str());

	//テクスチャバッファにデータ転送
	hr = buff->WriteToSubresource(
		0,
		nullptr,	//全領域へコピー
		texturedata,	//元データアドレス
		sizeof(XMFLOAT4) * Width,	//1ラインサイズ
		sizeof(XMFLOAT4) * texDataCount	//１枚サイズ
	);
	delete[] texturedata;

	if (FAILED(hr))ASSERT_MSG("単色塗りつぶしテクスチャバッファへのデータ転送に失敗\n");

	//シェーダーリソースビュー作成
	descHeapCBV_SRV_UAV->CreateSRV(device, buff, Format);

	//ビューを作成した位置のディスクリプタハンドルを取得
	DescHandles handles(descHeapCBV_SRV_UAV->GetCpuHandleTail(), descHeapCBV_SRV_UAV->GetGpuHandleTail());

	//専用のシェーダーリソースクラスにまとめる
	std::shared_ptr<TextureBuffer>result;
	result = std::make_shared<TextureBuffer>(buff, barrier, handles, texDesc);
	
	//テクスチャ用のリソースバリアに変更
	result->ChangeBarrier(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	//作成したカラーテクスチャ情報を記録
	ColorTexture colorTexData;
	colorTexData.color = Color;
	colorTexData.width = Width;
	colorTexData.tex = result;
	colorTextures.emplace_back(colorTexData);

	return result;
}

std::shared_ptr<TextureBuffer> D3D12App::GenerateTextureBuffer(const std::string& LoadImgFilePath)
{
	//既にあるか確認
	for (auto itr = loadImgTextures.begin(); itr != loadImgTextures.end(); ++itr)
	{
		if (itr->path == LoadImgFilePath)
		{
			return itr->textures[0];
		}
	}

	TexMetadata metadata{};
	ScratchImage scratchImg{};

	//ワイド文字変換
	auto wtexpath = KuroFunc::GetWideStrFromStr(LoadImgFilePath);

	//拡張子取得
	auto ext = KuroFunc::GetExtension(LoadImgFilePath);

	//ロード
	auto hr = loadLambdaTable[ext](
		wtexpath,
		&metadata,
		scratchImg);
	if (FAILED(hr))ASSERT_MSG("画像データ抽出に失敗\n");

	const Image* img = scratchImg.GetImage(0, 0, 0);	//生データ抽出

	//テクスチャリソース設定
	CD3DX12_RESOURCE_DESC texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,	//RGBAフォーマット
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels);

	//リソースバリア
	auto barrier = D3D12_RESOURCE_STATE_GENERIC_READ;

	//テクスチャ用リソースバッファの生成
	ComPtr<ID3D12Resource1>buff;
	CD3DX12_HEAP_PROPERTIES heapPropForTex(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	hr = device->CreateCommittedResource(
		&heapPropForTex,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		barrier,
		nullptr,
		IID_PPV_ARGS(&buff));
	if (FAILED(hr))ASSERT_MSG("ロード画像テクスチャバッファ生成に失敗\n");

	//名前セット
	buff->SetName(wtexpath.c_str());

	//テクスチャバッファにデータ転送
	hr = buff->WriteToSubresource(
		0,
		nullptr,	//全領域へコピー
		img->pixels,	//元データアドレス
		(UINT)img->rowPitch,	//1ラインサイズ
		(UINT)img->slicePitch	//１枚サイズ
	);
	if (FAILED(hr))ASSERT_MSG("ロード画像テクスチャバッファへのデータ転送に失敗");

	//シェーダーリソースビュー作成
	descHeapCBV_SRV_UAV->CreateSRV(device, buff, metadata.format);

	//ビューを作成した位置のディスクリプタハンドルを取得
	DescHandles handles(descHeapCBV_SRV_UAV->GetCpuHandleTail(), descHeapCBV_SRV_UAV->GetGpuHandleTail());

	//専用のシェーダーリソースクラスにまとめる
	std::shared_ptr<TextureBuffer>result;
	result = std::make_shared<TextureBuffer>(buff, barrier, handles, texDesc);

	//テクスチャ用のリソースバリアに変更
	result->ChangeBarrier(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	//作成したカラーテクスチャ情報を記録
	LoadImgTexture loadImgTexData;
	loadImgTexData.path = LoadImgFilePath;
	loadImgTexData.textures = { result };
	loadImgTextures.emplace_back(loadImgTexData);

	return result;
}

std::shared_ptr<TextureBuffer> D3D12App::GenerateTextureBuffer(const Vec2<int>& Size, const DXGI_FORMAT& Format, const char* Name)
{
	//テクスチャリソース設定
	CD3DX12_RESOURCE_DESC texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		Format,	//RGBAフォーマット
		(UINT)Size.x,
		(UINT)Size.y,
		(UINT16)1,
		(UINT16)1);
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	//リソースバリア
	auto barrier = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	//テクスチャ用リソースバッファの生成
	ComPtr<ID3D12Resource1>buff;
	CD3DX12_HEAP_PROPERTIES heapPropForTex(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	auto hr = device->CreateCommittedResource(
		&heapPropForTex,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		barrier,
		nullptr,
		IID_PPV_ARGS(&buff));
	if (FAILED(hr))ASSERT_MSG("分割画像用テクスチャバッファ生成に失敗\n");

	//名前セット
	if (Name != nullptr)
	{
		buff->SetName(KuroFunc::GetWideStrFromStr(Name).c_str());
	}

	//シェーダーリソースビュー作成
	descHeapCBV_SRV_UAV->CreateSRV(device, buff, texDesc.Format);
	DescHandles srvHandles(descHeapCBV_SRV_UAV->GetCpuHandleTail(), descHeapCBV_SRV_UAV->GetGpuHandleTail());

	//アンオーダードアクセスビュー作成
	descHeapCBV_SRV_UAV->CreateUAV(device, buff, 4, texDesc.Width * texDesc.Height, D3D12_UAV_DIMENSION_TEXTURE2D, texDesc.Format);
	DescHandles uavHandles(descHeapCBV_SRV_UAV->GetCpuHandleTail(), descHeapCBV_SRV_UAV->GetGpuHandleTail());

	//専用のシェーダーリソースクラスにまとめる
	std::shared_ptr<TextureBuffer>result;
	result = std::make_shared<TextureBuffer>(buff, barrier, srvHandles, texDesc);
	result->SetUAVHandles(uavHandles);

	return result;
}

std::vector<std::shared_ptr<TextureBuffer>> D3D12App::GenerateTextureBuffer(const std::string& LoadImgFilePath, const int& AllNum, const Vec2<int>& SplitNum)
{
	auto sourceTexture = GenerateTextureBuffer(LoadImgFilePath);
	return SplitTextureBuffer(sourceTexture, AllNum, SplitNum, LoadImgFilePath);
}

void D3D12App::GenerateTextureBuffer(std::shared_ptr<TextureBuffer>* Array, const std::string& LoadImgFilePath, const int& AllNum, const Vec2<int>& SplitNum)
{
	auto sourceTexture = GenerateTextureBuffer(LoadImgFilePath);
	return SplitTextureBuffer(Array, sourceTexture, AllNum, SplitNum, LoadImgFilePath);
}

std::shared_ptr<RenderTarget> D3D12App::GenerateRenderTarget(const DXGI_FORMAT& Format, const Color& ClearValue, const Vec2<int> Size, const wchar_t* TargetName, D3D12_RESOURCE_STATES InitState, int MipLevel, int ArraySize)
{
	//レンダーターゲット設定
	CD3DX12_RESOURCE_DESC desc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		static_cast<UINT>(Size.x),
		static_cast<UINT>(Size.y),
		ArraySize,
		MipLevel,
		Format,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);

	//レンダーターゲットのクリア値
	float clearColor[4] =
	{
		ClearValue.r,
		ClearValue.g,
		ClearValue.b,
		ClearValue.a
	};

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = Format;
	for (int i = 0; i < 4; ++i)
	{
		clearValue.Color[i] = clearColor[i];
	}

	//リソース生成
	ComPtr<ID3D12Resource1>buff;
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto hr = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		InitState,
		&clearValue,
		IID_PPV_ARGS(&buff));
	if (FAILED(hr))ASSERT_MSG("レンダーターゲットバッファ生成に失敗\n");

	//名前セット
	if (TargetName != nullptr)buff->SetName(TargetName);

	//SRV作成
	descHeapCBV_SRV_UAV->CreateSRV(device, buff, Format);
	//ビューを作成した位置のディスクリプタハンドルを取得
	DescHandles srvHandles(descHeapCBV_SRV_UAV->GetCpuHandleTail(), descHeapCBV_SRV_UAV->GetGpuHandleTail());

	//RTV作成
	descHeapRTV->CreateRTV(device, buff);
	//ビューを作成した位置のディスクリプタハンドルを取得
	DescHandles rtvHandles(descHeapRTV->GetCpuHandleTail(), descHeapRTV->GetGpuHandleTail());

	//専用のレンダーターゲットクラスにまとめる
	std::shared_ptr<RenderTarget>result;
	result = std::make_shared<RenderTarget>(buff, InitState, srvHandles, rtvHandles, desc, clearColor);

	return result;
}

std::shared_ptr<DepthStencil> D3D12App::GenerateDepthStencil(const Vec2<int>& Size, const DXGI_FORMAT& Format, const float& ClearValue)
{
	//リソース設定
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
		Format,	//深度値フォーマット
		static_cast<UINT>(Size.x),
		static_cast<UINT>(Size.y),
		1, 0,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);	//デプスステンシル

	//リソースバリア
	auto barrier = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	//デプスステンシルバッファ生成
	ComPtr<ID3D12Resource1>buff;
	auto hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&CD3DX12_CLEAR_VALUE(Format, ClearValue, 0),
		IID_PPV_ARGS(&buff));
	if (FAILED(hr))ASSERT_MSG("デプスステンシルバッファ生成に失敗\n");

	//DSV作成
	descHeapDSV->CreateDSV(device, buff);
	//ビューを作成した位置のディスクリプタハンドルを取得
	DescHandles handles(descHeapDSV->GetCpuHandleTail(), descHeapDSV->GetGpuHandleTail());

	//専用のレンダーターゲットクラスにまとめる
	std::shared_ptr<DepthStencil>result;
	result = std::make_shared<DepthStencil>(buff, barrier, handles, desc, ClearValue);

	return result;
}

void D3D12App::SetDescHeaps()
{
	//ディスクリプタヒープをセット
	ID3D12DescriptorHeap* heaps[] = { descHeapCBV_SRV_UAV->GetHeap().Get() };
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);
}

void D3D12App::Render(D3D12AppUser* User)
{
	SetDescHeaps();

	//スワップチェイン表示可能からレンダーターゲット描画可能へ
	swapchain->GetBackBufferRenderTarget()->ChangeBarrier(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	//レンダーターゲットをクリア
	swapchain->GetBackBufferRenderTarget()->Clear(commandList);

	//レンダリング処理
	User->Render();

	//レンダーターゲットからスワップチェイン表示可能へ
	swapchain->GetBackBufferRenderTarget()->ChangeBarrier(commandList, D3D12_RESOURCE_STATE_PRESENT);

	//命令のクローズ
	auto hr = commandList->Close();
	if (FAILED(hr))ASSERT_MSG("コマンドリスト命令のクローズに失敗\n");

	//コマンドリストの実行
	ID3D12CommandList* cmdLists[] = { commandList.Get() };	//コマンドリストの配列
	commandQueue->ExecuteCommandLists(1, cmdLists);

	//バッファをフリップ（裏表の入れ替え）
	hr = swapchain->GetSwapchain()->Present(1, 0);
	if (FAILED(hr))ASSERT_MSG("バックバッファのフリップに失敗\n");

	//バックバッファ番号取得
	auto frameIdx = swapchain->GetSwapchain()->GetCurrentBackBufferIndex();

	//コマンドアロケータリセット
	hr = commandAllocators[frameIdx]->Reset();	//キューをクリア
	if (FAILED(hr))ASSERT_MSG("コマンドアロケータリセットに失敗\n");

	//コマンドリスト
	hr = commandList->Reset(commandAllocators[frameIdx].Get(), nullptr);		//コマンドリストを貯める準備
	if (FAILED(hr))ASSERT_MSG("コマンドリストのリセットに失敗\n");

	//コマンドリストの実行完了を待つ
	swapchain->WaitPreviousFrame(commandQueue, frameIdx);

	//SplitTexBuff呼ばれた回数リセット
	splitTexBuffCount = 0;
}

#include<d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")
Microsoft::WRL::ComPtr<ID3DBlob>D3D12App::CompileShader(const std::string& FilePath, const std::string& EntryPoint, const std::string& ShaderModel)
{
	ComPtr<ID3DBlob>result;
	ComPtr<ID3DBlob> errorBlob = nullptr;	//エラーオブジェクト

	//ワイド文字に変換
	const std::wstring wideFilePath = KuroFunc::GetWideStrFromStr(FilePath);

	//頂点シェーダの読み込みとコンパイル
	auto hr = D3DCompileFromFile(
		wideFilePath.c_str(),		//シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//インクルード可能にする
		EntryPoint.c_str(), ShaderModel.c_str(),	//エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//デバッグ用設定
		0,
		result.GetAddressOf(), &errorBlob);

	//シェーダのエラー内容を表示
	if (FAILED(hr))
	{
		//errorBlobからエラー内容string型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += '\n';
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}
	return result;
}

std::shared_ptr<GraphicsPipeline>D3D12App::GenerateGraphicsPipeline(
	const PipelineInitializeOption& Option,
	const Shaders& ShaderInfo,
	const std::vector<InputLayoutParam>& InputLayout,
	const std::vector<RootParam>& RootParams,
	const std::vector<RenderTargetInfo>& RenderTargetFormat,
	const WrappedSampler& SamplerDesc)
{
	HRESULT hr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};

	std::vector<D3D12_INPUT_ELEMENT_DESC>inputLayout;
	//インプットレイアウト
	{
		for (auto& param : InputLayout)
		{
			D3D12_INPUT_ELEMENT_DESC input =
			{
				param.semantics.c_str(),	//セマンティック名
				0,				//同じセマンティック名が複数あるときに使うインデックス（０でよい）
				param.format,	//要素数とビット数を表す（XYZの３つでfloat型なので R32G32B32_FLOAT)
				0,	//入力スロットインデックス（０でよい）
				D3D12_APPEND_ALIGNED_ELEMENT,	//データのオフセット値（D3D12_APPEND_ALIGNED_ELEMENTだと自動設定）
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,		//入力データ種別（標準はD3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA）
				0		//一度に描画するインスタンス数（０でよい）
			};
			inputLayout.emplace_back(input);
		}
		desc.InputLayout.pInputElementDescs = &inputLayout[0];
		desc.InputLayout.NumElements = inputLayout.size();
	}

	//ルートパラメータ
	ComPtr<ID3D12RootSignature>rootSignature;
	{
		std::vector<CD3DX12_ROOT_PARAMETER>rootParameters;
		std::vector< CD3DX12_DESCRIPTOR_RANGE>rangeArray;

		//各レンジタイプでレジスターがいくつ登録されたか
		int registerNum[D3D12_DESCRIPTOR_RANGE_TYPE_NUM] = { 0 };
		for (auto& param : RootParams)
		{
			//ディスクリプタとして初期化
			if (param.descriptor)
			{
				//タイプの取得
				auto& type = param.descriptorRangeType;

				//ディスクリプタレンジ初期化
				CD3DX12_DESCRIPTOR_RANGE range{};
				range.Init(type, 1, registerNum[(int)type]);

				registerNum[(int)type]++;
				rangeArray.emplace_back(range);
			}
			//ビューで初期化
			else
			{
				rootParameters.emplace_back();
				if (param.viewType == SRV)
				{
					auto type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					rootParameters.back().InitAsShaderResourceView(registerNum[(int)type]);
					registerNum[(int)type]++;
				}
				else if (param.viewType == CBV)
				{
					auto type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					rootParameters.back().InitAsConstantBufferView(registerNum[(int)type]);
					registerNum[(int)type]++;
				}
				else if (param.viewType == UAV)
				{
					auto type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					rootParameters.back().InitAsUnorderedAccessView(registerNum[(int)type]);
					registerNum[(int)type]++;
				}
				else
				{
					ASSERT_MSG("ルートパラメータが上手くいきませんでした\n");
				}
			}
		}

		for (int i = 0; i < rangeArray.size(); ++i)
		{
			rootParameters.emplace_back();
			rootParameters.back().InitAsDescriptorTable(1, &rangeArray[i]);
		}

		// ルートシグネチャの設定
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_0(rootParameters.size(), &rootParameters[0], 1, &SamplerDesc.sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> rootSigBlob;
		ComPtr<ID3DBlob> errorBlob = nullptr;	//エラーオブジェクト
		// バージョン自動判定のシリアライズ
		auto hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
		if (FAILED(hr))
		{
			//errorBlobからエラー内容string型にコピー
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());

			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += '\n';
			//エラー内容を出力ウィンドウに表示
			OutputDebugStringA(errstr.c_str());
			ASSERT_MSG("ルートシグネチャのバージョン自動判定のシリアライズ失敗\n");
		}

		// ルートシグネチャの生成
		hr = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

		//ルートシグネチャ生成に失敗
		if (FAILED(hr))ASSERT_MSG("ルートシグネチャの生成に失敗\n");
	}

	//グラフィックスパイプライン設定にルートシグネチャをセット
	desc.pRootSignature = rootSignature.Get();

	//グラフィックスパイプライン設定
	{
		//サンプルマスク
		desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定

		// ラスタライザステート
		desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		//カリングモード
		if (!Option.calling)
		{
			desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		}
		//ワイヤーフレーム
		if (Option.wireFrame)
		{
			desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		}
		//深度テスト
		if (Option.depthTest)
		{
			desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			//デプスの書き込みを禁止（深度テストは行う）
			if (!Option.depthWriteMask)
			{
				desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			}
		}
		//デプスステンシルバッファフォーマット
		desc.DSVFormat = Option.dsvFormat;

		desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		//同時レンダーターゲットで独立したブレンディングを有効にするか
		desc.BlendState.IndependentBlendEnable = Option.independetBlendEnable;

		// 1ピクセルにつき1回サンプリング
		desc.SampleDesc.Count = 1;

		// 図形の形状設定
		desc.PrimitiveTopologyType = Option.primitiveTopologyType;

		//書き込み先レンダーターゲット
		desc.NumRenderTargets = 0;
		for (auto& info : RenderTargetFormat)
		{
			int idx = ++desc.NumRenderTargets - 1;
			if (D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT < desc.NumRenderTargets)
			{
				ASSERT_MSG("描画先レンダーターゲットの数が最大を超えています\n");
			}

			//描画先レンダーターゲットのフォーマット
			desc.RTVFormats[idx] = info.format;

			//アルファブレンディング設定
			if (info.blendMode == AlphaBlendMode_Trans)	//半透明合成
			{
				desc.BlendState.RenderTarget[idx].BlendEnable = true;
				desc.BlendState.RenderTarget[idx].SrcBlend = D3D12_BLEND_SRC_ALPHA;
				desc.BlendState.RenderTarget[idx].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				desc.BlendState.RenderTarget[idx].BlendOp = D3D12_BLEND_OP_ADD;
			}
			else if (info.blendMode == AlphaBlendMode_Add)	//加算合成
			{
				//加算合成のブレンドステート作成
				desc.BlendState.RenderTarget[idx].BlendEnable = true;
				desc.BlendState.RenderTarget[idx].SrcBlend = D3D12_BLEND_ONE;
				desc.BlendState.RenderTarget[idx].DestBlend = D3D12_BLEND_ONE;
				desc.BlendState.RenderTarget[idx].BlendOp = D3D12_BLEND_OP_ADD;
			}
			else desc.BlendState.RenderTarget[idx].BlendEnable = false;	//完全上書き
		}
	}

	//シェーダーオブジェクトセット
	if(ShaderInfo.vs.Get())desc.VS = CD3DX12_SHADER_BYTECODE(ShaderInfo.vs.Get());
	if(ShaderInfo.ps.Get())desc.PS = CD3DX12_SHADER_BYTECODE(ShaderInfo.ps.Get());
	if(ShaderInfo.ds.Get())desc.DS = CD3DX12_SHADER_BYTECODE(ShaderInfo.ds.Get());
	if(ShaderInfo.hs.Get())desc.HS = CD3DX12_SHADER_BYTECODE(ShaderInfo.hs.Get());
	if(ShaderInfo.gs.Get())desc.GS = CD3DX12_SHADER_BYTECODE(ShaderInfo.gs.Get());

	//グラフィックスパイプラインの生成
	ComPtr<ID3D12PipelineState>pipeline;
	hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipeline));
	if (FAILED(hr))ASSERT_MSG("グラフィックスパイプライン生成に失敗\n");

	return std::make_shared<GraphicsPipeline>(pipeline, rootSignature, Option.primitiveTopology);
}

std::shared_ptr<ComputePipeline> D3D12App::GenerateComputePipeline(const ComPtr<ID3DBlob>& ComputeShader, const std::vector<RootParam>& RootParams, const WrappedSampler& Sampler)
{
	HRESULT hr;

	// パイプラインステートを作成
	D3D12_COMPUTE_PIPELINE_STATE_DESC  desc = { 0 };

	//ルートパラメータ
	ComPtr<ID3D12RootSignature>rootSignature;
	{
		std::vector<CD3DX12_ROOT_PARAMETER>rootParameters;
		std::vector< CD3DX12_DESCRIPTOR_RANGE>rangeArray;

		//各レンジタイプでレジスターがいくつ登録されたか
		int registerNum[D3D12_DESCRIPTOR_RANGE_TYPE_NUM] = { 0 };
		for (auto& param : RootParams)
		{
			//ディスクリプタとして初期化
			if (param.descriptor)
			{
				//タイプの取得
				auto& type = param.descriptorRangeType;

				//ディスクリプタレンジ初期化
				CD3DX12_DESCRIPTOR_RANGE range{};
				range.Init(type, 1, registerNum[(int)type]);

				registerNum[(int)type]++;
				rangeArray.emplace_back(range);
			}
			//ビューで初期化
			else
			{
				rootParameters.emplace_back();
				if (param.viewType == SRV)
				{
					auto type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					rootParameters.back().InitAsShaderResourceView(registerNum[(int)type]);
					registerNum[(int)type]++;
				}
				else if (param.viewType == CBV)
				{
					auto type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					rootParameters.back().InitAsConstantBufferView(registerNum[(int)type]);
					registerNum[(int)type]++;
				}
				else if (param.viewType == UAV)
				{
					auto type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					rootParameters.back().InitAsUnorderedAccessView(registerNum[(int)type]);
					registerNum[(int)type]++;
				}
				else
				{
					ASSERT_MSG("ルートパラメータが上手くいきませんでした\n");
				}
			}
		}

		for (int i = 0; i < rangeArray.size(); ++i)
		{
			rootParameters.emplace_back();
			rootParameters.back().InitAsDescriptorTable(1, &rangeArray[i]);
		}

		// ルートシグネチャの設定
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_0(rootParameters.size(), &rootParameters[0], 1, &Sampler.sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> rootSigBlob;
		ComPtr<ID3DBlob> errorBlob = nullptr;	//エラーオブジェクト
		// バージョン自動判定のシリアライズ
		auto hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
		if (FAILED(hr))
		{
			//errorBlobからエラー内容string型にコピー
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());

			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += '\n';
			//エラー内容を出力ウィンドウに表示
			OutputDebugStringA(errstr.c_str());
			ASSERT_MSG("ルートシグネチャのバージョン自動判定のシリアライズ失敗\n");
		}

		// ルートシグネチャの生成
		hr = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

		//ルートシグネチャ生成に失敗
		if (FAILED(hr))ASSERT_MSG("ルートシグネチャの生成に失敗\n");
	}

	//グラフィックスパイプライン設定にルートシグネチャをセット
	desc.pRootSignature = rootSignature.Get();

	desc.CS = CD3DX12_SHADER_BYTECODE(ComputeShader.Get());
	desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	desc.NodeMask = 0;

	ComPtr<ID3D12PipelineState>pipeline;
	hr = device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipeline));
	if (FAILED(hr))ASSERT_MSG("コンピュートパイプライン生成に失敗\n");

	return std::make_shared<ComputePipeline>(pipeline, rootSignature);
}

void D3D12App::SetViewPortsAndScissorRects(const Vec2<float>& Size)
{
	//ビューポート設定
	commandList->RSSetViewports(1, &CD3DX12_VIEWPORT(0.0f, 0.0f, Size.x, Size.y));

	//シザー矩形設定
	commandList->RSSetScissorRects(1, &CD3DX12_RECT(0, 0, Size.x, Size.y));
}

void D3D12App::SetBackBufferRenderTarget()
{
	swapchain->GetBackBufferRenderTarget()->ChangeBarrier(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> BACK_BUFF_HANDLE = { swapchain->GetBackBufferRenderTarget()->AsRTV(commandList) };
	commandList->OMSetRenderTargets(BACK_BUFF_HANDLE.size(), &BACK_BUFF_HANDLE[0], FALSE, nullptr);
}

void D3D12App::SplitTextureBuffer(std::shared_ptr<TextureBuffer>* Array, const std::shared_ptr<TextureBuffer>& SorceTexBuffer, const int& AllNum, const Vec2<int>& SplitNum, const std::string& Name)
{
	//ディスクリプタヒープをセット
	ID3D12DescriptorHeap* heaps[] = { descHeapCBV_SRV_UAV->GetHeap().Get() };
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	splitImgPipeline->SetPipeline(commandList);
	SorceTexBuffer->SetComputeDescriptorBuffer(commandList, SRV, 1);

	SplitImgConstData constData;

	//分割前のサイズを記録
	constData.splitSize = { static_cast<int>(SorceTexBuffer->GetDesc().Width) / SplitNum.x,static_cast<int>(SorceTexBuffer->GetDesc().Height) / SplitNum.y };

	for (int i = 0; i < AllNum; ++i)
	{
		if (splitImgConstBuff.size() < splitTexBuffCount + 1)
		{
			std::string name = "SplitImgConstBuff - " + std::to_string(i);
			splitImgConstBuff.emplace_back(GenerateConstantBuffer(sizeof(SplitImgConstData), 1, nullptr, name.c_str()));
		}

		//描き込み先用のテクスチャバッファ
		auto splitResult = GenerateTextureBuffer(constData.splitSize, SorceTexBuffer->GetDesc().Format, (Name + " - " + std::to_string(i)).c_str());

		splitResult->SetComputeDescriptorBuffer(commandList, UAV, 0);

		//splitImgConstBuff[i]->Mapping(&constData);
		//splitImgConstBuff[i]->SetComputeDescriptorBuffer(commandList, CBV, 2);
		splitImgConstBuff[splitTexBuffCount]->Mapping(&constData);
		splitImgConstBuff[splitTexBuffCount]->SetComputeDescriptorBuffer(commandList, CBV, 2);

		static const int THREAD_NUM = 8;
		const UINT threadX = constData.splitSize.x / THREAD_NUM;
		const UINT threadY = constData.splitSize.y / THREAD_NUM;
		commandList->Dispatch(threadX, threadY, 1);

		//テクスチャ用のリソースバリアに変更
		splitResult->ChangeBarrier(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		Array[i] = splitResult;

		constData.imgNum.x++;
		if (SplitNum.x <= constData.imgNum.x)
		{
			constData.imgNum.x = 0;
			constData.imgNum.y++;
		}

		splitTexBuffCount++;
	}
}

std::vector<std::shared_ptr<TextureBuffer>> D3D12App::SplitTextureBuffer(const std::shared_ptr<TextureBuffer>& SorceTexBuffer, const int& AllNum, const Vec2<int>& SplitNum, const std::string& Name)
{
	std::vector<std::shared_ptr<TextureBuffer>>result(AllNum);
	SplitTextureBuffer(&result[0], SorceTexBuffer, AllNum, SplitNum, Name);
	return result;
}