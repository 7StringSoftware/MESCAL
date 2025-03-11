
#include <d2d1_1.h>
#include <d2d1effectauthor.h>
#include <d2d1effecthelpers.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include "CustomEffect.h"

// {3EE07F1C-50EE-4B72-A9AC-7C47BD08A65E}
DEFINE_GUID(CLSID_CustomEffect, 0x3ee07f1c, 0x50ee, 0x4b72, 0xa9, 0xac, 0x7c, 0x47, 0xbd, 0x8, 0xa6, 0x5e);


class CustomEffect::Pimpl
{
public:
    struct Interface : public juce::ComBaseClassHelper<ID2D1EffectImpl>, public juce::ComBaseClassHelper<ID2D1Transform>
    {
        STDOVERRIDEMETHODIMP Initialize(_In_ ID2D1EffectContext* effectContext, _In_ ID2D1TransformGraph* transformGraph) override
        {

        }

        STDOVERRIDEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE changeType) override
        {
            return S_OK;
        }

        STDOVERRIDEMETHODIMP SetGraph(_In_ ID2D1TransformGraph* transformGraph) override
        {
            return E_NOTIMPL;
        }

        STDOVERRIDEMETHODIMP MapInputRectsToOutputRect(_In_reads_(inputRectCount) const D2D1_RECT_L* inputRects, _In_reads_(inputRectCount) const D2D1_RECT_L* inputOpaqueSubRects, UINT32 inputRectCount, _Out_writes_(inputRectCount) D2D1_RECT_L* outputRects, _Out_writes_(inputRectCount) D2D1_RECT_L* outputOpaqueSubRects) override
        {

        }

        STDOVERRIDEMETHODIMP MapOutputRectToInputRects(_In_ CONST D2D1_RECT_L* outputRect, _Out_writes_(inputRectsCount) D2D1_RECT_L* inputRects, UINT32 inputRectsCount) CONST
        {
        }

        STDOVERRIDEMETHODIMP MapInvalidRect(UINT32 inputIndex, D2D1_RECT_L invalidInputRect, _Out_ D2D1_RECT_L* invalidOutputRect) CONST
        {
        }

        STDOVERRIDEMETHODIMP_(UINT32) GetInputCount() CONST
        {
            return 0;
        }
    };

    HRESULT __stdcall CreateEffect(_Outptr_ IUnknown** ppEffectImpl)
    {
        // This code assumes that the effect class initializes its reference count to 1.
        *ppEffectImpl = static_cast<ID2D1EffectImpl*>(new Interface{});
        if (*ppEffectImpl == nullptr)
        {
            return E_OUTOFMEMORY;
        }
        return S_OK;
    }

    static constexpr char const* const registrationXml =
        "< ? xml version = '1.0' ? >"
        "<Effect>"
        "<!--System Properties-->"
        "<Property name = 'DisplayName' type = 'string' value = 'CustomEffect' / >"
        "<Property name = 'Author' type = 'string' value = 'Seven String Software' / >"
        "<Property name = 'Category' type = 'string' value = 'Sample' / >"
        "<Property name = 'Description' type = 'string' value = 'This is a demo effect.' / >"
        "<Inputs/>"
        "<!--Custom Properties go here. -->"
        "</Effect>";
};
