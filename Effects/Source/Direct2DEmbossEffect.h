#pragma once

#include "Direct2DEffect.h"

class Direct2DEmbossEffect : public Direct2DEffect
{
public:
    ~Direct2DEmbossEffect() override = default;

    void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;

private:
};
