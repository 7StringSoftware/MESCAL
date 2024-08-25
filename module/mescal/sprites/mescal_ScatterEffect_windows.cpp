namespace mescal
{
    ScatterEffect::ScatterEffect()
    {
    }

    void ScatterEffect::setScatterDistanceMultiplier(float multiplier)
    {
        scatterMultiplier = multiplier;
    }

    void ScatterEffect::setParticleSize(int size)
    {
        particleSize = size;
    }

    void ScatterEffect::applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha)
    {
        spriteBatch.setAtlas(sourceImage);

        auto center = sourceImage.getBounds().toFloat().getCentre();

        std::vector<Sprite> sprites;
        particleSize = 128;
        for (int x = 0; x < sourceImage.getWidth(); x += particleSize)
        {
            for (int y = 0; y < sourceImage.getHeight(); y += particleSize)
            {
                Sprite sprite;
                sprite.destination = juce::Rectangle<int>(x, y, particleSize, particleSize).toFloat();

                auto angle = center.getAngleToPoint(sprite.destination.getCentre());
                auto distance = center.getDistanceFrom(sprite.destination.getCentre());
                sprite.destination.setCentre(center.getPointOnCircumference(distance * scatterMultiplier, angle));

                sprite.source = juce::Rectangle<int>(x, y, particleSize, particleSize);
                sprites.push_back(sprite);
            }
        }

        if (outputImage.isNull() || outputImage.getWidth() < width || outputImage.getHeight() < height)
        {
            outputImage = juce::Image(juce::Image::PixelFormat::ARGB, width, height, true);
        }

        spriteBatch.draw(outputImage, sprites, true);
        destContext.drawImageAt(outputImage, 0, 0);
    }

    void ScatterEffect::setSize(int width_, int height_)
    {
        width = width_;
        height = height_;
    }
};

