#pragma once

/**
 * A conic gradient is a gradient with colors that flow in an elliptical path around a center point. The ConicGradient class
 * stores a set of color stops that define how the colors change at each angular position.
 *
 * The actual gradient is painted by a GPU shader onto a JUCE Image by calling the draw method. Note that the Image needs to
 * be a Direct2D Image.
 */
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

    juce::Range<float> getRadiusRange() const noexcept
    {
        return radiusRange;
    }

    /**
     * Represents a single color position along the arc of the gradient
     */
    struct Stop
    {
        /**
        * The angle of the stop in radians. A value of zero radians represents straight up (12 o'clock).
        * Positive values represent clockwise rotation; negative values represent counter-clockwise rotation.
        */
        float angle;

        /**
         * Gradient colors are stored using 32-bit float values per color channel
         */
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
