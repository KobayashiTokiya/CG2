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
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
   
   // OFF（0）なら、加工せずそのまま元の色を出力
    if (gParam.gEnable == 0)
    {
        output.color = textureColor;
        return output;
    }
    
    if (gParam.gEffectMode == 1)
    {
        // ==========================================
        // グレースケール処理
        // ==========================================
        float32_t value = dot(textureColor.rgb, float32_t3(0.2125f, 0.7154f, 0.0721f));
        float32_t3 grayColor = float32_t3(value, value, value);
        
        // スライダーのX値（0～100）を強さ（0.0～1.0）として扱う
        float32_t strength = gParam.gColorScale.x * 0.01f;
        
        // カラーと白黒をなめらかに補間
        output.color.rgb = lerp(textureColor.rgb, grayColor, strength);
        output.color.a = textureColor.a;
    }
    else
    {
        // ==========================================
        // 背景色変更処理 (前回の機能)
        // ==========================================
        if (textureColor.b > textureColor.g + 0.1f && textureColor.b > textureColor.r + 0.1f)
        {
            output.color.r = gParam.gColorScale.x * 0.01f;
            output.color.g = gParam.gColorScale.y * 0.01f;
            output.color.b = gParam.gColorScale.z * 0.01f;
            output.color.a = textureColor.a;
        }
        else
        {
            output.color = textureColor;
        }
    }
    
    return output;
}