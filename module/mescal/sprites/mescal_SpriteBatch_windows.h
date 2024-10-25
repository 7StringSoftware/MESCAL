#pragma once

struct Sprite
{
    juce::Rectangle<int> atlasSourceArea;
    juce::Rectangle<float> drawArea;
};

class SpriteBatch
{
public:
    SpriteBatch();
    ~SpriteBatch();

    void setAtlas(juce::Image atlas);
    void draw(juce::Image destinationImage, const std::vector<Sprite>& sprites, bool clearImage);

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};
