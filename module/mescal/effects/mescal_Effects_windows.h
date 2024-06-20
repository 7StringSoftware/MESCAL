#pragma once

class Effect : public juce::ImageEffectFilter
{
public:
	enum class Type
	{
		gaussianBlur,
		spotSpecularLighting,
		spotDiffuseLighting,
		numEffectTypes
	};

	enum class GaussianBlurPropertyIndex
	{
		blurAmount,
		borderMode,
		optimization
	};

	using PropertyIndex = std::variant<GaussianBlurPropertyIndex>;
	using PropertyValue = std::variant<float, juce::Colour, juce::Point<float>, juce::Rectangle<float>, juce::AffineTransform>;

	static std::unique_ptr<Effect> create(Type effectType);
	Effect(Type effectType_);
	~Effect() override;

	void setProperty(PropertyIndex index, const PropertyValue& value);
	void getProperty(PropertyIndex index, PropertyValue& value);

	void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;
	void applyEffect(juce::Image& sourceImage, juce::Image& outputImage, float scaleFactor, float alpha);

	Type const effectType;

protected:
	struct Pimpl;
	std::unique_ptr<Pimpl> pimpl;
};
