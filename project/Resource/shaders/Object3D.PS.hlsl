#include "object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
};

struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    /// 1. テクスチャから色をサンプリング（抽出）する
    // input.texcoord（UV座標）を使って、画像の対応する場所の色を取ってくる
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);

    // 2. テクスチャの色と、マテリアルの色を掛け合わせる
    // (マテリアルが白なら、テクスチャの色がそのまま出る)
    output.color = gMaterial.color * textureColor;
    
    // ※もしアルファ値(透明度)が0なら描画しない処理を入れる場合
    if (output.color.a == 0.0)
    {
        discard;
    }
    return output;
}