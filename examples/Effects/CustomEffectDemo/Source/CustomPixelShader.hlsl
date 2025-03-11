Texture2D inputTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    float4 fillColor;
};

float4 main(float4 pos : SV_POSITION, float2 texCoord : TEXCOORD) : SV_TARGET
{
    return fillColor;
}
