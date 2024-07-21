#include "ConicGradientDemo.h"


static float normalizeAngle(float angle)
{
    while (angle < 0.0f)
        angle += juce::MathConstants<float>::twoPi;

    while (angle > juce::MathConstants<float>::twoPi)
        angle -= juce::MathConstants<float>::twoPi;

    return angle;
}

ConicGradientDemo::ConicGradientDemo()
{
    updateConicGradient();

    auto const& stops = conicGradient.getStops();
    for (size_t index = 0; index < stops.size(); ++index)
    {
        auto const& stop = stops[index];
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

    updateSliders();
}

void ConicGradientDemo::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    paintConicGradient(g);
}

void ConicGradientDemo::resized()
{
    auto localBounds = getLocalBounds();
    int size = juce::jmin(localBounds.getWidth(), localBounds.getHeight());
    conicGradientBounds = juce::Rectangle<int>{ size, size }.reduced(100).withCentre(localBounds.getCentre());
    conicGradient.setBounds(conicGradientBounds.withZeroOrigin().toFloat());

    auto stopIterator = conicGradient.getStops().begin();
    auto dragRadius = conicGradientBounds.getWidth() * 0.5f + 25.0f;
    for (auto& slider : sliders)
    {
        slider->setAngle(stopIterator->angle);
        slider->dragRadius = dragRadius;
        ++stopIterator;
    }

    updateSliders();
}

void ConicGradientDemo::updateConicGradient()
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

    conicGradient.clearStops();

    float angle = juce::MathConstants<float>::twoPi * 0.0f;
    float angleStep = juce::MathConstants<float>::twoPi / (float)(colors.size() - 1);
    //float gray = 0.0f;
    for (auto& color128 : colors)
    {
        //conicGradient.addStop(angle, { gray, gray, gray, 1.0f });
        //gray += 1.0f / (float)(colors.size() - 1);
        conicGradient.addStop(angle, color128);
        angle += angleStep;
    }
}

void ConicGradientDemo::updateSliders()
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

void ConicGradientDemo::paintConicGradient(juce::Graphics& g)
{
    if (conicGradientBounds.isEmpty())
        return;

    if (image.isNull() || image.getWidth() != getWidth() || image.getHeight() != getHeight())
        image = juce::Image(juce::Image::ARGB, conicGradientBounds.getWidth(), conicGradientBounds.getHeight(), true);

    conicGradient.draw(image, {});
    g.drawImage(image, conicGradientBounds.toFloat(), juce::RectanglePlacement::centred);
}

ConicGradientDemo::ArcSlider::ArcSlider(ConicGradientDemo& owner_, size_t index_) :
    owner(owner_),
    index(index_)
{
    setSize(30, 30);
}

void ConicGradientDemo::ArcSlider::setAngle(float newAngle)
{
    if (owner.getBounds().isEmpty())
        return;

    auto center = owner.getLocalBounds().getCentre().toFloat();
    auto const& stop = owner.conicGradient.getStops()[index];

    setCentrePosition(center.getPointOnCircumference(dragRadius, newAngle).toInt());

    if (approximatelyEqual(normalizeAngle(stop.angle), normalizeAngle(newAngle)))
        return;

    if (onChange)
        onChange(index, newAngle);
}

void ConicGradientDemo::ArcSlider::paint(juce::Graphics& g)
{
    auto const& stop = owner.conicGradient.getStops()[index];
    auto c = stop.color128.toColour();
    g.setColour(c.contrasting());
    g.fillEllipse(getLocalBounds().toFloat());
    g.setColour(c);
    g.fillEllipse(getLocalBounds().toFloat().reduced(2.0f));
}

void ConicGradientDemo::ArcSlider::resized()
{

}

void ConicGradientDemo::ArcSlider::mouseDown(const juce::MouseEvent& e)
{
    auto center = owner.getLocalBounds().getCentre().toFloat();
    mouseDragStartAngle = center.getAngleToPoint(e.getEventRelativeTo(&owner).position);
    startAngle = owner.conicGradient.getStops()[index].angle;
}

void ConicGradientDemo::ArcSlider::mouseDrag(const juce::MouseEvent& e)
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
