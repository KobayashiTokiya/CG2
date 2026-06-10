#include "Skybox.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float4 position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    // 座標変換
    output.position = mul(input.position, gTransformationMatrix.WVP);
    // 頂点座標（ローカル）をそのまま方向ベクトルとして使う
    output.texcoord = input.position.xyz;
    return output;
}