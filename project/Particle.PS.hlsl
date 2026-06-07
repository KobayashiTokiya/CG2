#include "Particle.hlsli"

// ※MaterialとDirectionalLightの構造体は一旦消す（またはコメントアウト）

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// ※b0 と b1 の ConstantBuffer の宣言も消しました！

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    //テクスチャの色をサンプリングする
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    // テクスチャの色に、VSから送られてきた色(input.color)を掛け算する
    output.color = textureColor*input.color;
    
    // textureのα値が0のときにPixelを棄却（この処理は残してOKです！）
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}