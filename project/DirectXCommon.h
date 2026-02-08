#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <cassert>

//ファイルやディレクトリに関する操作を行うライブラリ
#include <filesystem>
//ファイルに書いたり読んだりするライブラリ
//#include <fstream>
//時間を扱うライブラリ
//#include <chrono>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#include <dxcapi.h>

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

using namespace Microsoft::WRL;

#include "WinApp.h"
#include "Logger.h"
#include "StringUtility.h"
#include "externals/DirectXTex/DirectXTex.h"

#include <chrono>
#include <thread>

class DirectXCommon
{
public://メンバ関数
	//初期化
	void Initialize(WinApp* winApp);

	//デバイスの初期化
	void DeviceInitialize();

	//コマンド関連の初期化
	void CommandInitialize();

	//スワップチェーンの生成
	void SwapChainGenerate();

	//深度バッファ
	void CreateDepthSteencilTextureResource();

	//各種デスクリプタヒープ
	void CreatingVariousDescriptorTeaps();

	//デスクリプタヒープを生成する
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE headType, UINT numDescriptors, bool shaderVisible);

	//レンダ―ターゲットビューの初期化
	void RenderTargetViewInitializing();

	//深度ステンシルビューの初期化
	void DepthStencilViewInitializing();

	//フェンス生成
	void FenceInitializing();

	//ビューポート矩形の初期化
	void ViewportInitializing();

	//シザリング矩形の初期化
	void ScissorRectInitializing();

	//DXCコンパイラの生成
	void DXCCompilerGeneration();

	//ImGuiの初期化
	void ImGuiInitializing();

	//getter
	ID3D12CommandQueue* GetCommandQueue() const { return commandQueue_.Get(); }
	ID3D12CommandAllocator* GetCommandAllocator() const { return commandAllocator_.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
	ID3D12Device* GetDevice() const { return device_.Get(); }

	IDXGISwapChain4* GetSwapChain()const { return swapChain_.Get(); }

	//シェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile);

	//バッファリソースの生成
	Microsoft::WRL::ComPtr<ID3D12Resource>CreateBufferResource(size_t sizeInBytes);

	//テクスチャリソースの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	//リソース転送関数・テクスチャデータの転送
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImage);


	ID3D12DescriptorHeap* GetSRVDescriptorHeap()const { return srvDscriptorHeap_.Get(); }

#pragma region 公開用の関数(宣言)
	//SRVの指定番号のCPUデスクリプタハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	//SRVの指定番号のGPUデスクリプタハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

	//RTVの指定番号のCPUデスクリプタハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriptorHandle(uint32_t index);
	//RTVの指定番号のGPUデスクリプタハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriptorHandle(uint32_t index);

	//DSVの指定番号のCPUデスクリプタハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	//DSVの指定番号のGPUデスクリプタハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);
#pragma endregion

	//最大SRV数(最大テクスチャ枚数)
	static constexpr uint32_t kMaxSRVCount = 512;

private:
	//DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	//DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;

	//WindowsAPI
	WinApp* winApp = nullptr;

	//コマンド関連のメンバ変数
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;

	//スワップチェーンのメンバ変数
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};

	//RTV_format
	//D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;

	//デスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap>rtvDescriptorHeap_;
	ComPtr<ID3D12DescriptorHeap>srvDscriptorHeap_;
	ComPtr<ID3D12DescriptorHeap>dsvDescriptorHeap_;

	//各種デスクリプタサイズをメンバ変数にする
	uint32_t descriptorSizeRTV_ = 0;
	uint32_t descriptorSizeSRV_ = 0;
	uint32_t descriptorSizeDSV_ = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];

	// バックバッファ用
	ComPtr<ID3D12Resource> renderTargets_[2];

	//指定番号のCPUデスクリプタハンドルを取得
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	//指定番号のGPUデスクリプタハンドルを取得
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);



	//スワップチェーンリソース
	//std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	//フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t fenceValue_ = 0;

	HANDLE fenceEvent_ = nullptr;

	//ビューポート矩形
	D3D12_VIEWPORT viewport_{};

	//シザリング矩形
	D3D12_RECT scissorRect_{};

	//DXCコンパイラの生成
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr;
	Microsoft::WRL::ComPtr <IDxcCompiler3> dxcCompiler_ = nullptr;
	Microsoft::WRL::ComPtr <IDxcIncludeHandler> includeHandler_ = nullptr;

	//FPS固定初期化
	void InitializeFixFPS();

	//FPS固定更新
	void UpdateFixFPS();

	//記録時間(FPS固定用)
	std::chrono::steady_clock::time_point referece_;

public:
	//描画前処理
	void PreDraw();
	//描画後処理
	void PostDraw();
};
