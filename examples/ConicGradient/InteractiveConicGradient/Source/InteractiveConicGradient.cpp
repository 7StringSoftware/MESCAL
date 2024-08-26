#include <JuceHeader.h>
#include "InteractiveConicGradient.h"

static float normalizeAngle(float angle)
{
    while (angle < 0.0f)
        angle += juce::MathConstants<float>::twoPi;

    while (angle > juce::MathConstants<float>::twoPi)
        angle -= juce::MathConstants<float>::twoPi;

    return angle;
}

InteractiveConicGradient::InteractiveConicGradient()
{
    presetCombo.addItem("Roy G. Biv", royGBiv);
    presetCombo.addItem("HSV", hsv);
    presetCombo.addItem("Grayscale", grayscale);
    addAndMakeVisible(presetCombo);
    presetCombo.onChange = [this]()
        {
            setGradientStops(static_cast<PresetType>(presetCombo.getSelectedId()), static_cast<Direction>(directionCombo.getSelectedId()));
            createSliders();
            resized();
            updateSliders();
            repaint();
        };
    presetCombo.setSelectedId(grayscale, juce::dontSendNotification);

    directionCombo.addItem("Clockwise", clockwise);
    directionCombo.addItem("Counterclockwise", counterclockwise);
    addAndMakeVisible(directionCombo);
    directionCombo.onChange = presetCombo.onChange;
    directionCombo.setSelectedId(clockwise, juce::dontSendNotification);

    setGradientStops(grayscale, clockwise);

    createSliders();
    updateSliders();

	setSize(768, 768);
}

void InteractiveConicGradient::createSliders()
{
    sliders.clear();

    auto const& stops = conicGradient.getStops();
    for (size_t index = 0; index < stops.size(); ++index)
    {
        auto slider = std::make_unique<ArcSlider>(*this, index);
        slider->setName(juce::String{ index });
        addAndMakeVisible(slider.get());

        juce::Component::SafePointer<ArcSlider> safeSlider = slider.get();
        slider->onChange = [this](size_t index, float angle)
            {
                conicGradient.setStopAngle(index, angle);
                updateSliders();
            };

        sliders.push_back(std::move(slider));
    }
}

void InteractiveConicGradient::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    paintConicGradient(g);
}

void InteractiveConicGradient::resized()
{
    auto localBounds = getLocalBounds();
    int size = juce::jmin(localBounds.getWidth(), localBounds.getHeight());
    outerRadius = 0.4f * (float)size;
    conicGradient.setRadiusRange({ outerRadius * 0.75f, outerRadius});

    auto stopIterator = conicGradient.getStops().begin();
    auto dragRadius = outerRadius + 25.0f;
    for (auto& slider : sliders)
    {
        slider->setAngle(stopIterator->angle);
        slider->dragRadius = dragRadius;
        ++stopIterator;
    }

    updateSliders();

    int w = 200;
    int h = 34;
    presetCombo.setBounds(getWidth() - w - 10, 10, w, h);
    directionCombo.setBounds(presetCombo.getBounds().translated(0, h + 10));
}

void InteractiveConicGradient::setGradientStops(PresetType presetType, Direction direction)
{
    float sign = direction == clockwise ? 1.0f : -1.0f;

    conicGradient.clearStops();

    switch (presetType)
    {
    case royGBiv:
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

        float startAngle = 0.0f;
        float angle = startAngle;
        float angleStep = sign * juce::MathConstants<float>::twoPi / (float)(colors.size() - 1);
        for (auto& color128 : colors)
        {
            conicGradient.addStop(angle, color128);
            angle += angleStep;
        }

        conicGradient.setStopAngle(colors.size() - 1, sign * (startAngle + juce::MathConstants<float>::twoPi));
        break;
    };

    case hsv:
    {
        int numSteps = 16;
        for (float hue = 0.0f; hue <= 1.0f; hue += 1.0f / (float)numSteps)
        {
            conicGradient.addStop(sign * hue * juce::MathConstants<float>::twoPi, mescal::Color128::fromHSV(hue, 1.0f, 1.0f, 1.0f));
        }
        break;
    }

    case grayscale:
    {
        for (float level = 0.0f; level <= 1.0f; level += 0.125f)
        {
            conicGradient.addStop(sign * level * juce::MathConstants<float>::twoPi, mescal::Color128::grayLevel(level + 0.25f));
        }
        break;
    }
    }
}

void InteractiveConicGradient::updateSliders()
{
    auto const& stops = conicGradient.getStops();

    auto sliderRangeStartAngle = normalizeAngle(stops.back().angle);
    for (size_t index = 0; index < stops.size(); ++index)
    {
        auto& stop = stops[index];
        auto& slider = sliders[index];

        auto stopAngle = stop.angle;
        while (stopAngle < sliderRangeStartAngle)
            stopAngle += juce::MathConstants<float>::twoPi;

        auto sliderRangeEndAngle = stopAngle;
        if (index == stops.size() - 1)
        {
            sliderRangeEndAngle = stops.front().angle;
        }
        else
        {
            auto& nextStop = stops[index + 1];
            sliderRangeEndAngle = nextStop.angle;
        }
        while (sliderRangeEndAngle < stopAngle)
            sliderRangeEndAngle += juce::MathConstants<float>::twoPi;

        stopAngle = juce::jlimit(sliderRangeStartAngle, sliderRangeEndAngle, stopAngle);

        slider->arcRange = { sliderRangeStartAngle, sliderRangeEndAngle };

        conicGradient.setStopAngle(index, stopAngle);
        slider->setAngle(stopAngle);

        sliderRangeStartAngle = stopAngle;
    }

    repaint();
}

void InteractiveConicGradient::paintConicGradient(juce::Graphics& g)
{
    if (getLocalBounds().isEmpty())
        return;

    if (image.isNull() || image.getWidth() != getWidth() || image.getHeight() != getHeight())
        image = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);

    conicGradient.draw(image, juce::AffineTransform{}.translated(getLocalBounds().toFloat().getCentre()));
    g.drawImage(image, getLocalBounds().toFloat(), juce::RectanglePlacement::centred);
}

InteractiveConicGradient::ArcSlider::ArcSlider(InteractiveConicGradient& owner_, size_t index_) :
    owner(owner_),
    index(index_)
{
    setRepaintsOnMouseActivity(true);
    setSize(30, 30);
}

void InteractiveConicGradient::ArcSlider::setAngle(float newAngle)
{
    if (owner.getBounds().isEmpty())
        return;

    auto center = owner.getLocalBounds().getCentre().toFloat();
    auto const& stop = owner.conicGradient.getStops()[index];

    setCentrePosition(center.getPointOnCircumference(dragRadius, newAngle).toInt());

    if (juce::approximatelyEqual(normalizeAngle(stop.angle), normalizeAngle(newAngle)))
        return;

    if (onChange)
        onChange(index, newAngle);
}

void InteractiveConicGradient::ArcSlider::paint(juce::Graphics& g)
{
    float reduction = isMouseOver() || isMouseButtonDown(false) ? 0.0f : 4.0f;
    auto const& stop = owner.conicGradient.getStops()[index];
    auto c = stop.color128.toColour();
    g.setColour(c.contrasting());
    g.fillEllipse(getLocalBounds().toFloat().reduced(reduction));
    g.setColour(c);
    g.fillEllipse(getLocalBounds().toFloat().reduced(reduction + 2.0f));
}

void InteractiveConicGradient::ArcSlider::mouseDown(const juce::MouseEvent& e)
{
    auto center = owner.getLocalBounds().getCentre().toFloat();
    mouseDragStartAngle = center.getAngleToPoint(e.getEventRelativeTo(&owner).position);
    startAngle = owner.conicGradient.getStops()[index].angle;
}

void InteractiveConicGradient::ArcSlider::mouseDrag(const juce::MouseEvent& e)
{
    auto center = owner.getLocalBounds().getCentre().toFloat();
    auto mouseDragAngle = center.getAngleToPoint(e.getEventRelativeTo(&owner).position);
    auto delta = mouseDragAngle - mouseDragStartAngle;

    while (delta < -juce::MathConstants<float>::pi)
        delta += juce::MathConstants<float>::twoPi;

    while (delta > juce::MathConstants<float>::pi)
        delta -= juce::MathConstants<float>::twoPi;

    auto angle = startAngle + delta;

    angle = arcRange.clipValue(angle);

    setAngle(angle);
}
