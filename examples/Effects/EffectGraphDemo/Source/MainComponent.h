#pragma once

#include "EffectGraph.h"
#include "EffectGraphComponent.h"
namespace juce
{
#define PIP_JUCE_EXAMPLES_DIRECTORY_STRING  "C:\\JUCE-fork\\examples"
#include "C:\JUCE-fork\examples\GUI\WidgetsDemo.h"
}

class DemoEffectFilter : public mescal::MescalImageEffectFilter
{
public:
    DemoEffectFilter(mescal::Effect::Ptr effect_) :
        MescalImageEffectFilter(effect_)
    {
    }

    void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;

    std::function<void(const juce::Image& sourceImage)> onApplyEffect;
};

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    double angle = 0.0;

    //juce::WidgetsDemo widgetsDemo;
    struct ButtonHolder : public juce::Component
    {
        ButtonHolder()
        {
            addAndMakeVisible(effectButton);
        }

        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colours::white);
        }

        void resized() override
        {
            effectButton.setBounds(getLocalBounds().reduced(100));
        }

        struct EffectButton : public juce::Button
        {
            EffectButton() : Button("EffectButton")
            {
                setClickingTogglesState(true);
            }

            void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
            {
                if (image.isNull() || image.getWidth() != getWidth() || image.getHeight() != getHeight())
                {
                    image = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true };
                }

                effectGraph.outputEffect->applyEffect(image, juce::RectanglePlacement{}.getTransformToFit(image.getBounds().toFloat(), getLocalBounds().toFloat()), true);
                g.drawImageTransformed(image, juce::RectanglePlacement{}.getTransformToFit(image.getBounds().toFloat(), getLocalBounds().toFloat()));
            }

            EffectGraph effectGraph;
            juce::Image image;
            juce::VBlankAttachment vblank{ this, [this] { repaint(); } };

        } effectButton;
    } buttonHolder;

    std::unique_ptr<EffectGraphComponent> effectGraphComponent;
    juce::Image snapshot;
    DemoEffectFilter imageEffectFilter{ buttonHolder.effectButton.effectGraph.outputEffect };
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    juce::VBlankAttachment vblank{ this, [this] { animate(); } };

    void animate();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
