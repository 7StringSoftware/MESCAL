namespace mescal
{
    struct GPUConfig::Pimpl : public juce::DxgiAdapterListener
    {
        Pimpl(GPUConfig& owner_) :
            owner(owner_)
        {
            directX->adapters.addListener(*this);
        }

        ~Pimpl()
        {
            directX->adapters.removeListener(*this);
        }

        void adapterCreated(juce::DxgiAdapter::Ptr adapter) override
        {
            if (adapter->direct2DDevice)
            {
                adapter->direct2DDevice->SetMaximumTextureMemory(owner.requestedMaxiumTextureMemory);
            }
        }

        void adapterRemoved(juce::DxgiAdapter::Ptr) override
        {
        }

        juce::SharedResourcePointer<juce::DirectX> directX;
        GPUConfig& owner;
    };

    GPUConfig::GPUConfig() :
        pimpl(std::make_unique<Pimpl>(*this))
    {
    }

    GPUConfig::~GPUConfig() {}

    void GPUConfig::requestMaximumTextureMemory(uint64_t maxiumTextureMemory_)
    {
        requestedMaxiumTextureMemory = maxiumTextureMemory_;

        currentMaxTextureMemoryArray.clear();

        for (auto& adapter : pimpl->directX->adapters.getAdapterArray())
        {
            pimpl->adapterCreated(adapter);

            if (adapter->direct2DDevice)
            {
                currentMaxTextureMemoryArray.push_back(adapter->direct2DDevice->GetMaximumTextureMemory());
            }
        }
    }
}

