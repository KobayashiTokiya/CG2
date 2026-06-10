#include "TextureManager.h"
#include "DirectXCommon.h" 
#include "SrvManager.h"

// static変数の実体化
TextureManager* TextureManager::instance = nullptr;

//ImGuiで0番を使用するため、1番から使用
uint32_t TextureManager::kSRVIndexTop = 1;

// シングルトンインスタンスの取得
TextureManager* TextureManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new TextureManager;
	}
	return instance;
}

// 終了処理
void TextureManager::Finalize()
{
	delete instance;
	instance = nullptr;
}

// 初期化
// ★重要変更：ここで DirectXCommon のポインタを受け取って保存します
void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
	// メンバ変数に保存 (これを忘れると LoadTexture でクラッシュします)
	this->dxCommon = dxCommon;
	this->srvManager = srvManager;

	// SRVの数と同数予約
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

// テクスチャ読み込み
void TextureManager::LoadTexture(const std::string& filePath)
{
	//読み込み済みテクスチャを検索
	if (textureDatas.contains(filePath))
	{
		return;
	}
	//テクスチャ枚数上限チェック
	assert(srvManager->CanAllocate());

	// --- 1. テクスチャファイルの読み込み ---
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr;
	if (filePathW.ends_with(L".dds"))
	{
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	}
	else
	{
		hr= DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	assert(SUCCEEDED(hr));

	DirectX::ScratchImage mipImages{};
	if (DirectX::IsCompressed(image.GetMetadata().format))
	{
		mipImages = std::move(image);
	}
	else 
	{
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	}
	assert(SUCCEEDED(hr));


	// --- 2. 管理リストに追加 ---
	TextureData& textureData = textureDatas[filePath];

	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();

	//リソース作成
	textureData.resource = dxCommon->CreateTextureResource(textureData.metadata);
	textureData.intermediateResource = dxCommon->UploadTextureData(textureData.resource.Get(), mipImages);

	// --- 3. SRV 生成 ---
	textureData.srvIndex = srvManager->Allocate();

	textureData.srvHandleCPU = srvManager->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager->GetGPUDescriptorHandle(textureData.srvIndex);

	if (textureData.metadata.IsCubemap())
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = textureData.metadata.format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = UINT(textureData.metadata.mipLevels);
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

		dxCommon->GetDevice()->CreateShaderResourceView(
			textureData.resource.Get(),
			&srvDesc,
			textureData.srvHandleCPU
		);
	}
	else
	{
		srvManager->CreateSRVforTexture2D(
			textureData.srvIndex,
			textureData.resource.Get(),
			textureData.metadata.format,
			UINT(textureData.metadata.mipLevels)
		);
	}
}

// テクスチャのSRVハンドル取得 (GPU版)
D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
	//読み込み済みかチェック
	assert(textureDatas.contains(filePath));

	return textureDatas[filePath].srvHandleGPU;
}

// テクスチャ番号の取得
uint32_t TextureManager::GetSrvIndex(const std::string& filePath)
{
	//読み込み済みかチェック
	assert(textureDatas.contains(filePath));

	//srvIndexを返す
	return textureDatas[filePath].srvIndex;
}

// テクスチャ番号からGPUハンドルを取得
D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex)
{
	return srvManager->GetGPUDescriptorHandle(textureIndex);
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
	//範囲外指定違反チェック
	assert(textureDatas.contains(filePath));

	return textureDatas[filePath].metadata;
}