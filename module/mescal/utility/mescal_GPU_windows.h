#pragma once

class GPU
{
public:
    GPU();
    ~GPU();

    juce::String getName() const noexcept;
    size_t getDedicatedVideoMemory() const noexcept;
    size_t getDedicatedSystemMemory() const noexcept;
    size_t getSharedSystemMemory() const noexcept;
    uint64_t getUniqueID() const noexcept;

    static std::vector<std::unique_ptr<GPU>> getAvailableGPUs();

    std::function<void(uint64_t)> onAdapterCreated;
    std::function<void(uint64_t)> onAdapterRemoved;

    struct ProcessTextureMemory
    {
        ProcessTextureMemory();
        ~ProcessTextureMemory();

        void requestMaximumTextureMemory(uint64_t maxiumTextureMemoryBytes_);
        auto const& getMaximumTextureMemory() const noexcept
        {
            return currentMaxTextureMemoryBytesArray;
        }

    private:
        struct ProcessTextureMemoryInternal;
        std::unique_ptr<ProcessTextureMemoryInternal> internal;

        uint64_t requestedMaxiumTextureMemoryBytes = 0;
        std::vector<uint64_t> currentMaxTextureMemoryBytesArray;

        void updateTextureMemory();
    };

private:
    struct GPUInternal;
    std::unique_ptr<GPUInternal> internal;
};
