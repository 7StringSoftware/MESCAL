
#pragma once

template <typename T>
class SignedDistanceField
{
public:
    SignedDistanceField(juce::RectangleList<T> const &rectangles)
    {

    }

    static void createSignedDistanceField(juce::RectangleList<T> const& rectangles, juce::Image& outputImage)
    {
        outputImage.clear(outputImage.getBounds(), juce::Colours::transparentBlack);

        juce::Image::BitmapData bitmapData{ outputImage, juce::Image::BitmapData::readWrite };
        auto size = rectangles.getBounds();
        auto scale = 1.0f / juce::jmax(size.getWidth(), size.getHeight());
        for (auto const& r : rectangles)
        {
            auto snappedBounds = r.toNearestIntEdges();
            for (int y = snappedBounds.getY(); y < snappedBounds.getBottom(); ++y)
            {
                for (int x = snappedBounds.getX(); x < snappedBounds.getRight(); ++x)
                {
                    auto distance = juce::jmax(juce::jmax((T)x - r.getX(), r.getRight() - (T)x), juce::jmax((T)y - r.getY(), r.getBottom() - (T)y));
                    //auto distance = juce::jmin(juce::jmin((T)x - r.getX(), r.getRight() - (T)x), juce::jmin((T)y - r.getY(), r.getBottom() - (T)y));
                    distance = std::abs(distance);
                    auto scaledDistance = (float)distance * scale;
                    auto alpha = scaledDistance * 0.5f + 0.5f;

                    auto storedDistance = bitmapData.getPixelColour(x, y).getFloatAlpha();
                    if (storedDistance >= 0.5f)
                        alpha = juce::jmin(alpha, storedDistance);
                    bitmapData.setPixelColour(x, y, juce::Colours::white.withAlpha(alpha));
                }
            }
        }
    }
};
