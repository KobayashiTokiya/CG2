#include "object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
    float32_t shininess;
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

struct Camera
{
    float32_t3 worldPosition;
};
ConstantBuffer<Camera> gCamera : register(b2);


PixelShaderOutput main(VertexShaderOutput input)
{ 
    PixelShaderOutput output;
    output.color = gMaterial.color;
    //UV変換&テクスチャサンプル
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler,transformedUV.xy);//input.texcoord
   
    //discard
    //textureのα値が0.5以下のときにPixelを棄却
    if (textureColor.a<=0.5)
    {
        discard;
    }
    //textureのα値が0のときにPixelを棄却
    if (textureColor.a == 0.0)
    {
        discard;
    }
    //output.colorのα値が0の時にPixelを棄却
    if (output.color.a==0.0)
    {
        discard;
    }
    
    //Half-Lambert
    /*
        if (gMaterial.enableLighting != 0)
        {
            float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        
            output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
            output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            output.color.a = gMaterial.color.a * textureColor.a;
        }
        else
        {
            output.color = gMaterial.color * textureColor;
        }
    */
    if (gMaterial.enableLighting == 0)
    {
        output.color = gMaterial.color * textureColor;
        return output;
    }
    float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
    float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float32_t3 reflectLight = reflect(-gDirectionalLight.direction, normalize(input.normal));
    float RdotE = dot(reflectLight, toEye);
    float specularPow = pow(saturate(RdotE), gMaterial.shininess)*cos; //反射強度
    
    //拡散反射
    float32_t3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
    //鏡面反射
    float32_t3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);
    //拡散反射+鏡面反射
    output.color.rgb = diffuse + specular;
    //アルファは今まで通り
    output.color.a = gMaterial.color.a * textureColor.a;
 
    return output;
}