#pragma once

#include <JuceHeader.h>

class EffectGraph
{
public:
	EffectGraph();
    /// <summary>
    ///
    /// </summary>
    void paintSlider();
    void createSliderEffectGraph();

    void paintMetallicKnobImage(float angle);
    void createMetallicKnobEffectGraph();
    void paint3DButtonImages();
    void create3DButtonEffectGraph();
    void buildComponentEffectGraph();

    std::vector<juce::Image> sourceImages;
	mescal::Effect::Ptr outputEffect;

    mescal::Effect::Ptr addShadow(juce::Image const& sourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform);
    mescal::Effect::Ptr createInnerShadow(juce::Image const& sourceImage, juce::Colour const& shadowColor, float shadowSize, juce::AffineTransform transform);
};
