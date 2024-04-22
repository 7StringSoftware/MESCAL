#pragma once

#include "Direct2DEffect.h"

class Direct2DEmbossEffect : public Direct2DEffect
{
public:
    Direct2DEmbossEffect();
    ~Direct2DEmbossEffect() override;

private:
    struct EmbossPimpl;
    std::unique_ptr<EmbossPimpl> pimpl;
    Pimpl* getPimpl() const noexcept override;
};
