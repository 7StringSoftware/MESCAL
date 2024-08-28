#pragma once

/**
* Helper types used for effect properties
*/
using Vector2 = std::array<float, 2>;
using Vector3 = std::array<float, 3>;
using Vector4 = std::array<float, 4>;
using Enumeration = uint8_t;

inline Vector4 colourToVector4(juce::Colour colour)
{
    return { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() };
}

/**

 The Effect class is a wrapper for built-in Direct2D effects. The effects are processed using shaders
 in the GPU.

 An Effect takes a JUCE Image as an input, processes it with the effect, and paints the effect output onto another image. 
 Effect objects can also accept another Effect object as an input. This allows you easily apply a single effect to an Image, or 
 to chain effects together to create complex image processing graphs.

 Each effect has a set of properties that configure the effect, such as setting the blur radius or blend mode.

 To apply a single Effect, create an Effect object with the desired effect type, set the inputs, set the properties,
 and call applyEffect. For a single Effect you'll probably set the input to a JUCE Image and the output will be a JUCE Image.

 To apply multiple effects at once, determine what effects you want to use and the order in which they should be
 applied. Create the final effect, then create the Image and Effect objects that will be the inputs to the final effect.
 Call Effect::setInput on each input for the final effect and pass the Image or Effect object for that input. Repeat that some
 process for each effect "upstream" from the final effect. Note that you only need to call applyEffect on the final effect; the
 upstream effects will be applied in the correct order.

 The simplest effect graph is a single effect with no inputs:

 \image html simple_effect.svg

 Most effects have a single input and a single output:

 \image html one_in_one_out.svg

 Some effects have multiple inputs:

 \image html two_in_one_out.svg

 You can build more complex graphs; just set the input of the "downstream" effect to be another effect ("upstream").
 In this example, "Effect 1" is added as an input to "Effect 2":

 \image html two_in_two_effects.svg


 Effect graphs can be arbitrarily complex; the main limitation is the capacity of the GPU.


 The code for applying a blur effect to a single image looks like this:

 \code{.cpp}
	juce::Image sourceImage = ...;
	juce::Image outputImage = juce::Image{ juce::Image::ARGB, sourceImage.getWidth(), sourceImage.getHeight(), true };
	mescal::Effect blurEffect{ mescal::Effect::Type::gaussianBlur };
	blurEffect.setInput(0, sourceImage);
	blurEffect.applyEffect(outputImage, juce::AffineTransform{}, false);
\endcode

 And the same example that also adjusts the blur standard deviation:

 \code{.cpp}
	juce::Image sourceImage = ...;
	juce::Image outputImage = juce::Image{ juce::Image::ARGB, sourceImage.getWidth(), sourceImage.getHeight(), true };
	mescal::Effect blurEffect{ mescal::Effect::Type::gaussianBlur };
	blurEffect.setProperty(mescal::Effect::GaussianBlur::standardDeviation, 5.0f);
	blurEffect.setInput(0, sourceImage);
	blurEffect.applyEffect(outputImage, juce::AffineTransform{}, false);
\endcode

 To chain two effects together, set one effect to be the input of another effect. In this example, a JUCE Image is
 first blurred and then transformed using an affine transform. The affine transform is the last effect in the chain.

\code{.cpp}
   juce::Image sourceImage = ...;
   juce::Image outputImage = juce::Image{ juce::Image::ARGB, sourceImage.getWidth(), sourceImage.getHeight(), true };
   mescal::Effect blurEffect{ mescal::Effect::Type::gaussianBlur };
   mescal::Effect affineTransformEffect{ mescal::Effect::Type::affineTransform2D };
   blurEffect.setInput(0, sourceImage);
   affineTransformEffect.setInput(0, blurEffect);
   affineTransformEffect.applyEffect(outputImage, juce::AffineTransform{}, false);
\endcode

Note that applyEffect is only called on the affine transform effect; chained effects are applied recursively in the correct order.

*/

class Effect : public juce::ReferenceCountedObject
{
public:
	/**
	 * Enumeration of built-in effect types
	 */
	enum class Type
	{
		gaussianBlur,               /**< Gaussian blur effect */
		spotSpecularLighting,       /**< Spot specular lighting effect */
		shadow,                     /**< Shadow effect */
		spotDiffuseLighting,        /**< Spot diffuse lighting effect */
		perspectiveTransform3D,     /**< Perspective transform 3D effect */
		blend,                      /**< Blend effect */
		composite,                  /**< Composite effect */
		arithmeticComposite,        /**< Arithmetic composite effect */
		affineTransform2D,          /**< Affine transform 2D effect */
		numEffectTypes              /**< Number of effect types */
	};

	/**
	* Constants for built-in Direct2D gaussian blur effect
	*
	* Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/gaussian-blur
	*/
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

	/**
	* Constants for built-in Direct2D spot specular lighting effect
	*
	* Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/spot-specular-lighting
	*/
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

	/**
	* Constants for built-in Direct2D shadow effect
	*
	* Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/shadow
	*/
    struct Shadow
    {
        static constexpr int blurStandardDeviation = 0;
        static constexpr int color = 1;
        static constexpr int optimization = 2;

        static constexpr int speed = 0;
        static constexpr int balanced = 1;
        static constexpr int quality = 2;
    };

	/**
	* Constants for built-in Direct2D spot diffuse lighting effect
	*
	* Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/spot-diffuse-lighting
	*/
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

	/**
	* Constants for built-in Direct2D 3D perspective transform effect
	*
	* Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/3d-perspective-transform
	*/
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

	/**
	* Constants for built-in Direct2D blend effect
	*
	* Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/blend
	*/
    struct Blend
    {
        static constexpr int mode = 0;

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

	/**
	* Constants for built-in Direct2D composite effect
	*
	* Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/composite
	*/
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

	/**
	* Constants for built-in Direct2D arithmetic composite effect
	*
	* Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/arithmetic-composite
	*/
    struct ArithmeticComposite
    {
        static constexpr int coefficients = 0;
        static constexpr int clampOutput = 1;
    };

	/**
	* Constants for built-in Direct2D 2D affine transform effect
	*
	* Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/2d-affine-transform
	*/
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

	/**
    * Variant that can hold different types of inputs for an effect.
	*
	* Effects have zero or more inputs. Each input can be a JUCE Image or another Effect.
	* Chaining effects together allows for complex image processing graphs.
	*/
    using Input = std::variant<std::monostate, juce::Image, juce::ReferenceCountedObjectPtr<Effect>>;

	/**
    * Variant that can hold different types for getting and setting effect property values
	*
	* Effects have properties that configure the effect. Properties can have different types, such as
	* float, arrays, or text strings. The PropertyValue variant can hold any of these types.
	*/
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

	/**
	* Structure that contains metadata about an effect property
	*
	* PropertyInfo contains the name of the property and any relevant range information.
	*/
    struct PropertyInfo
    {
        juce::String name;
        std::optional<juce::Range<float>> range;
        juce::StringArray enumeration;
    };

	/**
	* Constructor for an Effect object
	*
	* @param effectType_ The type of effect to create
	*/
    Effect(Type effectType_);
    Effect(const Effect& other);
    ~Effect();

	/**
	* Get the name of the effect
	*/
    juce::String getName() const noexcept;

	/**
	* Get a reference to the effect's array of inputs.
	*
	* Effects can have zero, one, or multiple inputs. Each input can be connected to a
	* JUCE Image or another Effect.
	*/
	std::vector<Input> const& getInputs() const noexcept;

	/**
	* Set the input at the specified index to a JUCE Image
    *
    * @param index The index of the input
    * @param image The JUCE Image that will be the source for that input
	*/
    void setInput(int index, juce::Image const& image);

	/**
	* Set the input at the specified index to another Effect
    *
    * @param index The index of the input
    * @param image The Effect that will be the source for that input
    */
    void setInput(int index, juce::ReferenceCountedObjectPtr<Effect> otherEffect);

	/**
	* Run the effect graph and paint the output from the final effect in the chain onto outputImage
    *
    * This effect will be the final effect in the graph. If you have multiple chained effects you only
    * need to call applyEffect on the final effect. The final effect will walk balk up the chain and
    * apply any upstream effects in the correct order.
    *
    * @param The JUCE Image that will be painted with the output of the effect graph
    * @param transform The affine transform to apply to the effect output before the effect output is painted onto outputImage
    * @param clearDestination If true, outputImage will be cleared before the effect output is painted
	*/
    void applyEffect(juce::Image& outputImage, const juce::AffineTransform& transform, bool clearDestination);

	/**
	* Get the number of properties for the effect
	*/
    int getNumProperties();

	/**
	* Get the name of the property at the specified index
	*/
    juce::String getPropertyName(int index);

	/**
	* Set the value of a property
	*/
    void setPropertyValue(int index, const PropertyValue value);

	/**
	* Get the value of a property
	*/
    PropertyValue getPropertyValue(int index);

	/**
	* Get metadata for the specified property
	*/
    PropertyInfo getPropertyInfo(int index);

	Type const effectType;

    using Ptr = juce::ReferenceCountedObjectPtr<Effect>;

protected:
	/** @internal */
    struct Pimpl;
	std::unique_ptr<Pimpl> pimpl;
};
