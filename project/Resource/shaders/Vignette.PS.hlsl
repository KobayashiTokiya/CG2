#include "Fullscreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PostProcessParam
{
    int32_t gEnable;
    int32_t gEffectMode;
    float32_t2 padding;
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
   
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    float32_t2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    
    float vignette = correct.x * correct.y * 16.0f;
    
    vignette = saturate(pow(vignette, 0.8f));
    
    output.color.rgb *= vignette;
    
    return output;
}