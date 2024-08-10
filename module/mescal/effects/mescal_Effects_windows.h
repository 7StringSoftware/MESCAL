#pragma once

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
