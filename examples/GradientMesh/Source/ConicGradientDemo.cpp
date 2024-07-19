#include "ConicGradientDemo.h"

ConicGradientDemo::ConicGradientDemo()
{
    std::array<juce::Colour, 7> colors
    {
        juce::Colours::red,
        juce::Colours::orange,
        juce::Colours::yellow,
        juce::Colours::green,
        juce::Colours::blue,
        juce::Colours::indigo,
        juce::Colours::violet
    };

    float angle = 0.0f;
    float angleStep = juce::MathConstants<float>::twoPi / (float)(colors.size() - 1);
    for (auto& color : colors)
    {
        conicGradient.addStop(angle, color);
        angle += angleStep;
    }
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
    auto localBounds = getLocalBounds();
    int size = juce::jmin(localBounds.getWidth(), localBounds.getHeight());
    conicGradientBounds = juce::Rectangle<int>{ size, size }.reduced(100).withCentre(localBounds.getCentre());
    conicGradient.setBounds(conicGradientBounds.withZeroOrigin().toFloat());
}

void ConicGradientDemo::paintMesh(juce::Graphics& g)
{
    if (conicGradientBounds.isEmpty())
        return;

    if (image.isNull() || image.getWidth() != getWidth() || image.getHeight() != getHeight())
        image = juce::Image(juce::Image::ARGB, conicGradientBounds.getWidth(), conicGradientBounds.getHeight(), true);

    conicGradient.draw(image, {});
    g.drawImage(image, conicGradientBounds.toFloat(), juce::RectanglePlacement::centred);
}
