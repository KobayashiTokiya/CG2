#pragma once
#include <string>
#include <vector>
#include "externals/DirectXTex/DirectXTex.h"
#include <wrl.h>
#include "externals/DirectXTex/d3dx12.h"
#include <algorithm>
#include <unordered_map>

// 前方宣言
class DirectXCommon;
class SrvManager;

// テクスチャマネージャー
class TextureManager
{
public:
	// シングルトンインスタンスの取得
	static TextureManager* GetInstance();
	// 終了
	void Finalize();

	// 初期化
	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

	// テクスチャファイルの読み込み
	void LoadTexture(const std::string& filePath);

	// テクスチャのSRVハンドル(GPU用)を取得する
	//D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

	//SRVインデックスの開始番号
	//uint32_t GetTextureIndexByFilePath(const std::string& filePath);

	//テクスチャ番号からGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

	//メタデータを取得
	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);
	//SRVインデックスの取得
	uint32_t GetSrvIndex(const std::string& filePath);
	//GPUハンドルの取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

private:
	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

	DirectXCommon* dxCommon = nullptr;
	SrvManager* srvManager = nullptr;
private:
	// テクスチャ1枚分のデータ
	struct TextureData
	{
		std::string filePath;
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};

	// テクスチャデータ
	std::unordered_map<std::string,TextureData> textureDatas;

	//SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;


};