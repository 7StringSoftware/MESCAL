#pragma once

class ConicGradient
{
public:
    ConicGradient();
    ~ConicGradient();

    void setBounds(juce::Rectangle<float> bounds_);
    juce::Rectangle<float> getBounds() const noexcept;

    struct Stop
    {
        float angle;
        juce::Colour color;
    };

    void clearStops();
    void addStop(float angle, juce::Colour color);
    void addStops(juce::Span<Stop> newStops);

    void draw(juce::Image image, juce::AffineTransform transform);

private:
    juce::Rectangle<float> bounds;

    struct Stop128
    {
        float angle;
        mescal::Color128 color;
    };
    std::vector<Stop128> stops;

    void sortStops();

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
