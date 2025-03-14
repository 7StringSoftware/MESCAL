
#define D2D_INPUT_COUNT 0           // The pixel shader takes 1 input textur
#define D2D_INPUT0_COMPLEX          // The first input is sampled in a complex manner: to calculate the output of a pixel,
                                    // the shader samples more than just the corresponding input coordinate.
#define D2D_REQUIRES_SCENE_POSITION // The pixel shader requires the SCENE_POSITION input.

#include "d2d1effecthelpers.hlsli"

Texture2D inputTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    float4 fillColor;
};

D2D_PS_ENTRY(main)
{
    float2 toPixel = D2DGetScenePosition().xy;

    if (toPixel.x < 100.0f)
{
    return
float4(0,1,0,0);
}

    return float4(0,0,0,0);
}
