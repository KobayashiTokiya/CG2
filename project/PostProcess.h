#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "DirectXCommon.h"
#include "RenderTexture.h"
#include "Vector.h"

class PostProcess
{
public:
	void Initialize(DirectXCommon* dxCommon);
	void Draw(ID3D12GraphicsCommandList* commandList, RenderTexture* renderTexture,bool enable,const Vector3& colorScale);
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

	struct PostProcessData
	{
		int32_t enable;
		float padding[3];
		Vector3 colorScale;
	};
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuffer_;
	PostProcessData* cBufferData_ = nullptr;
};