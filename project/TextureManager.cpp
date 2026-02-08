#include "TextureManager.h"
#include "DirectXCommon.h" 

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
void TextureManager::Initialize(DirectXCommon* dxCommon)
{
	// メンバ変数に保存 (これを忘れると LoadTexture でクラッシュします)
	this->dxCommon = dxCommon;

	// SRVの数と同数予約
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

// テクスチャ読み込み
void TextureManager::LoadTexture(const std::string& filePath)
{
	for (const auto& data : textureDatas)
	{
		// 重複していたら何もせず抜ける
		if (data.filePath == filePath)
		{
			return;
		}
	}
	assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);

	// --- 1. テクスチャファイルの読み込み ---
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));


	// --- 2. 管理リストに追加 ---
	textureDatas.resize(textureDatas.size() + 1);
	TextureData& textureData = textureDatas.back();

	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();

	// ★保存しておいた dxCommon を使ってリソース作成
	textureData.resource = dxCommon->CreateTextureResource(textureData.metadata);
	textureData.intermediateResource = dxCommon->UploadTextureData(textureData.resource.Get(), mipImages);

	// --- 3. SRV 生成 ---
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop; // ImGui用に+1

	textureData.srvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	dxCommon->GetDevice()->CreateShaderResourceView(
		textureData.resource.Get(),
		&srvDesc,
		textureData.srvHandleCPU
	);
}

// テクスチャのSRVハンドル取得 (GPU版)
D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
	// 読み込み済みのテクスチャデータ配列から検索
	for (const auto& data : textureDatas)
	{
		// パスが一致したら、そのハンドルを返す
		if (data.filePath == filePath)
		{
			return data.srvHandleGPU;
		}
	}

	assert(0);

	// コンパイルエラー回避用のダミー返却
	return D3D12_GPU_DESCRIPTOR_HANDLE{};
}

// テクスチャ番号の取得
uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath)
{
	// 読み込み済みテクスチャデータを検索
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](const TextureData& textureData) { return textureData.filePath == filePath; }
	);

	if (it != textureDatas.end())
	{
		// 読み込み済みなら要素番号を返す
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}

	assert(0);
	return 0;
}

// テクスチャ番号からGPUハンドルを取得
D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex)
{
	// 範囲外指定違反チェック
	// (配列の要素数より大きい番号を指定していないか)
	assert(textureIndex < textureDatas.size());

	// 指定された番号のテクスチャデータを取得
	TextureData& textureData = textureDatas[textureIndex];

	// そのデータが持っているGPUハンドルを返す
	return textureData.srvHandleGPU;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureIndex)
{
	//範囲外指定違反チェック
	assert(textureIndex < textureDatas.size());

	//テクスチャデータの参照を取得
	TextureData& textureData = textureDatas[textureIndex];
	return textureData.metadata;
}