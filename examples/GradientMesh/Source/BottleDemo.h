#pragma once

#include "Base.h"

class BottleDemo : public juce::Component
{
public:
    BottleDemo();
    ~BottleDemo() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    juce::Path originalBottlePath;
    juce::Path bottleBodyTopPath;
    juce::AffineTransform pathTransform;
    juce::Path transformedBottlePath;
    std::unique_ptr<mescal::GradientMesh> mesh;
    juce::Image meshImage;

    juce::Path splitPath(juce::Path const& p);
};
