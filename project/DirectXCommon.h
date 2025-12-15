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

using namespace Microsoft::WRL;

#include "WinApp.h"
#include "Logger.h"
#include "StringUtility.h"

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
	void CreateDepthSteencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);
	
	
	//void CreateDescriptorHeap();

	

	ID3D12CommandQueue* GetCommandQueue() const { return commandQueue.Get(); }
	ID3D12CommandAllocator* GetCommandAllocator() const { return commandAllocator.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }

	IDXGISwapChain4* GetSwapChain()const { return swapChain.Get(); }

private:
	//DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	//DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	//WindowsAPI
	WinApp* winApp = nullptr;

	//コマンド関連のメンバ変数
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;

	//スワップチェーンのメンバ変数
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> depthStencilResource;
}