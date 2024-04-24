/*
  ==============================================================================

    ControlPointComponent.cpp
    Created: 23 Apr 2024 8:52:40am
    Author:  Matt Gonzalez

  ==============================================================================
*/

#include "ControlPointComponent.h"


ControlPointComponent::ControlPointComponent(GridPosition gridPosition_, std::optional<juce::Colour> color_)
    : Button({}),
    gridPosition(gridPosition_),
    color(color_)
{
}

void ControlPointComponent::clicked(const ModifierKeys& modifiers)
{
    if (! color.has_value())
    {
        return;
    }

    auto content = std::make_unique<juce::ColourSelector>();
    content->addChangeListener(this);
    colorSelector = content.get();
    content->setCurrentColour(*color, juce::dontSendNotification);
    content->setSize(300, 300);

    juce::CallOutBox::launchAsynchronously(std::move(content),
        getScreenBounds(),
        nullptr);
}

void ControlPointComponent::paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    float radius = juce::jmin(getWidth() * 0.45f, getHeight() * 0.45f);
    if (shouldDrawButtonAsDown)
        radius *= 0.9f;

    bool showColorWheel = color.has_value() && (shouldDrawButtonAsHighlighted || colorSelector != nullptr);
    if (showColorWheel)
    {
        const std::array<juce::Colour, 7> colors =
        {
            juce::Colours::red,
            juce::Colours::orange,
            juce::Colours::yellow,
            juce::Colours::green,
            juce::Colours::blue,
            juce::Colours::indigo,
            juce::Colours::violet
        };

        juce::Path p;
        float angleStep = juce::MathConstants<float>::twoPi / (float)colors.size();
        auto center = getLocalBounds().getCentre().toFloat();
        p.startNewSubPath(center);
        p.lineTo(center - juce::Point<float>{ 0.0f, radius });
        p.addCentredArc(getWidth() * 0.5f, getHeight() * 0.5f, radius, radius, 0.0f, 0.0f, angleStep, false);
        p.closeSubPath();

        float angle = 0.0f;
        for (auto const& color : colors)
        {
            g.setColour(color);
            g.fillPath(p, juce::AffineTransform::rotation(angle, center.x, center.y));
            angle += angleStep;
        }

        radius *= 0.7f;
    }

    auto c = color.has_value() ? *color : juce::Colours::white;
    g.setColour(c);

    auto ellipseBounds = getLocalBounds().toFloat().withSizeKeepingCentre(radius * 2.0f, radius * 2.0f);
    g.fillEllipse(ellipseBounds);
    g.setColour(c.contrasting());
    g.drawText(getName(), getLocalBounds(), juce::Justification::centred);

    if (!showColorWheel)
    {
        auto thickness = shouldDrawButtonAsHighlighted ? 3.0f : 1.0f;
        g.drawEllipse(ellipseBounds, thickness);
    }
}

void ControlPointComponent::mouseDown(const juce::MouseEvent& e)
{
    Button::mouseDown(e);

    dragger.startDraggingComponent(this, e);
}

void ControlPointComponent::mouseDrag(const juce::MouseEvent& e)
{
    Button::mouseDrag(e);

    dragger.dragComponent(this, e, nullptr);
}

void ControlPointComponent::mouseUp(const juce::MouseEvent& e)
{
    if (e.getDistanceFromDragStart() >= 5.0f)
    {
        return;
    }

    Button::mouseUp(e);
}

void ControlPointComponent::moved()
{
    if (onMove)
    {
        onMove();
    }
}

void ControlPointComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (colorSelector)
    {
        *color = colorSelector->getCurrentColour();
        moved();
    }
}
