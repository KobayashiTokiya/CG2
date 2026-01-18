#include "DirectXCommon.h"

void DirectXCommon::Initialize(WinApp* winApp)
{
	//NULL検出
	assert(winApp);

	//メンバ変数に記録
	this->winApp = winApp;

	DeviceInitialize();
	CommandInitialize();
	SwapChainGenerate();
	CreatingVariousDescriptorTeaps();
	CreateDepthSteencilTextureResource();

	RenderTargetViewInitializing();
	DepthStencilViewInitializing();
	FenceInitializing();
	ViewportInitializing();
	ScissorRectInitializing();
	DXCCompilerGeneration();
	ImGuiInitializing();
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
	//HRESULTはWindows系のエラーコードであり、
	//関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	//初期化の根本的な部分でエラーか出た場合はプログラムが間違っているか、
	// どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));


	// アダプターの列挙
	//使用するアダプタ(GPU)を決定する
	IDXGIAdapter4* useAdarter = nullptr;
	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i,
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
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	for (size_t i = 0; i < _countof(featureLevels); ++i)
	{
		hr = D3D12CreateDevice(useAdarter, featureLevels[i], IID_PPV_ARGS(&device_));
		if (SUCCEEDED(hr))
		{
			Logger::Log(
				std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}

	assert(device_ != nullptr);


	// エラー時にブレークを発生させる設定
	Logger::Log("Complete create D3D12Device!!!\n");
}
#pragma endregion

#pragma region コマンド関連初期化
void DirectXCommon::CommandInitialize()
{
	HRESULT hr;

	//コマンドアロケータを生成
	commandAllocator_ = nullptr;
	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	//コマンドアロケータを生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリストを生成
	commandList_ = nullptr;
	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, GetCommandAllocator(), nullptr, IID_PPV_ARGS(&commandList_));
	//コマンドアリストを生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドキューを生成
	commandQueue_ = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
	//コマンドキューを生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));
}
#pragma endregion

#pragma region スワップチェーンの生成
void DirectXCommon::SwapChainGenerate()
{
	//スワップチェーン生成
	swapChain_ = nullptr;
	swapChainDesc_;
	swapChainDesc_.Width = WinApp::kClientWidth;
	swapChainDesc_.Height = WinApp::kClientHeight;
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc_.SampleDesc.Count = 1;
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc_.BufferCount = 2;
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	ComPtr<IDXGISwapChain1> tempSwapChain;
	HRESULT hr = dxgiFactory_->CreateSwapChainForHwnd(GetCommandQueue(), winApp->GetHwnd(), &swapChainDesc_, nullptr, nullptr, tempSwapChain.GetAddressOf());
	assert(SUCCEEDED(hr));
	hr = tempSwapChain.As(&swapChain_); // 型をアップグレード
	assert(SUCCEEDED(hr));

	Logger::Log("Complete create SwapChain!!!\n");
};
#pragma endregion

#pragma region 深度バッファの生成
void DirectXCommon::CreateDepthSteencilTextureResource()
{

	//生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = winApp->kClientWidth;
	resourceDesc.Height = winApp->kClientHeight;
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
	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties,//heapの設定
		D3D12_HEAP_FLAG_NONE,//heapの特殊な設定。特になし。
		&resourceDesc,//Resouceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度値を書き込む状態にしておく
		&depthClearValue,//Clear最適値
		IID_PPV_ARGS(&depthStencilResource_));//作成するResouceポインタへのポインタ
	assert(SUCCEEDED(hr));
};
#pragma endregion

#pragma region 各種デスクリプタヒープの生成
void DirectXCommon::CreatingVariousDescriptorTeaps()
{
	assert(device_);

	//DescriptorSizeを取得しておく
	descriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeDSV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//DSV用のヒープでディスクリプタの数は１。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	rtvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	srvDscriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	dsvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	// DescriptorSizeを取得
	assert(rtvDescriptorHeap_);
	assert(dsvDescriptorHeap_);
}
#pragma endregion

#pragma region レンダーターゲットビューの設定
void DirectXCommon::RenderTargetViewInitializing()
{
	// レンダ―ターゲットビューの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// RTVハンドルの要素を２個に変更する
	const uint32_t backBufferCount = 2;

	//RTVヒープの先頭ハンドル(cpu)
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU =
		rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

	UINT rtvDescriptorSize =
		device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//裏表に２つ分
	for (uint32_t i = 0; i < backBufferCount; i++)
	{
		HRESULT hr = swapChain_->GetBuffer(
			i, IID_PPV_ARGS(&renderTargets_[i]));
		assert(SUCCEEDED(hr));

		rtvHandles_[i] = rtvHandleCPU;
		rtvHandles_[i].ptr += i * rtvDescriptorSize;

		//レンダ―ターゲットビューの生成
		device_->CreateRenderTargetView(
			renderTargets_[i].Get(),// RTVを生成したいリソース
			&rtvDesc,				// RTVの設定
			rtvHandles_[i]	// RTVを書き込むディスクリプタハンドル
		);
	}
}
#pragma endregion

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	return D3D12_CPU_DESCRIPTOR_HANDLE();
}
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	return D3D12_GPU_DESCRIPTOR_HANDLE();
}

#pragma region SRVに特化した公開用の関数(実装)

//SRV専用のデスクリプタハンドル取得関数を作成
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(srvDscriptorHeap_, descriptorSizeSRV_, index);
}
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(srvDscriptorHeap_, descriptorSizeSRV_, index);
}
#pragma endregion
#pragma region RTVに特化した公開用の関数(実装)
//RTV専用のデスクリプタハンドル取得関数を作成
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetRTVCPUDescriptorHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(srvDscriptorHeap_, descriptorSizeSRV_, index);
}
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetRTVGPUDescriptorHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(srvDscriptorHeap_, descriptorSizeSRV_, index);
}
#pragma endregion
#pragma region DSVに特化した公開用の関数(実装)
//DSV専用のデスクリプタハンドル取得関数を作成
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetDSVCPUDescriptorHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(srvDscriptorHeap_, descriptorSizeSRV_, index);
}
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetDSVGPUDescriptorHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(srvDscriptorHeap_, descriptorSizeSRV_, index);
}
#pragma endregion

#pragma region 深度ステンシルビューの初期化
void DirectXCommon::DepthStencilViewInitializing()
{
	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//DSVHeapの先頭にDSVをつくる
	device_->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc, dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());
}
#pragma endregion

#pragma region フェンス生成
void DirectXCommon::FenceInitializing()
{
	HRESULT hr;

	//初期値0でFenceを作る
	fence_ = nullptr;
	hr = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));

	//FenceのSignalを待つためのイベントを作成する
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);
}
#pragma endregion

#pragma region ビューポート矩形の初期化
void DirectXCommon::ViewportInitializing()
{
	//ビューポート矩形の設定
	viewport_.Width = WinApp::kClientWidth;
	viewport_.Height = WinApp::kClientHeight;
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
}
#pragma endregion

#pragma region シザリング矩形の初期化
void DirectXCommon::ScissorRectInitializing()
{
	scissorRect_.left = 0;
	scissorRect_.right = WinApp::kClientWidth;
	scissorRect_.top = 0;
	scissorRect_.bottom = WinApp::kClientHeight;
}
#pragma endregion

#pragma region DXCコンパイラの生成
void DirectXCommon::DXCCompilerGeneration()
{
	HRESULT hr;

	//DXCユーティリティの生成
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));

	//DXCコンパイラの生成
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	//デフォルトインクルードハンドラの生成
	//現時点でincludeはしないが、includeに対応するための設定を行っておく
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}
#pragma endregion

#pragma region ImGuiの初期化
void DirectXCommon::ImGuiInitializing()
{
	//バージョンチェック
	IMGUI_CHECKVERSION();

	//コンテキストの生成
	ImGui::CreateContext();

	//スタイルの設定
	ImGui::StyleColorsDark();

	//Win32用の初期化
	ImGui_ImplWin32_Init(winApp->GetHwnd());

	//RTVフォーマット
	DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//DirectX12の初期化
	ImGui_ImplDX12_Init(
		device_.Get(),
		swapChainDesc_.BufferCount,
		rtvFormat,
		srvDscriptorHeap_.Get(),
		srvDscriptorHeap_->GetCPUDescriptorHandleForHeapStart(),
		srvDscriptorHeap_->GetGPUDescriptorHandleForHeapStart());
}
#pragma endregion

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>
DirectXCommon::CreateDescriptorHeap(
	D3D12_DESCRIPTOR_HEAP_TYPE heapType,
	UINT numDescriptors,
	bool shaderVisible)
{
	assert(device_);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = heapType;
	desc.NumDescriptors = numDescriptors;
	desc.Flags = shaderVisible
		? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		: D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr =
		device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));

	return descriptorHeap;
}

//描画前処理
void DirectXCommon::PreDraw()
{

	//バックバッファの番号取得
		//書き込むバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();
	
	// リソースバリアの書き込み可能に変更
	//TransitionBarrerの設定
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTargets_[backBufferIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier);
	
	//描画先のRTVのDSVを指定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

	commandList_->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, &dsvHandle);

	// 画面全体の色をクリア
	//指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	commandList_->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearColor, 0, nullptr);
	
	// 画面全体の深度をクリア
	//指定した深度で画面全体をクリアする
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// SRV用のデスクリプタヒープを指定する
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvDscriptorHeap_.Get()};
	commandList_->SetDescriptorHeaps(1, descriptorHeaps);
	
	// ビューポート領域の設定
	commandList_->RSSetViewports(1, &viewport_);

	// シザー矩形の設定
	commandList_->RSSetScissorRects(1, &scissorRect_);
}

//描画後処理
void DirectXCommon::PostDraw()
{
	// バックバッファの番号取得
	//書き込むバックバッファのインデックスを取得
	UINT bbIndex = swapChain_->GetCurrentBackBufferIndex();

	// リソースバリアで表示状態に変更
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTargets_[bbIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource =
		D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList_->ResourceBarrier(1, &barrier);

	// グラフィックスコマンドをクローズ
	HRESULT hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	// GPUコマンドの実行
	ID3D12CommandList* commandLists[] = { commandList_.Get()};
	commandQueue_->ExecuteCommandLists(1, commandLists);

	// GPU画面の交換を通知
	swapChain_->Present(1, 0);

	// Fenceの値を更新
	fenceValue_++;

	// コマンドキューにシグナルを送る
	commandQueue_->Signal(fence_.Get(), fenceValue_);

	// コマンド完了待ち
	if (fence_->GetCompletedValue() < fenceValue_)
	{
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		//イベント待つ
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	// コマンドアロケーターのリセット
	hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));

	// コマンドリストのリセット
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));
	//
}


//CompileShader関数
Microsoft::WRL::ComPtr<IDxcBlob>
DirectXCommon::CompileShader(
	const std::wstring& filePath,
	const wchar_t* profile)
{
	Log(Logger::GetStream(),
		ConvertString(std::format(
			L"Begin CompileShader,path:{},profile:{}\n",
			filePath, profile)));

	// 1. hlsl 読み込み
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource;
	HRESULT hr =
		dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));

	DxcBuffer buffer{};
	buffer.Ptr = shaderSource->GetBufferPointer();
	buffer.Size = shaderSource->GetBufferSize();
	buffer.Encoding = DXC_CP_UTF8;

	// 2. Compile
	LPCWSTR arguments[] = {
		filePath.c_str(),
		L"-E", L"main",
		L"-T", profile,
		L"-Zi", L"-Qembed_debug",
		L"-Od",
		L"-Zpr",
	};

	Microsoft::WRL::ComPtr<IDxcResult> result;
	hr = dxcCompiler_->Compile(
		&buffer,
		arguments,
		_countof(arguments),
		includeHandler_.Get(),
		IID_PPV_ARGS(&result));
	assert(SUCCEEDED(hr));

	// 3. エラーチェック
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> error;
	result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&error), nullptr);
	if (error && error->GetStringLength() != 0)
	{
		Log(Logger::GetStream(), error->GetStringPointer());
		assert(false);
	}

	// 4. バイナリ取得
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
	hr = result->GetOutput(
		DXC_OUT_OBJECT,
		IID_PPV_ARGS(&shaderBlob),
		nullptr);
	assert(SUCCEEDED(hr));

	Log(Logger::GetStream(),
		ConvertString(std::format(
			L"Compile Succeeded,path:{},profile:{}\n",
			filePath, profile)));

	return shaderBlob;
}