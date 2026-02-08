#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dbghelp.h>
#include <strsafe.h>
#include <dxgidebug.h>

#include <dxcapi.h>
#include <vector>
#include <sstream>
//コムポインタ
#include <wrl.h>

#include <numbers>


#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

#include "Matrix.h"
#include "Vector.h"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM IParam);


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
//スプライト共通部
#include "SpriteCommon.h"
//スプライト
#include "Sprite.h"


#pragma region コメントアウト（構造体・関数）

/*
//ヴェクター４を作る
struct Vector4
{
	float x, y, z, w;
};

struct Matrix3x3
{
	float m[3][3];
};

struct VertexData
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

//TransformationMatrix
struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct Material
{
	Vector4 color;
	int32_t enableLighting;
	float padding[3]; // パディング（サイズ調整）
	Matrix4x4 uvTransform;
};

VertexData vertices[6] = {
	{  0.0f,  0.5f, 0.0f, 1.0f },
	{  0.5f, -0.5f, 0.0f, 1.0f },
	{ -0.5f, -0.5f, 0.0f, 1.0f },
};
*/
#pragma endregion


// ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
	{
		return true;
	}

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


// メイン関数
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	//ポインタ
	WinApp* winApp = nullptr;

	//WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

	//ポインタ
	DirectXCommon* dxCommon = nullptr;
	//DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	SpriteCommon* spriteCommon = nullptr;
	//スプライト共通部の初期化
	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);

	//入力クラスの初期化
	Input* input = new Input();
	input->Initialize(winApp);

#pragma region コメントアウト（古い初期化コード）
	/*
	ID3D12Device* device = dxCommon->GetDevice();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	//RootSignature作成
		D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
		descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
		descriptorRange[0].BaseShaderRegister = 0;
		descriptorRange[0].NumDescriptors = 1;
		descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


		//RootParameter作成
		D3D12_ROOT_PARAMETER rootParamenters[4] = {};
		rootParamenters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParamenters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParamenters[0].Descriptor.ShaderRegister = 0;

		rootParamenters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParamenters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParamenters[1].Descriptor.ShaderRegister = 0;

		rootParamenters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParamenters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParamenters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
		rootParamenters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

		rootParamenters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParamenters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParamenters[3].Descriptor.ShaderRegister = 1;

		descriptionRootSignature.pParameters = rootParamenters;
		descriptionRootSignature.NumParameters = _countof(rootParamenters);



		D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
		staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
		staticSamplers[0].ShaderRegister = 0;
		staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		descriptionRootSignature.pStaticSamplers = staticSamplers;
		descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


		ID3DBlob* signatureBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;

		HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
			D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);


		hr = D3D12SerializeRootSignature(&descriptionRootSignature,
			D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
		if (FAILED(hr))
		{
			assert(false);
		}

		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;

		// GPU上にルートシグネチャを作成
		hr = device->CreateRootSignature(
			0,
			signatureBlob->GetBufferPointer(),
			signatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&rootSignature));

		assert(SUCCEEDED(hr));

		// 使い終わったBlobは解放する (ComPtrでない場合)
		if (signatureBlob) signatureBlob->Release();
		if (errorBlob) errorBlob->Release();



	//インプットレイアウト
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//struct VertexShaderInput
	//{
	//	float32_t4 position : POSITION0;
	//};

	//BlendState
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


	//RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(FRONT)(NONE)
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//ShaderをCompileする

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob =dxCommon->CompileShader(L"Resource/shaders/Object3d.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob =dxCommon->CompileShader(L"Resource/shaders/Object3d.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される。
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	//VertexResource
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC vertexResourceDesc{};

	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(VertexData) * 6;//リソースのサイズ。今回はVector4を3頂点

	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 1. 四角形の頂点データ定義
	VertexData vertices[6];
	// 左下
	vertices[0].position = { -0.5f, -0.5f, 0.5f, 1.0f }; // Zを 0.5f に
	vertices[0].texcoord = { 0.0f, 1.0f };
	vertices[0].normal = { 0.0f, 0.0f, -1.0f };
	// 左上
	vertices[1].position = { -0.5f,  0.5f, 0.5f, 1.0f }; // Zを 0.5f に
	vertices[1].texcoord = { 0.0f, 0.0f };
	vertices[1].normal = { 0.0f, 0.0f, -1.0f };
	// 右下
	vertices[2].position = { 0.5f, -0.5f, 0.5f, 1.0f }; // Zを 0.5f に
	vertices[2].texcoord = { 1.0f, 1.0f };
	vertices[2].normal = { 0.0f, 0.0f, -1.0f };

	// --- 2枚目の三角形 ---
	// 左上
	vertices[3].position = { -0.5f,  0.5f, 0.5f, 1.0f }; // Zを 0.5f に
	vertices[3].texcoord = { 0.0f, 0.0f };
	vertices[3].normal = { 0.0f, 0.0f, -1.0f };
	// 右上
	vertices[4].position = { 0.5f,  0.5f, 0.5f, 1.0f }; // Zを 0.5f に
	vertices[4].texcoord = { 1.0f, 0.0f };
	vertices[4].normal = { 0.0f, 0.0f, -1.0f };
	// 右下
	vertices[5].position = { 0.5f, -0.5f, 0.5f, 1.0f }; // Zを 0.5f に
	vertices[5].texcoord = { 1.0f, 1.0f };
	vertices[5].normal = { 0.0f, 0.0f, -1.0f };

	// 頂点リソースを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite;

	// ヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProp{};
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	// リソース設定
	D3D12_RESOURCE_DESC vertexResDesc{};
	vertexResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResDesc.Alignment = 0;
	vertexResDesc.Width = sizeof(vertices);
	vertexResDesc.Height = 1;
	vertexResDesc.DepthOrArraySize = 1;
	vertexResDesc.MipLevels = 1;
	vertexResDesc.Format = DXGI_FORMAT_UNKNOWN;
	vertexResDesc.SampleDesc.Count = 1;
	vertexResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vertexResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr = device->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&vertexResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexResourceSprite));
	assert(SUCCEEDED(hr));

	// データを書き込む
	VertexData* vertexDataBegin = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataBegin));
	std::memcpy(vertexDataBegin, vertices, sizeof(vertices));
	vertexResourceSprite->Unmap(0, nullptr);

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite;

	// リソース設定（頂点バッファと同じように作ります）
	D3D12_HEAP_PROPERTIES heapPropMat{};
	heapPropMat.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resDescMat{};
	resDescMat.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDescMat.Width = sizeof(TransformationMatrix); // サイズは行列構造体分
	resDescMat.Height = 1;
	resDescMat.DepthOrArraySize = 1;
	resDescMat.MipLevels = 1;
	resDescMat.SampleDesc.Count = 1;
	resDescMat.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// リソースを実際に作成
	hr = device->CreateCommittedResource(
		&heapPropMat,
		D3D12_HEAP_FLAG_NONE,
		&resDescMat,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&transformationMatrixResourceSprite));
	assert(SUCCEEDED(hr));

	// データを書き込むためのポインタを作る
	TransformationMatrix* transformationMatrixDataSprite = nullptr;

	// GPUメモリと紐づける（Map）
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));

	// とりあえず単位行列を入れておく
	transformationMatrixDataSprite->WVP = MatrixMath::MakeIdentity4x4();
	transformationMatrixDataSprite->World = MatrixMath::MakeIdentity4x4();

	// ビューを作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(vertices);
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite;
	materialResourceSprite = dxCommon->CreateBufferResource(sizeof(Material));

	// データを書き込む
	Material* materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));

	// 初期値設定（これをしないと真っ黒になる）
	materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白
	materialDataSprite->enableLighting = 0; // スプライトなのでライティング不要
	materialDataSprite->uvTransform = MatrixMath::MakeIdentity4x4();
	*/
#pragma endregion

	// ファイルを読み込む
	DirectX::ScratchImage mipImages = dxCommon->LoadTexture("Resource/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	// GPUバッファを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResourceSprite = dxCommon->CreateTextureResource(metadata);

	// データを転送する
	dxCommon->UploadTextureData(textureResourceSprite.Get(), mipImages);

	// SRV (Shader Resource View) を作る
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// SRVを作る場所（ディスクリプタヒープの場所）を決める
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(1);
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(1);

	// SRV作成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResourceSprite.Get(), &srvDesc, srvHandleCPU);

	// スプライトの生成と初期化

	//Sprite* sprite = new Sprite();
	//sprite->Initialize(spriteCommon);

	std::vector<Sprite*>sprites;
	const int kSpriteCount = 5;
	for (uint32_t i = 0; i < kSpriteCount; ++i)
	{
		Sprite* pSprite = new Sprite();
		pSprite->Initialize(spriteCommon);
		
		//初期位置を少しずつずらす
		Vector2 startPos = { i * 200.0f,0.0f };
		pSprite->SetPosition(startPos);
		
		sprites.push_back(pSprite);
	}

	// ImGui用の変数を定義（Vector構造体は Vector.h 由来）
	Vector2 spritePosition = { 0.0f, 0.0f };
	float spriteRotation = 0.0f;
	Vector2 spriteSize = { 640.0f, 360.0f };
	Vector4 spriteColor = { 1.0f, 1.0f, 1.0f, 1.0f };


	// メインループ
	while (winApp->ProcessMessage() == false)
	{
		// 入力情報の更新
		input->Update();

	
		//ImGuiのフレーム開始処理
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//UIの構築 (スプライトクラスのセッターを使う)
		ImGui::Begin("Sprite Controller"); // ウィンドウのタイトル

		// スプライトの座標
		ImGui::DragFloat2("Position", &spritePosition.x, 1.0f);
		// スプライトの回転
		ImGui::DragFloat("Rotation", &spriteRotation, 0.01f);
		// スプライトのサイズ
		ImGui::DragFloat2("Size", &spriteSize.x, 1.0f);
		// 色
		ImGui::ColorEdit4("Color", &spriteColor.x);

		ImGui::End(); // ウィンドウの終わり

		//更新処理

		// ImGuiの値をスプライトに反映
		//sprite->SetPosition(spritePosition);
		//sprite->SetRotation(spriteRotation);
		//sprite->SetSize(spriteSize);
		//sprite->SetColor(spriteColor);

		// スプライトの更新（行列計算など）
		//sprite->Update();

		for (size_t i = 0; i < sprites.size(); ++i)
		{
			Sprite* pSprite = sprites[i];

			// 個別の位置計算： (ImGuiの基準位置) + (スプライトごとのオフセット)
			// こうしないと、ImGuiを触った瞬間に全員が同じ場所に集合してしまいます
			Vector2 offset = { i * 50.0f, 50.0f };
			pSprite->SetPosition({ spritePosition.x + offset.x, spritePosition.y + offset.y });

			// 回転・サイズ・色は全員同じにする
			pSprite->SetRotation(spriteRotation);
			pSprite->SetSize(spriteSize);
			pSprite->SetColor(spriteColor);

			// 更新行列の計算
			pSprite->Update();
		}

		//描画処理

		// 描画前処理（画面クリアなど）
		dxCommon->PreDraw();

		// スプライト共通設定（ルートシグネチャ、PSO設定）
		spriteCommon->CommonDrawSettings();

		// スプライト描画
		//sprite->Draw(dxCommon->GetCommandList(), srvHandleGPU);

		for (Sprite* pSprite :sprites)
		{
			pSprite->Draw(dxCommon->GetCommandList(), srvHandleGPU);
		}

		// ImGuiの内部コマンド生成
		ImGui::Render();
		// ImGuiの描画コマンドを発行
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

		// 描画後処理（フリップなど）
		dxCommon->PostDraw();
	}
	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	for (Sprite* pSprite : sprites)
	{
		delete pSprite;
	}
	sprites.clear(); // 忘れずにクリア
	//delete sprite;
	delete spriteCommon;
	delete dxCommon;
	delete input;
	delete winApp;

	return 0;
}