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

 https://learn.microsoft.com/en-us/windows/win32/direct2d/built-in-effects

 An Effect has a set of inputs and a set of properties. Running the effect processes the inputs according to the effect's
 properties and paints the processed output onto an Image.

 An Effect input can be set to be a JUCE Image or another Effect. Connecting an Effect to the input of another
 Effect feeds the output of the "upstream" effect directly into the "downstream" effect. You can chain multiple effects together
 to create complex image processing graphs.

 Some effects have no inputs:

 \image html simple_effect.svg

 Most effects have a single input and a single output:

 \image html one_in_one_out.svg

 Some effects have multiple inputs:

 \image html two_in_one_out.svg

 Chained effects are automatically run the proper order; here, "Effect 1" will run first, then "Effect 2" will process the
 output of "Effect 1".

 \image html two_in_two_effects.svg

 Effect graphs can be arbitrarily complex; the main limitation is the capacity of the GPU.

 To apply a single Effect, create an Effect object with the desired effect type, set the inputs, set the properties,
 and call applyEffect.

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

   //
   // sourceImage -> blurEffect
   //
   blurEffect.setInput(0, sourceImage);

   //
   // blurEffect -> affineTransformEffect
   //
   affineTransformEffect.setInput(0, blurEffect);

   //
   // sourceImage -> blurEffect -> affineTransformEffect
   //
   // Only call applyEffect on the last effect in the chain; blurEffect will be applied first, then affineTransformEffect
   //
   affineTransformEffect.applyEffect(outputImage, juce::AffineTransform{}, false);
\endcode

 To apply multiple effects, determine what effects you want to use and the order in which they should be  applied. Create the
 final effect, then create the Image and Effect objects that will be the inputs to the final effect. Call Effect::setInput on
 each input for the final effect and pass the Image or Effect object for that input. Repeat that same process for each effect
 upstream from the final effect. Note that you only need to call applyEffect on the final effect; the upstream effects will be
 applied in the correct order.

Note that applyEffect is only called on the affine transform effect; chained effects are applied recursively in the correct order.

Effects only run in the GPU, so any Image objects used for inputs or outputs must be Direct2D Image objects.

*/

class Effect : public juce::ReferenceCountedObject
{
public:
    struct Ptr : juce::ReferenceCountedObjectPtr<Effect>
    {
        Ptr() : juce::ReferenceCountedObjectPtr<Effect>() {}
        Ptr(Effect* effect) : juce::ReferenceCountedObjectPtr<Effect>(effect) {}

        Ptr operator<< (juce::Image const& image)
        {
            get()->addInput(image);
            return get();
        }

        Ptr operator<< (Ptr effect)
        {
            get()->addInput(effect);
            return get();
        }
    };

    /**
     * Enumeration of built-in effect types
     */
    enum class Type
    {
        affineTransform2D,          /**< Affine Transform 2D effect */
        alphaMask,                  /**< Alpha mask effect */
        arithmeticComposite,        /**< Arithmetic composite effect */
        blend,                      /**< Blend effect */
        chromaKey,                  /**< Chroma key effect */
        composite,                  /**< Composite effect */
        crop,                       /**< Crop effect */
        edgeDetect,                 /**< Edge detect effect */
        emboss,                     /**< Emboss effect */
        flood,                      /**< Flood effect */
        gaussianBlur,               /**< GaussianBlur effect */
        highlightsAndShadows,       /**< Highlights and Shadows effect */
        invert,                     /**< Invert effect */
        luminanceToAlpha,           /**< Luminance to alpha effect */
        perspectiveTransform3D,     /**< Perspective Transform 3D effect */
        shadow,                     /**< Shadow effect */
        spotDiffuseLighting,        /**< Spot Diffuse Lighting effect */
        spotSpecularLighting,       /**< Spot Specular Lighting effect */
        numEffectTypes              /**< Number of effect types */
    };

    /**
    * Constants for built-in Direct2D 2D affine transform effect
    *
    * Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/2d-affine-transform
    */
    struct AffineTransform2D : public Ptr
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

        static AffineTransform2D create(juce::AffineTransform transform)
        {
            auto effect = new Effect{ Effect::Type::affineTransform2D };
            effect->setPropertyValue(AffineTransform2D::transformMatrix, transform);
            return { effect };
        }
        AffineTransform2D(Effect* effect) : Ptr(effect) {}
    };

    /**
    * Built-in Direct2D alpha mask effect
    *
    * Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/alpha-mask
    */
    struct AlphaMask : public Ptr
    {
        static AlphaMask create()
        {
            return { new Effect{ Effect::Type::alphaMask } };
        }
        AlphaMask(Effect* effect) : Ptr(effect) { }
    };

    /**
    * Constants for built-in Direct2D arithmetic composite effect
    *
    * Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/arithmetic-composite
    */
    struct ArithmeticComposite : public Ptr
    {
        static constexpr int coefficients = 0;
        static constexpr int clampOutput = 1;

        static ArithmeticComposite create(float c0, float c1, float c2, float c3)
        {
            auto effect = new Effect{ Effect::Type::arithmeticComposite };
            effect->setPropertyValue(ArithmeticComposite::coefficients, Vector4{ c0, c1, c2, c3 });
            return { effect };
        }
        ArithmeticComposite(Effect* effect) : Ptr(effect) { }
    };

    /**
    * Constants for built-in Direct2D blend effect
    *
    * Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/blend
    */
    struct Blend : public Ptr
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

        static Blend create(int initialMode)
        {
            auto effect = new Effect{ Effect::Type::blend };
            effect->setPropertyValue(Blend::mode, initialMode);
            return { effect };
        }
        Blend(Effect* effect) : Ptr(effect) { }
    };

    /**
    * Constants for built-in Direct2D composite effect
    *
    * Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/composite
    */
    struct Composite : public Ptr
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

        static Composite create(int initialMode)
        {
            auto effect = new Effect{ Effect::Type::composite };
            effect->setPropertyValue(Composite::mode, initialMode);
            return { effect };
        }
        Composite(Effect* effect) : Ptr(effect) { }
    };

    /**
    * Constants for built-in Direct2D crop effect
    *
    * Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/crop
    */
    struct Crop : public Ptr
    {
        static constexpr int rect = 0;
        static constexpr int borderMode = 1;

        static Crop create(juce::Rectangle<float> cropArea);
        Crop(Effect* effect) : Ptr(effect) {}
    };

    /**
    * Constants for built-in Direct2D 2D emboss effect
    *
    * Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/emboss
    */
    struct Emboss
    {
        static constexpr int height = 0;
        static constexpr int direction = 0;
    };

    /**
    * Constants for built-in Direct2D 2D affine transform effect
    *
    * Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/flood
    */
    struct Flood : public Ptr
    {
        static constexpr int color = 0;

        static Flood create(juce::Colour floodColor)
        {
            auto effect = new Effect{ Effect::Type::flood };
            effect->setPropertyValue(Flood::color, floodColor);
            return { effect };
        }
        Flood(Effect* effect) : Ptr(effect) {}
    };

    /**
    * Constants for built-in Direct2D gaussian blur effect
    *
    * Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/gaussian-blur
    */
    struct GaussianBlur : public Ptr
    {
        static constexpr int standardDeviation = 0;
        static constexpr int optimization = 1;
        static constexpr int borderMode = 2;

        static constexpr int speed = 0;
        static constexpr int balanced = 1;
        static constexpr int quality = 2;

        static constexpr int soft = 0;
        static constexpr int hard = 1;

        static GaussianBlur create(float standardDeviation);
        GaussianBlur(Effect* effect) : Ptr(effect) { }
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
    * Constants for built-in Direct2D shadow effect
    *
    * Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/shadow
    */
    struct Shadow : public Ptr
    {
        static constexpr int blurStandardDeviation = 0;
        static constexpr int color = 1;
        static constexpr int optimization = 2;

        static constexpr int speed = 0;
        static constexpr int balanced = 1;
        static constexpr int quality = 2;

        static Shadow create(float standardDeviation, juce::Colour color)
        {
            auto effect = new Effect{ Effect::Type::shadow };
            effect->setPropertyValue(Shadow::blurStandardDeviation, standardDeviation);
            effect->setPropertyValue(Shadow::color, color);
            return { effect };
        }
        Shadow(Effect* effect) : Ptr(effect) {}
    };

    /**
    * Constants for built-in Direct2D spot diffuse lighting effect
    *
    * Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/spot-diffuse-lighting
    */
    struct SpotDiffuseLighting : public Ptr
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

        SpotDiffuseLighting withLightPosition(float x, float y, float z);
        SpotDiffuseLighting withPointsAt(float x, float y, float z);
        SpotDiffuseLighting withFocus(float focus);
        SpotDiffuseLighting withLimitingConeAngle(float angle);
        SpotDiffuseLighting withDiffuseConstant(float constant);
        SpotDiffuseLighting withSurfaceScale(float scale);
        SpotDiffuseLighting withColor(juce::Colour colour);
        SpotDiffuseLighting withKernelUnitLength(float x, float y);
        SpotDiffuseLighting withScaleMode(int mode);
    };

    /**
    * Constants for built-in Direct2D spot specular lighting effect
    *
    * Reference: https://learn.microsoft.com/en-us/windows/win32/direct2d/spot-specular-lighting
    */
    struct SpotSpecularLighting : public Ptr
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

        static SpotSpecularLighting create();
        SpotSpecularLighting(Effect* effect) : Ptr(effect) { }
        SpotSpecularLighting withLightPosition(float x, float y, float z);
        SpotSpecularLighting withPointsAt(float x, float y, float z);
        SpotSpecularLighting withFocus(float focus);
        SpotSpecularLighting withLimitingConeAngle(float angle);
        SpotSpecularLighting withSpecularExponent(float exponent);
        SpotSpecularLighting withSpecularConstant(float constant);
        SpotSpecularLighting withSurfaceScale(float scale);
        SpotSpecularLighting withColor(juce::Colour colour);
        SpotSpecularLighting withKernelUnitLength(float x, float y);
        SpotSpecularLighting withScaleMode(int mode);
    };

    /**
    * Variant that can hold different types of inputs for an effect.
    *
    * Effects have zero or more inputs. Each input can be a JUCE Image or another Effect.
    * Chaining effects together allows for complex image processing graphs.
    */
    using Input = std::variant<std::monostate, juce::Image, Ptr>;

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
        juce::AffineTransform,
        juce::Colour,
        juce::Rectangle<float>>;

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
    Effect(const Effect&& other) noexcept;
    ~Effect();

    static Ptr create(Type effectType);

    /**
    * Get the name of the effect
    */
    juce::String getName() const noexcept;

    /**
    * Get a reference to the effect's array of inputs.
    *
    * Effects can have zero, one, or multiple inputs.
    */
    std::vector<Input> const& getInputs() const noexcept;

    /**
    * Set the input at the specified index to a JUCE Image
    *
    * @param index The index of the input
    * @param image The JUCE Image to use as the input
    */
    void setInput(int index, juce::Image const& image);
    void addInput(juce::Image const& image);

    /**
    * Set the input at the specified index to another Effect. This builds an effect processing graph.
    *
    * @param index The index of the input
    * @param image The Effect to use as the input
    */
    void setInput(int index, Ptr otherEffect);
    void addInput(Ptr otherEffect);

    /**
    * Run this Effect and paint the output from this Effect onto outputImage.
    *
    * If any of the inputs to this Effect are also Effect objects, then this effect will walk up the chain
    * and recursively run any upstream effects in the correct order. You only need to call applyEffect
    * for the last effect in the chain.
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

    std::function<void(Effect*, int, PropertyValue)> onPropertyChange;

protected:
    /** @internal */
    struct Pimpl;
    std::shared_ptr<Pimpl> pimpl;
};
