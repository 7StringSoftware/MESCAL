#include "ConicGradientDemo.h"

ConicGradientDemo::ConicGradientDemo()
{

}

ConicGradientDemo::~ConicGradientDemo()
{

}

void ConicGradientDemo::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    paintMesh(g);

}

void ConicGradientDemo::resized()
{
    updateConicGradient();
}

void ConicGradientDemo::updateConicGradient()
{   
    juce::ColourGradient gradient = juce::ColourGradient::horizontal(juce::Colours::red, 0.0f, juce::Colours::violet, 1.0f);
     gradient.addColour(0.25, juce::Colours::orange);
     gradient.addColour(0.5, juce::Colours::yellow);
     gradient.addColour(0.75, juce::Colours::yellow);
//     gradient.addColour(0.66, juce::Colours::yellow);

    conicGradient.setColourGradient(gradient);
    auto size = juce::jmin(getWidth() * 0.9f, getHeight() * 0.9f);
    conicGradient.setBounds(juce::Rectangle<float>{ size, size });
}

void ConicGradientDemo::paintMesh(juce::Graphics& g)
{
    if (image.isNull() || image.getWidth() != getWidth() || image.getHeight() != getHeight())
    {
        auto bounds = conicGradient.getBounds().toNearestInt();
        image = juce::Image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    }

    conicGradient.draw(4, image, {});
    g.drawImage(image, getLocalBounds().toFloat(), juce::RectanglePlacement::centred);
}
