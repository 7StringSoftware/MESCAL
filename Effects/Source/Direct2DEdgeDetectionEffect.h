#pragma once

#include "Direct2DEffect.h"

class Direct2DEdgeDetectionEffect : public Direct2DEffect
{
public:
    Direct2DEdgeDetectionEffect();
    ~Direct2DEdgeDetectionEffect() override;

private:
    struct EdgeDetectionPimpl;
    std::unique_ptr<EdgeDetectionPimpl> pimpl;
    Pimpl* getPimpl() const noexcept override;
};
