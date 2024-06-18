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
    g.drawImageAt(output, getWidth() / 2, 0);
}

void MainComponent::resized()
{
    input = juce::Image{ juce::Image::PixelFormat::ARGB, getWidth() / 2, getHeight(), true };

    {
	    juce::Graphics g{ input };
	    g.fillCheckerBoard(getLocalBounds().toFloat(), 20.0f, 20.0f, juce::Colours::darkgrey, juce::Colours::transparentBlack);

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
    }

    output = juce::Image{ juce::Image::PixelFormat::ARGB, getWidth() / 2, getHeight(), true };

    {
        juce::Graphics g{ output };
        effect.applyEffect(input, g, 1.0f, 1.0f);
    }

    {
        juce::Image::BitmapData bitmapData{ output, juce::Image::BitmapData::ReadWriteMode::readWrite };
        for (auto x = 0; x < bitmapData.width; ++x)
        {
            for (auto y = 0; y < bitmapData.height; ++y)
            {
                auto pixel = bitmapData.getPixelColour(x, y);
                if (pixel == juce::Colours::black)
                {
                    bitmapData.setPixelColour(x, y, juce::Colours::transparentBlack);
                }
            }
        }
    }

    auto temp = input.createCopy();
    {
        juce::Graphics g{ temp };
        g.setTiledImageFill(output, 0, 0, 1.0f);
        g.fillRect(temp.getBounds());
    }

    output = temp;
}
