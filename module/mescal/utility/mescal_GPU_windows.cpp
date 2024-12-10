namespace mescal
{
    struct GPU::GPUInternal
    {
        GPUInternal(juce::DxgiAdapter::Ptr adapterIn)
        {
            if (adapterIn && adapterIn->dxgiAdapter)
            {
                DXGI_ADAPTER_DESC1 desc{};
                adapterIn->dxgiAdapter->GetDesc1(&desc);
                name = juce::String{ desc.Description };
                dedicatedVideoMemory = desc.DedicatedVideoMemory;
                dedicatedSystemMemory = desc.DedicatedSystemMemory;
                sharedSystemMemory = desc.SharedSystemMemory;
                uniqueID = desc.AdapterLuid.LowPart | (static_cast<uint64_t>(desc.AdapterLuid.HighPart) << 32);
            }
        }

        ~GPUInternal() {}

        juce::String name;
        size_t dedicatedVideoMemory = 0;
        size_t dedicatedSystemMemory = 0;
        size_t sharedSystemMemory = 0;
        uint64_t uniqueID = 0;
    };

    GPU::GPU() = default;
    GPU::~GPU() = default;

    juce::String GPU::getName() const noexcept
    {
        return internal->name;
    }

    size_t GPU::getDedicatedVideoMemory() const noexcept
    {
        return internal->dedicatedVideoMemory;
    }

    size_t GPU::getDedicatedSystemMemory() const noexcept
    {
        return internal->dedicatedSystemMemory;
    }

    size_t GPU::getSharedSystemMemory() const noexcept
    {
        return internal->sharedSystemMemory;
    }

    uint64_t GPU::getUniqueID() const noexcept
    {
        return internal->uniqueID;
    }

    std::vector<std::unique_ptr<GPU>> GPU::getAvailableGPUs()
    {
        std::vector<std::unique_ptr<GPU>> gpus;
        juce::SharedResourcePointer<juce::DirectX> directX;

        for (juce::DxgiAdapter::Ptr adapter : directX->adapters.getAdapterArray())
        {
            gpus.emplace_back(std::make_unique<GPU>());
            gpus.back()->internal = std::make_unique<GPUInternal>(adapter);
        }

        return gpus;
    }

    struct GPU::ProcessTextureMemory::ProcessTextureMemoryInternal : public juce::DxgiAdapterListener
    {
        ProcessTextureMemoryInternal(GPU::ProcessTextureMemory& owner_) :
            owner(owner_)
        {
            directX->adapters.addListener(*this);
        }

        ~ProcessTextureMemoryInternal()
        {
            directX->adapters.removeListener(*this);
        }

        void adapterCreated(juce::DxgiAdapter::Ptr adapter) override
        {
            if (adapter->direct2DDevice)
            {
                adapter->direct2DDevice->SetMaximumTextureMemory(owner.requestedMaxiumTextureMemoryBytes);
            }
        }

        void adapterRemoved(juce::DxgiAdapter::Ptr) override {}

        juce::SharedResourcePointer<juce::DirectX> directX;
        GPU::ProcessTextureMemory& owner;
    };

    GPU::ProcessTextureMemory::ProcessTextureMemory() :
        internal(std::make_unique<ProcessTextureMemoryInternal>(*this))
    {
    }

    GPU::ProcessTextureMemory::~ProcessTextureMemory() {}

    void GPU::ProcessTextureMemory::requestMaximumTextureMemory(uint64_t maxiumTextureMemory_)
    {
        requestedMaxiumTextureMemoryBytes = maxiumTextureMemory_;

        updateTextureMemory();
    }

    void GPU::ProcessTextureMemory::updateTextureMemory()
    {
        currentMaxTextureMemoryBytesArray.clear();

        for (auto& adapter : internal->directX->adapters.getAdapterArray())
        {
            internal->adapterCreated(adapter);

            if (adapter->direct2DDevice)
            {
                currentMaxTextureMemoryBytesArray.push_back(adapter->direct2DDevice->GetMaximumTextureMemory());
            }
        }
    }
}

