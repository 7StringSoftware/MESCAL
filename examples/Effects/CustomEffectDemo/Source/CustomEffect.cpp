
#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d2d1_3helper.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effecthelpers.h>
#include <initguid.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_graphics/native/juce_Direct2DMetrics_windows.h>
#include <juce_graphics/native/juce_EventTracing.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DPixelDataPage_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include <juce_graphics/native/juce_Direct2DGraphicsContext_windows.h>
#include <juce_graphics/native/juce_Direct2DImageContext_windows.h>
#include "CustomEffect.h"
namespace CustomComputeShader
{
#include "ComputeShader.h"
}

DEFINE_GUID(CLSID_CustomEffect, 0x3ee07f1c, 0x50ee, 0x4b72, 0xa9, 0xac, 0x7c, 0x47, 0xbd, 0x8, 0xa6, 0x5e);
DEFINE_GUID(CLSID_CustomEffectPixelShader, 0xbae25466, 0xc066, 0x4086, 0x82, 0x22, 0xf0, 0x1d, 0xf7, 0xf3, 0x9f, 0xd3);
DEFINE_GUID(CLSID_ComputeShader,0x4febef1a, 0xd634, 0x4a46, 0xbb, 0xbd, 0xe1, 0x88, 0xb8, 0x2c, 0x91, 0x5b);

static constexpr std::wstring_view xml =
LR"(<?xml version='1.0' encoding='UTF-16' ?>
<Effect>
<Property name='DisplayName' type='string' value='compute shader test' />
<Property name='Author' type='string' value='Microsoft Corporation' />
<Property name='Category' type='string' value='Stylize' />
<Property name='Description' type='string' value='Adds a ripple effect that can be animated'/>
<Inputs>
<Input name='Source'/>
</Inputs>
<Property name='Count' type='int32'>
    <Property name='DisplayName' type='string' value='Count'/>
</Property>
</Effect>
)";

class CustomEffect::Pimpl
{
public:
    Pimpl()
    {
    }

    ~Pimpl()
    {
    }

    struct Interface : public juce::ComBaseClassHelper<ID2D1EffectImpl, ID2D1ComputeTransform>
    {
        HRESULT setRectangleCount(int count)
        {
            constants.numRectangles = count;
            return S_OK;
        }

        int getRectangleCount() const
        {
            return constants.numRectangles;
        }

        Interface()
        {
            juce::SharedResourcePointer<juce::DirectX> directX;

            const D2D1_PROPERTY_BINDING bindings[] =
            {
                D2D1_VALUE_TYPE_BINDING(L"Count", &Interface::setRectangleCount, &Interface::getRectangleCount)
            };

            auto hr = directX->getD2DFactory()->RegisterEffectFromString(CLSID_CustomEffect, xml.data(), bindings, 1, CreateEffect);
            jassert(SUCCEEDED(hr));
        }

        ~Interface()
        {
            computeInfo.resetAndGetPointerAddress();
        }

        JUCE_COMRESULT QueryInterface(REFIID refId, void** result) override
        {
            if (refId == __uuidof (ID2D1Transform))
                return castToType<ID2D1Transform>(result);

            return juce::ComBaseClassHelper<ID2D1EffectImpl, ID2D1ComputeTransform>::QueryInterface(refId, result);
        }


        STDOVERRIDEMETHODIMP Initialize(_In_ ID2D1EffectContext* effectContext, _In_ ID2D1TransformGraph* transformGraph) override
        {
            //HRESULT hr = effectContext->LoadPixelShader(CLSID_CustomEffectPixelShader, g_main, sizeof(g_main));
            HRESULT hr = effectContext->LoadComputeShader(CLSID_ComputeShader, CustomComputeShader::g_main, sizeof(CustomComputeShader::g_main));
            jassert(SUCCEEDED(hr));

            if (SUCCEEDED(hr))
            {
                // The graph consists of a single transform. In fact, this class is the transform,
                // reducing the complexity of implementing an effect when all we need to
                // do is use a single pixel shader.
                //auto drawTransform = static_cast<ID2D1ComputeTransform*>(this);
                hr = transformGraph->SetSingleTransformNode(this);
                //transformGraph->Clear();
                //hr = transformGraph->AddNode(this);
                //jassert(SUCCEEDED(hr));
                //hr = transformGraph->SetOutputNode(this);
                jassert(SUCCEEDED(hr));
            }

            return hr;
        }

        STDOVERRIDEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE changeType) override
        {
            if (computeInfo)
            {
                computeInfo->SetComputeShaderConstantBuffer(reinterpret_cast<BYTE*>(&constants), sizeof(constants));
                return S_OK;
            }

            return E_INVALIDARG;
        }

        STDOVERRIDEMETHODIMP SetGraph(_In_ ID2D1TransformGraph* transformGraph) override
        {
            return E_NOTIMPL;
        }

        static HRESULT __stdcall CreateEffect(_Outptr_ IUnknown** ppEffectImpl)
        {
            // This code assumes that the effect class initializes its reference count to 1.
            *ppEffectImpl = static_cast<ID2D1EffectImpl*>(new Interface{});
            if (*ppEffectImpl == nullptr)
            {
                return E_OUTOFMEMORY;
            }
            return S_OK;
        }

        STDOVERRIDEMETHODIMP_(UINT32) GetInputCount() CONST
        {
            return numInputs;
        }

        STDOVERRIDEMETHODIMP MapOutputRectToInputRects(_In_ CONST D2D1_RECT_L* outputRect, _Out_writes_(inputRectCount) D2D1_RECT_L* inputRects, UINT32 inputRectCount) CONST
        {
            if (inputRectCount > numInputs)
            {
                return E_INVALIDARG;
            }

            inputRects[0] = *outputRect;
            return S_OK;
        }

        STDOVERRIDEMETHODIMP MapInputRectsToOutputRect(_In_reads_(inputRectCount) CONST D2D1_RECT_L* inputRects, _In_reads_(inputRectCount) CONST D2D1_RECT_L* inputOpaqueSubRects, UINT32 inputRectCount, _Out_ D2D1_RECT_L* outputRect, _Out_ D2D1_RECT_L* outputOpaqueSubRect)
        {
            if (inputRectCount > numInputs)
            {
                return E_INVALIDARG;
            }

            outputRect->left = 0;
            outputRect->top = 0;
            outputRect->right = outputWidth;
            outputRect->bottom = outputHeight;

            outputOpaqueSubRect->left = 0;
            outputOpaqueSubRect->top = 0;
            outputOpaqueSubRect->right = outputWidth;
            outputOpaqueSubRect->bottom = outputHeight;

            return S_OK;
        }

        STDOVERRIDEMETHODIMP MapInvalidRect(UINT32 inputIndex, D2D1_RECT_L invalidInputRect, _Out_ D2D1_RECT_L* invalidOutputRect) const
        {
            if (inputIndex > numInputs)
            {
                return E_INVALIDARG;
            }

            invalidOutputRect->left = 0;
            invalidOutputRect->top = 0;
            invalidOutputRect->right = outputWidth;
            invalidOutputRect->bottom = outputHeight;
            return S_OK;
        }

        STDOVERRIDEMETHODIMP SetDrawInfo(_In_ ID2D1DrawInfo* drawInfo)
        {
            // Set the pixel shader for the draw info
            HRESULT hr = drawInfo->SetPixelShader(CLSID_CustomEffectPixelShader);
            if (FAILED(hr))
            {
                return hr;
            }

            return S_OK;
        }

        // Inherited via ID2D1ComputeTransform
        HRESULT __stdcall SetComputeInfo(ID2D1ComputeInfo* computeInfoIn) override
        {
            HRESULT hr = computeInfoIn->SetComputeShader(CLSID_ComputeShader);
            if (FAILED(hr))
            {
                return hr;
            }

            *computeInfo.resetAndGetPointerAddress() = computeInfoIn;
            computeInfo->AddRef();

            return S_OK;
        }

        HRESULT __stdcall CalculateThreadgroups(const D2D1_RECT_L* outputRect, UINT32* dimensionX, UINT32* dimensionY, UINT32* dimensionZ) override
        {
            static const int CS5_numThreadsX = 32;
            static const int CS5_numThreadsY = 32;

            auto width = outputRect->right - outputRect->left;
            auto height = outputRect->bottom - outputRect->top;
            *dimensionX = (width + CS5_numThreadsX - 1) & ~(CS5_numThreadsX - 1);
            *dimensionY = (height + CS5_numThreadsY - 1) & ~(CS5_numThreadsY - 1);
            *dimensionZ = 1;
            return S_OK;
        }

        uint32_t numInputs = 1;
        uint32_t outputWidth = 1024, outputHeight = 1024;
        juce::ComSmartPtr<ID2D1ComputeInfo> computeInfo;
        struct Constants
        {
            int numRectangles = 0;
        } constants;

    } effectInterface;

    struct Resources
    {
        HRESULT create()
        {
            if (!adapter)
            {
                adapter = directX->adapters.getDefaultAdapter();
            }

            if (adapter && !deviceContext)
            {
                juce::ComSmartPtr<ID2D1DeviceContext1> deviceContext1;
                if (const auto hr = adapter->direct2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
                    deviceContext1.resetAndGetPointerAddress());
                    FAILED(hr))
                {
                    jassertfalse;
                    return hr;
                }

                deviceContext1->QueryInterface<ID2D1DeviceContext2>(deviceContext.resetAndGetPointerAddress());
            }

            return S_OK;
        }

        void release()
        {
            deviceContext = nullptr;
            adapter = nullptr;
        }

        juce::SharedResourcePointer<juce::DirectX> directX;
        juce::DxgiAdapter::Ptr adapter;
        juce::ComSmartPtr<ID2D1DeviceContext2> deviceContext;
    };

    juce::SharedResourcePointer<Resources> resources;
    juce::ComSmartPtr<ID2D1Effect> customD2DEffect;
    juce::ComSmartPtr<ID2D1Bitmap> rectangleBitmap;
    juce::HeapBlock<D2D1_VECTOR_4F> rectangles{ 256 * 256, true };

    void createD2DEffect()
    {
        if (auto hr = resources->create(); FAILED(hr))
        {
            jassertfalse;
            return;
        }

        if (!customD2DEffect)
        {
            if (const auto hr = resources->deviceContext->CreateEffect(CLSID_CustomEffect, customD2DEffect.resetAndGetPointerAddress()); FAILED(hr))
            {
                jassertfalse;
            }
        }

        if (!rectangleBitmap)
        {
            D2D1_BITMAP_PROPERTIES bitmapProperties;
            bitmapProperties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
            bitmapProperties.dpiX = USER_DEFAULT_SCREEN_DPI;
            bitmapProperties.dpiY = USER_DEFAULT_SCREEN_DPI;
            D2D1_SIZE_U size{ 1024, 1024 };
            if (const auto hr = resources->deviceContext->CreateBitmap(size, bitmapProperties, rectangleBitmap.resetAndGetPointerAddress()); FAILED(hr))
            {
                jassertfalse;
            }
        }
    }
};

CustomEffect::CustomEffect()
    : pimpl{ std::make_unique<Pimpl>() }
{
}

CustomEffect::~CustomEffect()
{
}

static void getLastError()
{
    wchar_t messageBuffer[256] = {};

    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        messageBuffer,
        (DWORD)juce::numElementsInArray(messageBuffer) - 1,
        nullptr);

    DBG(messageBuffer);
    jassertfalse;
}
void CustomEffect::applyEffect(juce::Image const& sourceImage, juce::Image& outputImage, const juce::AffineTransform& transform, bool clearDestination)
{
    pimpl->createD2DEffect();
    if (!pimpl->customD2DEffect)
    {
        return;
    }
    juce::Direct2DPixelData::Ptr outputPixelData = dynamic_cast<juce::Direct2DPixelData*>(outputImage.getPixelData().get());
    if (!outputPixelData)
    {
        return;
    }

    pimpl->resources->deviceContext->SetTarget(outputPixelData->getFirstPageForDevice(pimpl->resources->adapter->direct2DDevice));
    pimpl->resources->deviceContext->BeginDraw();
    pimpl->resources->deviceContext->Clear();

    if (!transform.isIdentity())
        pimpl->resources->deviceContext->SetTransform(juce::D2DUtilities::transformToMatrix(transform));

    {
        juce::Image::BitmapData bd{ sourceImage, juce::Image::BitmapData::readOnly };
        pimpl->rectangleBitmap->CopyFromMemory(nullptr, bd.data, bd.lineStride);
    }

    pimpl->customD2DEffect->SetInput(0, pimpl->rectangleBitmap);

    pimpl->resources->deviceContext->DrawImage(pimpl->customD2DEffect.get());
    [[maybe_unused]] auto hr = pimpl->resources->deviceContext->EndDraw();
    jassert(SUCCEEDED(hr));
}
