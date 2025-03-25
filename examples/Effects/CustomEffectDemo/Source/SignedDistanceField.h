
#pragma once

template <typename T>
class SignedDistanceField
{
public:
    SignedDistanceField(juce::RectangleList<T> const &rectangles)
    {
    }

    static void createSignedDistanceField(juce::RectangleList<T> const& rectangles, juce::Image& sdfImage)
    {
        juce::Image::BitmapData bitmapData{ sdfImage, juce::Image::BitmapData::readWrite };
        for (auto y = 0; y < bitmapData.height; ++y)
        {
            memset(bitmapData.getLinePointer(y), 0, bitmapData.lineStride);
        }

        auto rectangleListBounds = rectangles.getBounds().toFloat();
        auto rectangleListSize = (float)juce::jmax(rectangleListBounds.getWidth(), rectangleListBounds.getHeight());
        auto imageBounds = sdfImage.getBounds().toFloat();
        auto imageSize = (float)juce::jmax(bitmapData.width, bitmapData.height);

        auto rectangleListNormalizeScale = 1.0f / rectangleListSize;
        auto rectangleListSizeOverImageSize = rectangleListSize / imageSize;
        auto scaledImageBounds = imageBounds * rectangleListSizeOverImageSize;

        for (auto const& r : rectangles)
        {
            auto intersection = r.toFloat().getIntersection(scaledImageBounds);
            if (intersection.isEmpty())
                continue;

            auto snappedIntersection = intersection.getSmallestIntegerContainer();
            for (int physicalX = snappedIntersection.getX(); physicalX < snappedIntersection.getRight(); ++physicalX)
            {
                float pixelCenterX = (float)physicalX + 0.5f;
                for (int physicalY = snappedIntersection.getY(); physicalY < snappedIntersection.getBottom(); ++physicalY)
                {
                    float pixelCenterY = (float)physicalY + 0.5f;
                    auto distance = juce::jmin(juce::jmin(pixelCenterX - intersection.getX(), intersection.getRight() - pixelCenterX), juce::jmin(pixelCenterY - intersection.getY(), intersection.getBottom() - pixelCenterY));
                    distance = std::abs(distance);
                    auto scaledDistance = distance * rectangleListNormalizeScale;
                    auto alpha = scaledDistance * 0.5f + 0.5f;

                    alpha = juce::jmax(alpha, bitmapData.getPixelColour(physicalX, physicalY).getFloatAlpha());
                    bitmapData.setPixelColour(physicalX, physicalY, juce::Colours::white.withAlpha(alpha));
                }
            }
        }
#if 0
        for (int x = 0; x < bitmapData.width; ++x)
        {
            float scaledX = (float)x * rectangleListSizeOverImageSize;
            for (int y = 0; y < bitmapData.height; ++y)
            {
                float scaledY = (float)y * rectangleListSizeOverImageSize;

                auto minScaledDistance = std::numeric_limits<float>::max();
                bool inside = false;
                for (auto const& r : rectangles)
                {
                    if (!r.contains(scaledX, scaledY))
                        continue;

                    auto distance = juce::jmin(juce::jmin(scaledX - r.getX(), r.getRight() - scaledX), juce::jmin(scaledY - r.getY(), r.getBottom() - scaledY));
                    distance = std::abs(distance);
                    auto scaledDistance = distance * rectangleListNormalizeScale;
                    minScaledDistance = juce::jmin(minScaledDistance, scaledDistance);
                    inside = true;
                }

                if (inside)
                {
                    auto alpha = minScaledDistance * 0.5f + 0.5f;
                    bitmapData.setPixelColour(x, y, juce::Colours::white.withAlpha(alpha));
                }
                else
                {
                    bitmapData.setPixelColour(x, y, juce::Colours::transparentBlack);
                }
            }
#endif
    }
};
