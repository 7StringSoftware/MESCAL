#include "WidgetsComponent.h"

WidgetsComponent::WidgetsComponent()
{
    //setLookAndFeel(&mescalLookAndFeel);

    for (int index = 0; index < 8; ++index)
    {
        auto& button = buttons.emplace_back(std::make_unique<juce::TextButton>(juce::String{index}));
        addAndMakeVisible(button.get());
    }

    addAndMakeVisible(rotarySlider);
    addAndMakeVisible(verticalSlider);
    addAndMakeVisible(horizontalSlider);

    setSize(768, 768);
}

WidgetsComponent::~WidgetsComponent()
{
    setLookAndFeel(nullptr);
}

void WidgetsComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void WidgetsComponent::resized()
{
//     rotarySlider.setBounds(0, 0, getWidth() / 2, getHeight() / 2);
//     verticalSlider.setBounds(rotarySlider.getRight() + 10, 0, 100, getHeight() / 2);

    juce::Rectangle<int> r{20, 200, 40, 20 };
    for (auto& textButton : buttons)
    {
        textButton->setBounds(r);
        r.expand(20, 10);

        if (r.getRight() >= getWidth())
        {
            r.setX(0);
            r.translate(0, 200);
        }
        else
        {

            r.setX(r.getRight());
        }
    }

    horizontalSlider.setBounds(10, 600, getWidth() - 20, 40);
}
