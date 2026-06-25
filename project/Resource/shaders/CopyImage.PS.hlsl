#include "Fullscreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PostProcessParam
{
    int32_t gEnable;
    float32_t3 padding;
    float32_t3 gColorScale; // C++側の colorScale
};
ConstantBuffer<PostProcessParam> gParam : register(b0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
   
   // OFF（0）なら、加工せずそのまま元の色を出力
    if (gParam.gEnable == 0)
    {
        output.color = textureColor;
        return output;
    }
    
    //ON の場合:背景かどうかの判定（クリアカラーの青みを抜き出す）
    if (textureColor.b > textureColor.g + 0.1f && textureColor.b > textureColor.r + 0.1f)
    {
        output.color.r = gParam.gColorScale.x * 0.01f;
        output.color.g = gParam.gColorScale.y * 0.01f;
        output.color.b = gParam.gColorScale.z * 0.01f;
        output.color.a = textureColor.a;
    }
    else
    {
        // オブジェクト部分はそのまま
        output.color = textureColor;
    }
    
    return output;
}