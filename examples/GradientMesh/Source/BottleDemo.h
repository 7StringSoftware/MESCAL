#pragma once

#include "Base.h"

class BottleDemo : public juce::Component
{
public:
    BottleDemo();
    ~BottleDemo() override;

private:
    void paint(juce::Graphics&) override;
    void resized() override;

    std::array<juce::Point<float>, 21> snifterBowlPoints
    {
        juce::Point<float>{ 100.011f, 270.229f },
        { 59.3992f, 258.319f },
        { 31.8452f, 237.769f },
        { 11.2868f, 207.584f },
        { 6.4171f, 171.644f },
        { 9.90173f, 142.282f },
        { 20.0133f, 105.351f },
        { 33.1367f, 69.9504f },
         { 52.414f, 10.4942f },
         { 262.0f, 10.4942f },
        { 278.847f, 53.6437f },
        { 292.235f, 85.9187f },
        { 307.035f, 122.144f },
        { 315.416f, 156.097f },
        { 315.009f, 184.403f },
        { 305.068f, 212.644f },
        { 290.483f, 232.512f },
        { 272.845f, 247.83f },
        { 245.557f, 260.325f },
        { 200.0f, 270.229f },
        { 150.0f, 275.229f }
    };



    std::unique_ptr<mescal::GradientMesh> mesh;
    mescal::Effect effect{ mescal::Effect::Type::gaussianBlur };
    juce::AffineTransform transform;
    juce::Image meshImage;
    juce::Image effectImage;
    std::unique_ptr<juce::Drawable> snifter = juce::Drawable::createFromImageData(BinaryData::Snifter_Glass_Brandy_svg, BinaryData::Snifter_Glass_Brandy_svgSize);
    std::unique_ptr<juce::Drawable> snifterForeground = juce::Drawable::createFromImageData(BinaryData::Snifter_foreground_svg, BinaryData::Snifter_foreground_svgSize);
    std::unique_ptr<juce::Drawable> snifterBowl = juce::Drawable::createFromImageData(BinaryData::Snifter_bowl_svg, BinaryData::Snifter_bowl_svgSize);
    juce::Path outline = snifterBowl->getOutlineAsPath();

    juce::Path splitPath(juce::Path const& p);
};
