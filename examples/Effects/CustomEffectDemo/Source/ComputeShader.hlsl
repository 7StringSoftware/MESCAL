//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// These values should match those in DftTransform. These values control
// the number of threads in each thread group. Higher values mean more work is
// done in parallel on the GPU - they don't have to be these particular values.
// However for Shader Model 5, z <= 64 and x*y*z <= 1024.
#define NUMTHREADS_X 32
#define NUMTHREADS_Y 1 //32
#define NUMTHREADS_Z 1

#define PI 3.14159265358979

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture;
SamplerState inputSampler : register(s0);

// These are default constants passed by D2D. See PixelShader and VertexShader
// projects for how to pass custom values into a shader.
cbuffer systemConstants : register(b0)
{
    int4 resultRect; // Represents the input rectangle to the shader
    int2 outputOffset;
    float2 sceneToInput0X;
    float2 sceneToInput0Y;
};

cbuffer constants : register(b1)
{
    int numRectangles;
};

// The image does not necessarily begin at (0,0) on InputTexture. The shader needs
// to use the coefficients provided by Direct2D to map the requested image data to
// where it resides on the texture.
float2 ConvertInput0SceneToTexelSpace(float2 inputScenePosition)
{
    float2 ret;
    ret.x = inputScenePosition.x * sceneToInput0X[0] + sceneToInput0X[1];
    ret.y = inputScenePosition.y * sceneToInput0Y[0] + sceneToInput0Y[1];
    return ret;
}

// numthreads(x, y, z)
// This specifies the number of threads in each dispatched threadgroup.
// For Shader Model 5, z <= 64 and x*y*z <= 1024
[numthreads(NUMTHREADS_X, NUMTHREADS_Y, NUMTHREADS_Z)]
void main(
    // dispatchThreadId - Uniquely identifies a given execution of the shader, most commonly used parameter.
    // Definition: (groupId.x * NUM_THREADS_X + groupThreadId.x, groupId.y * NUMTHREADS_Y + groupThreadId.y, groupId.z * NUMTHREADS_Z + groupThreadId.z)
    uint3 dispatchThreadId : SV_DispatchThreadID,

    // groupThreadId - Identifies an individual thread within a thread group.
    // Range: (0 to NUMTHREADS_X - 1, 0 to NUMTHREADS_Y - 1, 0 to NUMTHREADS_Z - 1)
    uint3 groupThreadId : SV_GroupThreadID,

    // groupId - Identifies which thread group the individual thread is being executed in.
    // Range defined in DFTHorizontalTransform::CalculateThreadgroups
    uint3 groupId : SV_GroupID,

    // One dimensional indentifier of a compute shader thread within a thread group.
    // Range: (0 to NUMTHREADS_X * NUMTHREADS_Y * NUMTHREADS_Z - 1)
    uint groupIndex : SV_GroupIndex
    )
{
    int width = resultRect[2] - resultRect[0];
    int height = resultRect[3] - resultRect[1];

    // It is likely that the compute shader will execute beyond the bounds of the input image, since the shader is executed in chunks sized by
    // the threadgroup size defined in DFTHorizontalTransform::CalculateThreadgroups. For this reason each shader should ensure the current
    // dispatchThreadId is within the bounds of the input image before proceeding.
    if ((int) dispatchThreadId.x >= width || (int) dispatchThreadId.y >= height)
    {
        return;
    }
    
    float2 uv = float2(dispatchThreadId.xy) / float2(width, height);
    float4 sample = InputTexture.SampleLevel(inputSampler, uv, .0);
    float alpha = sample.w;
    if (alpha >= 0.5f)
    {
        float smoothed = smoothstep(0.5f - 0.01f, 0.5f + 0.01f, alpha);
        smoothed = (smoothed - 0.5f) * 2.0f;
        OutputTexture[dispatchThreadId.xy + outputOffset.xy + resultRect.xy] = float4(alpha, 0.0, 0.0f, alpha);
        return;
    }
        
    OutputTexture[dispatchThreadId.xy + outputOffset.xy + resultRect.xy] = float4(0.0, 0.0, 0.0, 0.0);
    return;

    #if 0
    int textureWidth = 256;
    int textureHeight = 256;

    float4 color = float4(1.0, 0.0, 0.0, 1.0);
    int rectangleIndex = (int) dispatchThreadId.x;
    float4 rectangle = InputTexture.Load(int3(rectangleIndex, 0, 0));
    for (int x = (int) rectangle.x; x < (int) rectangle.z; ++x)
    {
        for (int y = (int) rectangle.y; y < (int) rectangle.w; ++y)
        {
            color = float4(0.0f, 1.0f, 1.0f, 1.0f);
            OutputTexture[uint2(x, y)] = color;
        }
    }
#endif

#if 0
    for (int rectangleIndex = 0; rectangleIndex < numRectangles; ++rectangleIndex)
    {
        int inputX = rectangleIndex & 255;
        int inputY = rectangleIndex >> 8;
        float4 rectangle = InputTexture.Load(int3(inputX, inputY, 0));

        int left = (int) rectangle.x;
        int top = (int) rectangle.y;
        int right = (int) rectangle.z;
        int bottom = (int) rectangle.w;
        if ((int) dispatchThreadId.x >= left &&
            (int) dispatchThreadId.x < right &&
            (int) dispatchThreadId.y >= top &&
            (int) dispatchThreadId.y < bottom)
        {
            color = float4(0.0f, 1.0f, 1.0f, 1.0f);
            break;
        }
    }
#endif

        //OutputTexture[dispatchThreadId.xy + outputOffset.xy + resultRect.xy] = color;
}
