#pragma once

#include "Direct2DEffect.h"

class Direct2DEdgeDetectionEffect : public Direct2DEffect
{
public:
    ~Direct2DEdgeDetectionEffect() override = default;

    void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;

private:
};
