
namespace mescal
{
    struct DirectXResources
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
}
