#pragma once

class MescalImageType : public juce::NativeImageType
{
public:
    MescalImageType();
    ~MescalImageType() override;

    juce::ImagePixelData::Ptr create(juce::Image::PixelFormat, int width, int height, bool clearImage, juce::Image::Permanence) const override;
    int getTypeID() const override;
};
