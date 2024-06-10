#pragma once

struct Sprite
{
    juce::Rectangle<float> destination;
    juce::Rectangle<int> source;
};

class SpriteBatch
{
public:
    SpriteBatch();
    ~SpriteBatch();

    void clear();
    void setAtlas(juce::Image atlas);
    void draw(juce::Image destinationImage, const std::vector<Sprite>& sprites);

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
