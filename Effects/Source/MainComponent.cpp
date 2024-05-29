#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize (900, 400);
    setRepaintsOnMouseActivity(true);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.drawImageAt(input, 0, 0);
    g.drawImageAt(blurred, input.getWidth(), 0);

    auto mousePos = getMouseXYRelative();
    g.beginTransparencyLayer(0.25f);
    g.drawImageAt(output, input.getWidth() * 0, 0);
    g.endTransparencyLayer();
}

void MainComponent::resized()
{
    input = juce::Image{ juce::Image::PixelFormat::ARGB, getWidth() / 3, getHeight(), true };

    {
	    juce::Graphics g{ input };
	    g.fillCheckerBoard(input.getBounds().toFloat(), 20.0f, 20.0f, juce::Colours::white.withAlpha(0.5f), juce::Colours::red.withAlpha(0.0f));

#if 0
	    {
	        g.setColour(juce::Colours::darkseagreen.withAlpha(1.0f));
	        juce::Path path;
	        path.addStar(input.getBounds().getCentre().toFloat(), 8, 100.0f, 200.0f);
	        g.fillPath(path);
	    }

	    g.setColour(juce::Colours::darkslateblue.withAlpha(0.5f));
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

    blurred = juce::Image{ juce::Image::PixelFormat::ARGB, input.getWidth(), input.getHeight(), true };
    //blur->applyEffect(input, output, 1.0f, 1.0f);
    {
        juce::Graphics g{ blurred };
        blur->applyEffect(input, g, 1.0f, 1.0f);
    }

    output = juce::Image{ juce::Image::PixelFormat::ARGB, input.getWidth(), input.getHeight(), true };
    {
        juce::Graphics g{ output };
        effect->applyEffect(blurred, g, 1.0f, 1.0f);
    }
    //
//     input = output.createCopy();
//     output.clear(output.getBounds(), juce::Colours::transparentBlack);
//     effect->applyEffect(input, output, 1.0f, 1.0f);
}
