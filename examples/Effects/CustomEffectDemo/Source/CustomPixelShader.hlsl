
#define D2D_INPUT_COUNT 0           // The pixel shader takes 1 input textur
#define D2D_INPUT0_COMPLEX          // The first input is sampled in a complex manner: to calculate the output of a pixel,
                                    // the shader samples more than just the corresponding input coordinate.

#include "d2d1effecthelpers.hlsli"

Texture2D inputTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    float4 fillColor;
};

D2D_PS_ENTRY(main)
{
    return
fillColor;
}
