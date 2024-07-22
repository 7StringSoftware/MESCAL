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
        mescal::Color128 color128;
    };

    void clearStops();
    void addStop(float angle, Color128 color128);
    void addStops(juce::Span<Stop> newStops);
    auto const& getStops() const noexcept
    {
        return stops;
    }
    void setStopAngle(size_t index, float angle);

    void draw(juce::Image image, juce::AffineTransform transform);

private:
    juce::Rectangle<float> bounds;

    std::vector<Stop> stops;

    void sortStops();

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
