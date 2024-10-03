#pragma once

class GPUConfig
{
public:
    GPUConfig();
    ~GPUConfig();

    void requestMaximumTextureMemory(uint64_t maxiumTextureMemory_);
    auto const& getMaximumTextureMemory() const noexcept
    {
        return currentMaxTextureMemoryArray;
    }
    
private:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    uint64_t requestedMaxiumTextureMemory = 0;
    std::vector<uint64_t> currentMaxTextureMemoryArray;
};
