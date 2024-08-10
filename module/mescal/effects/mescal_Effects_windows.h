#pragma once

enum class GaussianBlurProperty
{
    standardDeviation,
    optimization,
    borderMode,
    numProperties
};

enum class SpotSpecularLightingProperty
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
    // Default value is solid white (0xffffff)
    lightColor,

    // Kernel unit length
    kernelUnitLength,

    // Scale mode 0-5
    scaleMode,

    numProperties
};

enum class PerspectiveTransform3DProperty
{
    interpolationMode,

    borderMode,

    depth,

    perspectiveOrigin,

    localOffset,

    globalOffset,

    rotationOrigin,

    rotation,

    numProperties
};

enum class ShadowProperty
{
    standardDeviation,
    color,
    optimization,
    numProperties
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

using Vector2 = std::array<float, 2>;
using Vector3 = std::array<float, 3>;
using Vector4 = std::array<float, 4>;
using Enumeration = uint8_t;

class Effect : public juce::ImageEffectFilter
{
public:
	enum class Type
	{
		gaussianBlur,
		spotSpecularLighting,
        shadow,
        spotDiffuseLighting,
        perspectiveTransform3D,
		numEffectTypes
	};

    Effect(Type effectType_);
    Effect(const Effect& other);
    ~Effect() override;

    juce::String getName() const noexcept;

	void applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha) override;
	void applyEffect(juce::Image& sourceImage, juce::Image& outputImage, float scaleFactor, float alpha, bool clearDestination);

    using PropertyValue = std::variant<std::monostate,
        juce::String,
        bool,
        uint32_t,
        int,
        float,
        Vector2,
        Vector3,
        Vector4,
        Enumeration>;

    struct Property
    {
        juce::String name;
        PropertyValue defaultValue;
    };

    const std::vector<Property>& getProperties() const noexcept;
    void setPropertyValue(int index, const PropertyValue& value);

	Type const effectType;

protected:
    friend class EffectChain;

    void initProperties();

    struct Pimpl;
	std::unique_ptr<Pimpl> pimpl;
};

class EffectChain
{
public:
    void addEffect(Effect::Type effectType);
    void applyEffects(juce::Image& sourceImage, juce::Image& outputImage, float scaleFactor, float alpha, bool clearDestination);

    auto& getEffect(size_t index) noexcept { return effects[index]; }

protected:
    std::vector<Effect> effects;
};
