#pragma once

class ConicGradient
{
public:
    ConicGradient();
    ~ConicGradient();

    void setBounds(juce::Rectangle<float> bounds_);
    void setColourGradient(juce::ColourGradient gradient_);

    void draw(int numSegments, juce::Image image, juce::AffineTransform transform);

private:
    juce::ColourGradient gradient;
    juce::Rectangle<float> bounds;

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
