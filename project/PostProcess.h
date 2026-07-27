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
	void Draw(ID3D12GraphicsCommandList* commandList, RenderTexture* renderTexture,bool enable,int effectModel,const Vector3& colorScale);
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineStatePostProcess_; // 通常 / 複合用
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineStateGrayscale_;   // グレースケール
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineStateVignette_;    // ビネット
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineStateBoxFilter_;  // ボックスフィルター
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineStateGaussian_;   // ガウシアンブラー
	struct PostProcessData
	{
		int32_t enable;
		int32_t effectMode;
		float padding[2];
		Vector3 colorScale;
	};
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuffer_;
	PostProcessData* cBufferData_ = nullptr;
};