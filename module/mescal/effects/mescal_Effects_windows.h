#pragma once

using Vector2 = std::array<float, 2>;
using Vector3 = std::array<float, 3>;
using Vector4 = std::array<float, 4>;
using Enumeration = uint8_t;

inline Vector4 colourToVector4(juce::Colour colour)
{
    return { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() };
}

class Effect : public juce::ReferenceCountedObject
{
public:
	enum class Type
	{
		gaussianBlur,
		spotSpecularLighting,
        shadow,
        spotDiffuseLighting,
        perspectiveTransform3D,
        blend,
        composite,
        arithmeticComposite,
        affineTransform2D,
		numEffectTypes
	};

    struct GaussianBlur
    {
        static constexpr int standardDeviation = 0;
        static constexpr int optimization = 1;
        static constexpr int borderMode = 2;

        static constexpr int speed = 0;
        static constexpr int balanced = 1;
        static constexpr int quality = 2;

        static constexpr int soft = 0;
        static constexpr int hard = 1;
    };

    struct SpotSpecular
    {
        static constexpr int lightPosition = 0;
        static constexpr int pointsAt = 1;
        static constexpr int focus = 2;
        static constexpr int limitingConeAngle = 3;
        static constexpr int specularExponent = 4;
        static constexpr int specularConstant = 5;
        static constexpr int surfaceScale = 6;
        static constexpr int color = 7;
        static constexpr int kernelUnitLength = 8;
        static constexpr int scaleMode = 9;

        static constexpr int nearestNeighbor = 0;
        static constexpr int linear = 1;
        static constexpr int cubic = 2;
        static constexpr int multiSampleLinear = 3;
        static constexpr int anisotropic = 4;
        static constexpr int highQualityCubic = 5;
    };

    struct Shadow
    {
        static constexpr int blurStandardDeviation = 0;
        static constexpr int color = 1;
        static constexpr int optimization = 2;

        static constexpr int speed = 0;
        static constexpr int balanced = 1;
        static constexpr int quality = 2;
    };

    struct SpotDiffuseLighting
    {
        static constexpr int lightPosition = 0;
        static constexpr int pointsAt = 1;
        static constexpr int focus = 2;
        static constexpr int limitingConeAngle = 3;
        static constexpr int diffuseConstant = 4;
        static constexpr int surfaceScale = 5;
        static constexpr int color = 6;
        static constexpr int kernelUnitLength = 7;
        static constexpr int scaleMode = 8;

        static constexpr int nearestNeighbor = 0;
        static constexpr int linear = 1;
        static constexpr int cubic = 2;
        static constexpr int multiSampleLinear = 3;
        static constexpr int anisotropic = 4;
        static constexpr int highQualityCubic = 5;
    };

    struct PerspectiveTransform3D
    {
        static constexpr int interpolationMode = 0;
        static constexpr int borderMode = 1;
        static constexpr int depth = 2;
        static constexpr int perspectiveOrigin = 3;
        static constexpr int localOffset = 4;
        static constexpr int globalOffset = 5;
        static constexpr int rotationOrigin = 6;
        static constexpr int rotation = 7;

        static constexpr int nearestNeighbor = 0;
        static constexpr int linear = 1;
        static constexpr int cubic = 2;
        static constexpr int multiSampleLinear = 3;
        static constexpr int anisotropic = 4;

        static constexpr int soft = 0;
        static constexpr int hard = 1;
    };

    struct Blend
    {
        static constexpr int multiply = 0;
        static constexpr int screen = 1;
        static constexpr int darken = 2;
        static constexpr int lighten = 3;
        static constexpr int dissolve = 4;
        static constexpr int colorBurn = 5;
        static constexpr int linearBurn = 6;
        static constexpr int darkerColor = 7;
        static constexpr int lighterColor = 8;
        static constexpr int colorDodge = 9;
        static constexpr int linearDodge = 10;
        static constexpr int overlay = 11;
        static constexpr int softLight = 12;
        static constexpr int hardLight = 13;
        static constexpr int vividLight = 14;
        static constexpr int linearLight = 15;
        static constexpr int pinLight = 16;
        static constexpr int hardMix = 17;
        static constexpr int difference = 18;
        static constexpr int exclusion = 19;
        static constexpr int hue = 20;
        static constexpr int saturation = 21;
        static constexpr int color = 22;
        static constexpr int luminosity = 23;
        static constexpr int subtract = 24;
        static constexpr int division = 25;
    };

    struct Composite
    {
        static constexpr int mode = 0;

        static constexpr int sourceOver = 0;
        static constexpr int destinationOver = 1;
        static constexpr int sourceIn = 2;
        static constexpr int destinationIn = 3;
        static constexpr int sourceOut = 4;
        static constexpr int destinationOut = 5;
        static constexpr int sourceAtop = 6;
        static constexpr int destinationAtop = 7;
        static constexpr int xor = 8;
        static constexpr int plus = 9;
        static constexpr int sourceCopy = 10;
        static constexpr int boundedSourceCopy = 11;
        static constexpr int maskInvert = 12;
    };

    struct ArithmeticComposite
    {
        static constexpr int coefficients = 0;
        static constexpr int clampOutput = 1;
    };

    struct AffineTransform2D
    {
        static constexpr int interpolationMode = 0;
        static constexpr int borderMode = 1;
        static constexpr int transformMatrix = 2;
        static constexpr int sharpness = 3;

        static constexpr int nearestNeighbor = 0;
        static constexpr int linear = 1;
        static constexpr int cubic = 2;
        static constexpr int multiSampleLinear = 3;
        static constexpr int anisotropic = 4;
        static constexpr int highQualityCubic = 5;

        static constexpr int soft = 0;
        static constexpr int hard = 1;
    };

    using Input = std::variant<juce::Image, juce::ReferenceCountedObjectPtr<Effect>>;

    using PropertyValue = std::variant<std::monostate,
        juce::String,
        bool,
        uint32_t,
        int,
        float,
        Vector2,
        Vector3,
        Vector4,
        Enumeration,
        juce::AffineTransform>;

    struct PropertyInfo
    {
        juce::String name;
        std::optional<juce::Range<float>> range;
        juce::StringArray enumeration;
    };

    Effect(Type effectType_);
    Effect(const Effect& other);
    ~Effect();

    juce::String getName() const noexcept;

    void setInput(int index, juce::Image const& image);
    void setInput(int index, juce::ReferenceCountedObjectPtr<Effect> otherEffect);
    std::vector<Input> const& getInputs() const noexcept;

    void applyEffect(juce::Image& outputImage, float scaleFactor, float alpha, bool clearDestination);

    int getNumProperties();
    juce::String getPropertyName(int index);
    void setPropertyValue(int index, const PropertyValue value);
    PropertyValue getPropertyValue(int index);
    PropertyInfo getPropertyInfo(int index);

	Type const effectType;

    using Ptr = juce::ReferenceCountedObjectPtr<Effect>;

protected:
    struct Pimpl;
	std::unique_ptr<Pimpl> pimpl;
};
