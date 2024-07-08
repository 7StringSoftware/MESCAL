#pragma once

#include "Base.h"

class BottleDemo : public juce::Component
{
public:
    BottleDemo();
    ~BottleDemo() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    std::unique_ptr<mescal::GradientMesh> mesh;
    mescal::Effect effect{ mescal::Effect::Type::spotSpecularLighting };
    juce::AffineTransform transform;
    juce::Image meshImage;
    juce::Image effectImage;

    juce::Path splitPath(juce::Path const& p);
};
