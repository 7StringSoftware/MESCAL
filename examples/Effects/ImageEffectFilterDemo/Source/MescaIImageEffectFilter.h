#pragma once

#include <JuceHeader.h>

class MescalImageEffectFilter : public juce::ImageEffectFilter
{
public:
    MescalImageEffectFilter() = default;
    MescalImageEffectFilter(mescal::Effect::Ptr effectGraph_) :
        effectGraph(effectGraph_)
    {
    }

    void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;

    void setEffectGraph(mescal::Effect::Ptr newEffectGraph) { effectGraph = newEffectGraph; }
    mescal::Effect* const getEffectGraph() { return effectGraph.get(); }

private:
    mescal::Effect::Ptr effectGraph;
    juce::Image outputImage;
};
