#pragma once

#include <JuceHeader.h>

class EffectGraph
{
public:
	EffectGraph();

    void paintMetallicKnobImage(float angle);
    void createMetallicKnobEffectGraph();
    void paint3DButtonImages();
    void create3DButtonEffectGraph();

    std::vector<juce::Image> sourceImages;
	mescal::Effect::Ptr outputEffect;
};
