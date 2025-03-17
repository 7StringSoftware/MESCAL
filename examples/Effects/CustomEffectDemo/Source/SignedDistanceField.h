
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
        juce::Image::BitmapData bitmapData{ outputImage, juce::Image::BitmapData::readWrite };
        memset(bitmapData.data, 0, bitmapData.pixelStride * bitmapData.height * bitmapData.lineStride);

        auto size = rectangles.getBounds();
        auto scale = 1.0f / juce::jmax(size.getWidth(), size.getHeight());
        for (auto const& r : rectangles)
        {
            auto snappedBounds = r.toNearestIntEdges();
            for (int y = snappedBounds.getY(); y < snappedBounds.getBottom(); ++y)
            {
                for (int x = snappedBounds.getX(); x < snappedBounds.getRight(); ++x)
                {
                    auto distance = juce::jmin(juce::jmin((T)x - r.getX(), r.getRight() - (T)x), juce::jmin((T)y - r.getY(), r.getBottom() - (T)y));
                    distance = std::abs(distance);
                    auto scaledDistance = (float)distance * scale;
                    auto alpha = scaledDistance * 0.5f + 0.5f;

                    auto storedDistance = bitmapData.getPixelColour(x, y).getFloatAlpha();
                    if (storedDistance >= 0.5f)
                        alpha = juce::jmin(alpha, storedDistance);
                    bitmapData.setPixelColour(x, y, juce::Colours::black.withAlpha(alpha));
                }
            }
        }
    }
};
