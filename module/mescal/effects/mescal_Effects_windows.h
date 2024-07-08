#pragma once

enum SpotSpecularLightingProperty
{
    // Light position in image coordinates
    // Default value: x:0.0f, y:0.0f, z:0.0f
    lightPosition,

    // Focus point for the spotlight
    // Default value: x:0.0f, y:0.0f, z:0.0f
    focusPointPosition,

    // Focus from 0.0f to 200.0f
    // Default is 1.0f
    focus,

    // Cone angle that restricts the projection area from 0 to 90 degrees
    // Default value is 90.0f
    limitingConeAngle,

    // Specular exponent from 1.0f to 128.0f
    // Default value is 1.0f
    specularExponent,

    // Specular constant from 0.0f to 10000.0f
    // Default value is 1.0f
    specularConstant,

    // Surface scale from 0.0f to 10000.0f
    // Default value is 1.0f
    surfaceScale,

    // Light color (RBG value)
    // Default value is ffffff
    lightColor,

    // Kernel unit length
    kernelUnitLength,

    // Scale mode 0-5
    scaleMode
};

struct RGBColor
{
    static RGBColor fromColour(juce::Colour colour)
    {
        return { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue() };
    };

    float r = 1.0f, g = 1.0f, b = 1.0f;
};

struct Point3D
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
};

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

    Effect(Type effectType_);
    ~Effect() override;

	void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;
	void applyEffect(juce::Image& sourceImage, juce::Image& outputImage, float scaleFactor, float alpha);

    using PropertyValue = std::variant<float, juce::Point<float>, juce::Colour, RGBColor, Point3D>;

    size_t  getNumProperties() const noexcept;
    void setProperty(int index, const PropertyValue& value);
    const PropertyValue& getProperty(int index);

	Type const effectType;

protected:
    struct Pimpl;
	std::unique_ptr<Pimpl> pimpl;
};
#if 0

class GaussianBlur : public Effect
{
public:
    GaussianBlur();
    ~GaussianBlur() override;

    enum class PropertyIndex
    {
        // Amount of blur starting at 0.0f
        // Default value: 3.0f
        // Blur radius = standard deviation * 3.0f
        standardDeviation,
        optimizationMode,
        borderMode
    };

    enum class OptimizationMode
    {
        speed,
        balanced,
        quality
    };

    enum class BorderMode
    {
        soft,
        hard
    };

    using PropertyValue = std::variant<float, OptimizationMode, BorderMode>;

    void setProperty(PropertyIndex index, const PropertyValue& value);
    void getProperty(PropertyIndex index, PropertyValue& value);
};

class SpotSpecularLighting : public Effect
{
public:
    SpotSpecularLighting();
    ~SpotSpecularLighting() override;

    enum class PropertyIndex
    {
        // Light position in image coordinates
        // Default value: x:0.0f, y:0.0f, z:0.0f
        lightPosition,

        // Focus point for the spotlight
        // Default value: x:0.0f, y:0.0f, z:0.0f
        pointsAt,

        // Focus from 0.0f to 200.0f
        // Default is 1.0f
        focus,

        // Cone angle that restricts the projection area from 0 to 90 degrees
        // Default value is 90.0f
        limitingConeAngle,

        // Specular exponent from 1.0f to 128.0f
        // Default value is 1.0f
        specularExponent,

        // Specular constant from 0.0f to 10000.0f
        // Default value is 1.0f
        specularConstant,

        // Surface scale from 0.0f to 10000.0f
        // Default value is 1.0f
        surfaceScale,

        // Light color (RBG value)
        // Default value is ffffff
        lightColor,

        // Kernel unit length
        kernelUnitLength,

        // Scale mode 0-5
        scaleMode
    };

    using PropertyValue = std::variant<juce::Point<float>, juce::Colour, float>;

    void configureEffect() override;

    void setProperty(PropertyIndex index, const PropertyValue& value);
    void getProperty(PropertyIndex index, PropertyValue& value);
};
#endif
