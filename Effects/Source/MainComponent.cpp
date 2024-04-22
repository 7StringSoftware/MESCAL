#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize (600, 400);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint (juce::Graphics& g)
{
    g.drawImageAt(image, 0, 0);
    g.addTransform(juce::AffineTransform::translation(getWidth() * 0.5f, 0.0f));
    embossEffect.applyEffect(image, g, 1.0f, 1.0f);
}

void MainComponent::resized()
{
    image = juce::Image{ juce::Image::PixelFormat::ARGB, getWidth() / 2, getHeight(), true };
    juce::Graphics g{ image };
    g.fillCheckerBoard(getLocalBounds().toFloat(), 20.0f, 20.0f, juce::Colours::white, juce::Colours::lightgrey);

    {
        g.setColour(juce::Colours::darkseagreen);
        juce::Path path;
        path.addStar(image.getBounds().getCentre().toFloat(), 8, 100.0f, 200.0f);
        g.fillPath(path);
    }

    g.setColour(juce::Colours::darkslateblue);
    g.fillEllipse(100.0f, 100.0f, 200.0f, 200.0f);

    g.setColour(juce::Colours::orchid);
    g.fillRoundedRectangle(juce::Rectangle<float>{ 30.0f, 25.0f, 125.0f, 200.0f }, 30.0f);

    {
        g.setColour(juce::Colours::coral);
        juce::Path path;
        path.addCentredArc(getWidth() * 0.15f, getHeight() * 0.45f, 125.0f, 125.0f, 0.0f, 0.35f * juce::MathConstants<float>::pi, 1.15f * juce::MathConstants<float>::pi, true);
        g.strokePath(path, juce::PathStrokeType{ 30.0f });
    }
}
