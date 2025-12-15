#include "DirectXCommon.h"

void DirectXCommon::Initialize(WinApp* winApp)
{


	//NULL検出
	assert(winApp);

	//メンバ変数に記録
	this->winApp = winApp;


}

#pragma region デバイスの初期化
void DirectXCommon::DeviceInitialize()
{

	HRESULT hr;

	//デバックレイヤーをオンに
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();

		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif // _DEBUG

	//DXGIファクトリーの生成
	//DXGIFactoryの生成
	IDXGIFactory7* dxgiFactory = nullptr;
	//HRESULTはWindows系のエラーコードであり、
	//関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	//初期化の根本的な部分でエラーか出た場合はプログラムが間違っているか、
	// どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));


	// アダプターの列挙
	//使用するアダプタ(GPU)を決定する
	IDXGIAdapter4* useAdarter = nullptr;
	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdarter)) !=
		DXGI_ERROR_NOT_FOUND; ++i)
	{
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdarter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));//取得できないのは一大事
		//ソフトウェアアダプタでなければ採用！
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//採用したアダプタの情報をログに出力。wstringの方なので注意
			Logger::Log(
				StringUtility::ConvertString(
					std::format(
						L"Use Adapater:{}\n", adapterDesc.Description
					)
				)
			);
			break;
		}
		useAdarter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする
	}
	//適切なアダプタが見つからなかったので起動できない
	assert(useAdarter != nullptr);

	// デバイス生成
	ID3D12Device* device = nullptr;
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	for (size_t i = 0; i < _countof(featureLevels); ++i)
	{
		hr = D3D12CreateDevice(useAdarter, featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr))
		{
			Logger::Log(
				std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}

	assert(device != nullptr);


	// エラー時にブレークを発生させる設定
	Logger::Log("Complete create D3D12Device!!!\n");
}
#pragma endregion

#pragma region コマンド関連初期化
void DirectXCommon::CommandInitialize()
{
	HRESULT hr;

	//コマンドアロケータを生成
	commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケータを生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリストを生成
	commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, GetCommandAllocator(), nullptr, IID_PPV_ARGS(&commandList));
	//コマンドアリストを生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドキューを生成
	commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//コマンドキューを生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));
}
#pragma endregion

#pragma region スワップチェーンの生成
void DirectXCommon::SwapChainGenerate()
{
	//スワップチェーン生成
	swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::kClientWidth;
	swapChainDesc.Height = WinApp::kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	ComPtr<IDXGISwapChain1> tempSwapChain;
	HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(GetCommandQueue(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr, tempSwapChain.GetAddressOf());
	assert(SUCCEEDED(hr));
	hr=tempSwapChain.As(&swapChain); // 型をアップグレード
	assert(SUCCEEDED(hr));

	Logger::Log("Complete create SwapChain!!!\n");
};
#pragma endregion

#pragma region 深度バッファの生成
void DirectXCommon::CreateDepthSteencilTextureResource(ID3D12Device* device, int32_t width, int32_t height)
{
	//生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//Resourceの生成
	ID3D12Resource* depthStencilResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,//heapの設定
		D3D12_HEAP_FLAG_NONE,//heapの特殊な設定。特になし。
		&resourceDesc,//Resouceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度値を書き込む状態にしておく
		&depthClearValue,//Clear最適値
		IID_PPV_ARGS(&depthStencilResource));//作成するResouceポインタへのポインタ
	assert(SUCCEEDED(hr));
};
#pragma endregion

//void DirectXCommon::CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
//{
//	ID3D12DescriptorHeap* descriptorHeap = nullptr;
//	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
//	descriptorHeapDesc.Type = heapType;
//	descriptorHeapDesc.NumDescriptors = numDescriptors;
//	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
//	assert(SUCCEEDED(hr));
//	return descriptorHeap;
//
//}

