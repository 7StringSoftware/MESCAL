#include "MainComponent.h"

MainComponent::MainComponent() : displayComponent(*this),
spotSpecularLightingControlComponent(effect.get())
{
    addAndMakeVisible(displayComponent);
    addAndMakeVisible(spotSpecularLightingControlComponent);

    spotSpecularLightingControlComponent.onEffectChange = [this]
        {
            updateEffect();
        };

    setSize(1024, 1024);
    setRepaintsOnMouseActivity(true);

    setOpaque(true);
}

MainComponent::~MainComponent()
{
}

void MainComponent::initEffect()
{
    spotSpecularLightingControlComponent.initEffect();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void MainComponent::resized()
{
    int controlH = 50;
    displayComponent.setBounds(getLocalBounds().withTrimmedBottom(controlH));
    spotSpecularLightingControlComponent.setBounds(getLocalBounds());

    auto area = displayComponent.getLocalBounds().reduced(6);

    input = juce::Image{ juce::Image::PixelFormat::ARGB, area.getWidth(), area.getHeight(), true };
    //input = juce::ImageFileFormat::loadFrom(BinaryData::A_Sunday_on_La_Grande_Jatte_Georges_Seurat_1884_jpg, BinaryData::A_Sunday_on_La_Grande_Jatte_Georges_Seurat_1884_jpgSize);
#if 1

    {
        juce::Graphics g{ input };

        g.fillCheckerBoard(getLocalBounds().toFloat(), 20.0f, 20.0f, juce::Colours::darkgrey, juce::Colours::lightgrey.withAlpha(0.1f));

#if 0
        {
            g.setColour(juce::Colours::darkseagreen.withAlpha(1.0f));
            juce::Path path;
            path.addStar(input.getBounds().getCentre().toFloat(), 8, 100.0f, 200.0f);
            g.fillPath(path);
        }

        g.setColour(juce::Colours::darkslateblue.withAlpha(1.0f));
        g.fillEllipse(100.0f, 100.0f, 200.0f, 200.0f);

        g.setColour(juce::Colours::orchid);
        g.fillRoundedRectangle(juce::Rectangle<float>{ 30.0f, 25.0f, 125.0f, 200.0f }, 30.0f);

        {
            g.setColour(juce::Colours::coral);
            juce::Path path;
            path.addCentredArc(getWidth() * 0.15f, getHeight() * 0.45f, 125.0f, 125.0f, 0.0f, 0.35f * juce::MathConstants<float>::pi, 1.15f * juce::MathConstants<float>::pi, true);
            g.strokePath(path, juce::PathStrokeType{ 30.0f });
        }
#endif
    }
#endif

    applyEffect();
}

void MainComponent::updateEffect()
{
    applyEffect();
    repaint();
}

void MainComponent::applyEffect()
{
    if (input.isNull() || input.getBounds().isEmpty())
    {
        return;
    }

    if (output.isNull() || output.getWidth() != input.getWidth() || output.getHeight() != input.getHeight())
    {
        output = juce::Image{ juce::Image::PixelFormat::ARGB, input.getWidth(), input.getHeight(), true };
    }

    effect->applyEffect(input, output, 1.0f, 1.0f);
}

void MainComponent::DisplayComponent::paint(juce::Graphics& g)
{
    g.drawImageWithin(owner.input, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::centred, false);
    g.drawImageWithin(owner.output, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::centred, false);
    //int x = (getWidth() - owner.input.getWidth()) / 2;
    //int y = (getHeight() - owner.input.getHeight()) / 2;
    //g.drawImageAt(owner.input, x, y);
    //g.drawImageAt(owner.output, x, y);
}
