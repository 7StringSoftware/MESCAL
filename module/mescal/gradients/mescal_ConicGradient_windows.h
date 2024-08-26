#pragma once

class ConicGradient
{
public:
    ConicGradient();
    ~ConicGradient();

    void setRadiusRange(juce::Range<float> radiusRange_)
    {
        jassert(radiusRange_.getStart() >= 0.0f);
        radiusRange = radiusRange_;
    }

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
    std::vector<Stop> stops;
    juce::Range<float> radiusRange;

    void sortStops();

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
