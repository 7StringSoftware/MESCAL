#pragma once

#include "MescaIImageEffectFilter.h"
#include "../../EffectGraphDemo\Source/EffectGraphComponent.h"

class ImageEffectFilterDemo  : public juce::Component
{
public:
    ImageEffectFilterDemo()
    {
        setOpaque(true);
        addAndMakeVisible(child);

        addAndMakeVisible(blurAmountSlider);
        blurAmountSlider.setRange({ 0.0, 50.0 }, 0.1);
        blurAmountSlider.setValue(3.0, juce::dontSendNotification);
        blurAmountSlider.onValueChange = [this] 
            { 
                imageEffectFilter.getEffectGraph()->setPropertyValue(mescal::Effect::GaussianBlur::standardDeviation, (float)blurAmountSlider.getValue());
                repaint(); 
            };

        setSize(child.sourceImage.getWidth() * 2, child.sourceImage.getHeight());

        //imageEffectFilter.setEffectGraph(mescal::Effect::create(mescal::Effect::Type::gaussianBlur));
        auto effectGraph = makePerspectiveTransformGraph();
        effectGraph->onPropertyChange = [this](int)
            {
                repaint();
            };
        imageEffectFilter.setEffectGraph(effectGraph);
        child.setComponentEffect(&imageEffectFilter);

        effectGraphDisplay.setOutputEffect(imageEffectFilter.getEffectGraph(), child.sourceImage.getWidth(), child.sourceImage.getHeight());
        addAndMakeVisible(effectGraphDisplay);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour{ 0xff111111 });
        g.drawImageAt(child.sourceImage, 0, 0);
    }

    void resized() override
    {
        child.setBounds(getLocalBounds().removeFromLeft(getWidth() / 2));
        effectGraphDisplay.setBounds(getLocalBounds().removeFromRight(getWidth() / 2));
        blurAmountSlider.setBounds(20, getHeight() - 50, 300, 30);
    }

private:
    MescalImageEffectFilter imageEffectFilter;
    juce::Slider blurAmountSlider{ juce::Slider::LinearHorizontal, juce::Slider::TextBoxLeft };

    auto makePerspectiveTransformGraph()
    {
        auto perspectiveTransform = mescal::Effect::create(mescal::Effect::Type::perspectiveTransform3D);
        return perspectiveTransform;
    }

    auto makeLightingEffectGraph()
    {
        auto luminanceToAlpha = mescal::Effect::create(mescal::Effect::Type::luminanceToAlpha);

#if 0
        auto lighting = mescal::Effect::create(mescal::Effect::Type::spotSpecularLighting);
        lighting->setPropertyValue(mescal::Effect::SpotSpecularLighting::specularExponent, 10.0f);
        lighting->setPropertyValue(mescal::Effect::SpotSpecularLighting::lightPosition, mescal::Vector3{ 0.0f, 0.0f, 100.0f });
        lighting->setPropertyValue(mescal::Effect::SpotSpecularLighting::pointsAt, mescal::Vector3{ (float)child.sourceImage.getWidth() * 0.5f, (float)child.sourceImage.getHeight() * 0.5f, 0.0f});
        lighting->setPropertyValue(mescal::Effect::SpotSpecularLighting::color, juce::Colours::red.withAlpha(1.0f));
        lighting->setInput(0, luminanceToAlpha);
#endif

        auto lighting = mescal::Effect::create(mescal::Effect::Type::spotDiffuseLighting);
        lighting->setPropertyValue(mescal::Effect::SpotDiffuseLighting::lightPosition, mescal::Vector3{ 0.0f, 0.0f, 100.0f });
        lighting->setPropertyValue(mescal::Effect::SpotDiffuseLighting::pointsAt, mescal::Vector3{ (float)child.sourceImage.getWidth() * 0.5f, (float)child.sourceImage.getHeight() * 0.5f, 0.0f });
        lighting->setPropertyValue(mescal::Effect::SpotDiffuseLighting::color, juce::Colours::red.withAlpha(1.0f));
        lighting->setInput(0, luminanceToAlpha);

//         auto gaussianBlur = mescal::Effect::create(mescal::Effect::Type::gaussianBlur);
//         gaussianBlur->setInput(0, lighting);

//         auto alphaMaskEffect = mescal::Effect::create(mescal::Effect::Type::alphaMask);
//         alphaMaskEffect->setInput(0, gaussianBlur);

        return lighting;
    }

    struct Child : public juce::Component
    {
        Child()
        {
            setOpaque(false);
        }

        void paint(juce::Graphics& g) override
        {
            //
            // Paint the image
            //
            g.drawImageAt(sourceImage, 0, 0);
        }

        juce::Image const sourceImage = juce::ImageFileFormat::loadFrom(BinaryData::Buttons_png, BinaryData::Buttons_pngSize);
    } child;

    EffectGraphComponent effectGraphDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageEffectFilterDemo)
};
