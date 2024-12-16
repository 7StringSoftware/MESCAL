#include "DemoSelectorComponent.h"

DemoSelectorComponent::DemoSelectorComponent() :
    Button("Demo selector")
{
}

void DemoSelectorComponent::paintButton(juce::Graphics& g, bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/)
{
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(hitBox.toFloat(), hitBox.getHeight() * 0.1f);

    g.setColour(juce::Colours::white);
    g.setFont({ hitBox.getHeight() * 0.35f });
    g.drawText("DEMO", hitBox, juce::Justification::centred);
}

void DemoSelectorComponent::setHitBox(juce::Rectangle<int> hitBox_)
{
    hitBox = hitBox_;
}

bool DemoSelectorComponent::hitTest(int x, int y)
{
    return hitBox.contains(x, y);
}

void DemoSelectorComponent::resized()
{
}
