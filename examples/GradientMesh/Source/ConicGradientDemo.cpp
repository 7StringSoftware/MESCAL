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
    conicGradient.setBounds(juce::Rectangle<float>{ getWidth() * 0.9f, getHeight() * 0.9f}.withCentre(getLocalBounds().getCentre().toFloat()));
}

void ConicGradientDemo::paintMesh(juce::Graphics& g)
{
    if (image.isNull() || image.getWidth() != getWidth() || image.getHeight() != getHeight())
    {
        image = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    }

    conicGradient.draw(32, image, {});
    g.drawImageAt(image, 0, 0);
}
