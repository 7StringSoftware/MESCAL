#pragma once

#include "Base.h"
#include "liquidfun/Box2D.h"
#include "LiquidFunRenderer.h"

class BottleDemo : public juce::Component
{
public:
    BottleDemo();
    ~BottleDemo() override;

private:
    void paint(juce::Graphics&) override;
    void resized() override;

    struct LiquidFun
    {
        void createWorld();
        void step(double msec);
        void paint(juce::Graphics& g, juce::AffineTransform const& transform);
        void resize(juce::Rectangle<int> size);

        b2World world{ { 0.0f, -50.0f } };
        b2ParticleSystem* particleSystem = nullptr;
        LiquidFunRenderer renderer;
        juce::Rectangle<float> snifterBowlBoxWorldArea;
        juce::Rectangle<int> paintArea;

        static std::array<juce::Point<float>, 21> constexpr snifterBowlPoints
        {
            juce::Point<float>{ 262.0f, 10.4942f },
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
            { 150.0f, 275.229f },
            { 100.011f, 270.229f },
            { 59.3992f, 258.319f },
            { 31.8452f, 237.769f },
            { 11.2868f, 207.584f },
            { 6.4171f, 171.644f },
            { 9.90173f, 142.282f },
            { 20.0133f, 105.351f },
            { 33.1367f, 69.9504f },
            { 52.414f, 10.4942f }
        };
    } liquidFun;

    double lastTime = Time::getMillisecondCounterHiRes();
    VBlankAttachment vblankAttachment
    {
        this,
        [this]()
        {
            auto now = Time::getMillisecondCounterHiRes();
            auto delta = now - lastTime;
            if (delta >= 30.0)
            {
                lastTime = now;

                liquidFun.step(delta);
                repaint();
            }
        }
    };

    juce::Rectangle<int> animationArea, snifterArea;
    mescal::Effect blurEffect{ mescal::Effect::Type::gaussianBlur };
    mescal::EffectChain shadowEffectChain;
    juce::AffineTransform snifterBackgroundTransform, snifterForegroundTransform, liquidFunTransform;
    juce::Image snifterImage, liquidImage, effectOutputImage;
    std::unique_ptr<juce::Drawable> snifter = juce::Drawable::createFromImageData(BinaryData::Snifter_background_svg, BinaryData::Snifter_background_svgSize);
    std::unique_ptr<juce::Drawable> snifterOutline = juce::Drawable::createFromImageData(BinaryData::Snifter_outline_svg, BinaryData::Snifter_outline_svgSize);
    std::unique_ptr<juce::Drawable> snifterForeground = juce::Drawable::createFromImageData(BinaryData::Snifter_foreground_svg, BinaryData::Snifter_foreground_svgSize);
};
