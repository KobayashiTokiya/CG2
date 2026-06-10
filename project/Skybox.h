#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "Vector.h"
#include "Matrix.h"
#include "Transform.h"

// 前方宣言
class SkyboxCommon;
class Camera;

class Skybox
{
private:
    struct VertexPos
    {
        Vector4 position;
    };

    struct TransformationMatrix
    {
        Matrix4x4 WVP;
        Matrix4x4 World;
    };

public:
	void Initialize(SkyboxCommon* skyboxCommon, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);
	void Update(Camera* camera);
	void Draw();

private://箱の生成
    void CreateCube();

private:
    SkyboxCommon* skyboxCommon_ = nullptr;
    Camera* camera_ = nullptr;

    // トランスフォーム
    Transform transform_{};

    // テクスチャ（GPUハンドルのみを保持）
    D3D12_GPU_DESCRIPTOR_HANDLE textureHandle_{};

    // --- 頂点バッファ ---
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

    // --- インデックスバッファ ---
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_;
    D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

    // --- 定数バッファ (座標変換行列) ---
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationResource_;
    TransformationMatrix* transformationMatrixData_ = nullptr; // Map先のポインタ
};