#include "Skybox.h"
#include "SkyboxCommon.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include <algorithm>

void Skybox::Initialize(SkyboxCommon* skyboxCommon, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle)
{
    // 引数で受け取ったものをメンバ変数に記録
    this->skyboxCommon_ = skyboxCommon;
    this->textureHandle_ = textureHandle;

    // 1. 箱のモデルデータ（頂点・インデックス）を生成
    CreateCube();

    // 2. 定数バッファ(座標変換行列用)の生成
    DirectXCommon* dxCommon = skyboxCommon_->GetDxCommon();

    // 変数名を transformationResource_ に統一
    transformationResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));

    // マップ先のポインタも transformationMatrixData_ に統一
    transformationResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

    // 初期値として単位行列を書き込んでおく
    transformationMatrixData_->WVP = MatrixMath::MakeIdentity4x4();
    transformationMatrixData_->World = MatrixMath::MakeIdentity4x4();

    // 3. 初期状態の設定 (カメラを包むように巨大化)
    transform_.scale = { 500.0f, 500.0f, 500.0f };
    transform_.rotate = { 0.0f, 0.0f, 0.0f };
    transform_.translate = { 0.0f, 0.0f, 0.0f };
}

void Skybox::Update(Camera* camera)
{
    this->camera_ = camera;
    // スカイボックスは常にカメラと同じ位置に配置する
    transform_.translate = camera->GetTranslate();

    // ワールド行列の作成
    Matrix4x4 worldMatrix = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // WVP行列の計算 (World * View * Projection)
    Matrix4x4 viewMatrix = camera->GetViewMatrix();
    Matrix4x4 projectionMatrix = camera->GetProjectionMatrix();
    Matrix4x4 worldViewMatrix = MatrixMath::Multiply(worldMatrix, viewMatrix);
    Matrix4x4 worldViewProjectionMatrix = MatrixMath::Multiply(worldViewMatrix, projectionMatrix);

    // 定数バッファに書き込む
    transformationMatrixData_->World = worldMatrix;
    transformationMatrixData_->WVP = worldViewProjectionMatrix;
}

void Skybox::Draw()
{
    ID3D12GraphicsCommandList* commandList = skyboxCommon_->GetDxCommon()->GetCommandList();

    // 安全装置：ハンドルが空なら描画をスキップ（クラッシュ防止）
    if (textureHandle_.ptr == 0) return;

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->IASetIndexBuffer(&indexBufferView_);

    // --- ここを 0 と 1 に修正 ---
    // 0番：定数バッファ(WVP)
    commandList->SetGraphicsRootConstantBufferView(0, transformationResource_->GetGPUVirtualAddress());

    // 1番：テクスチャ(キューブマップ)
    commandList->SetGraphicsRootDescriptorTable(1, textureHandle_);
    // ---------------------------

    commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

void Skybox::CreateCube()
{
    DirectXCommon* dxCommon = skyboxCommon_->GetDxCommon();

    // --- 1. 頂点データ (8つの角) ---
    VertexPos vertices[8] = {
        { {-1.0f, -1.0f,  1.0f, 1.0f} }, // 0: 左下前
        { { 1.0f, -1.0f,  1.0f, 1.0f} }, // 1: 右下前
        { {-1.0f,  1.0f,  1.0f, 1.0f} }, // 2: 左上前
        { { 1.0f,  1.0f,  1.0f, 1.0f} }, // 3: 右上前
        { {-1.0f, -1.0f, -1.0f, 1.0f} }, // 4: 左下奥
        { { 1.0f, -1.0f, -1.0f, 1.0f} }, // 5: 右下奥
        { {-1.0f,  1.0f, -1.0f, 1.0f} }, // 6: 左上奥
        { { 1.0f,  1.0f, -1.0f, 1.0f} }  // 7: 右上奥
    };

    // 頂点バッファの生成
    vertexBuffer_ = dxCommon->CreateBufferResource(sizeof(VertexPos) * 8);
    VertexPos* vertexMap = nullptr;
    vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexMap));
    std::copy(std::begin(vertices), std::end(vertices), vertexMap);
    vertexBuffer_->Unmap(0, nullptr);

    // 頂点バッファビューの設定
    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexPos) * 8;
    vertexBufferView_.StrideInBytes = sizeof(VertexPos);

    // --- 2. インデックスデータ (36個) ---
    uint16_t indices[36] = {
        0, 1, 2, 2, 1, 3, // 前
        5, 4, 7, 7, 4, 6, // 奥
        4, 0, 6, 6, 0, 2, // 左
        1, 5, 3, 3, 5, 7, // 右
        2, 3, 6, 6, 3, 7, // 上
        4, 5, 0, 0, 5, 1  // 下
    };

    // インデックスバッファの生成
    indexBuffer_ = dxCommon->CreateBufferResource(sizeof(uint16_t) * 36);
    uint16_t* indexMap = nullptr;
    indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexMap));
    std::copy(std::begin(indices), std::end(indices), indexMap);
    indexBuffer_->Unmap(0, nullptr);

    // インデックスバッファビューの設定
    indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = sizeof(uint16_t) * 36;
    indexBufferView_.Format = DXGI_FORMAT_R16_UINT;
}