#pragma once

#include "Base.h"

struct GridPosition
{
    int row, column;
};

class BezierControlPointComponent : public juce::Button, public juce::ChangeListener
{
public:
    BezierControlPointComponent(GridPosition gridPosition_, std::optional<juce::Colour> color_);
    ~BezierControlPointComponent() override = default;

    void clicked(const ModifierKeys& modifiers) override;
    void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    void moved() override;

    std::function<void()> onMouseEnter;
    std::function<void()> onMove;

    void changeListenerCallback(ChangeBroadcaster* source) override;

    GridPosition const gridPosition;
    std::optional<juce::Colour> color;

private:
    juce::ComponentDragger dragger;
    juce::Component::SafePointer<juce::ColourSelector> colorSelector;
};
