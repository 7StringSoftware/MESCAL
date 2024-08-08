#include "MainComponent.h"
#include "PropertyComponents.h"

using Variant = std::variant<int, float, juce::Point<float>>;
using Range = std::variant<juce::Range<int>, juce::Range<float>>;

struct PropertyInfo
{
    const char* name;
    Variant value;
    Range range;
};

static constexpr std::array<PropertyInfo, 3> propertyValues
{
    PropertyInfo{ "name", 3.0f, juce::Range<float>{ 0.0f, 100.0f } },
    PropertyInfo{ "name", 1, juce::Range<int>{ 0, 2 } },
    PropertyInfo{ "name", juce::Point<float>{ 0.0f, 0.0f }, juce::Range<float>{} }
};

struct PropertyInfoGroup
{
    PropertyInfo const* const properties;
    size_t numProperties;
};

static constexpr std::array<PropertyInfo, 2> propertyValues2
{
    PropertyInfo{ "name", 3.0f, juce::Range<float>{ 0.0f, 100.0f } },
    PropertyInfo{ "name", 1, juce::Range<int>{ 0, 2 } }
};

static constexpr std::array<PropertyInfoGroup, 2> propertyGroups
{
    PropertyInfoGroup{ propertyValues.data(), propertyValues.size() },
    PropertyInfoGroup{ propertyValues2.data(), propertyValues2.size() }
};

MainComponent::MainComponent()
{
    //sourceImages.emplace_back(juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize));
    sourceImages.emplace_back(juce::ImageFileFormat::loadFrom(BinaryData::Piet_Mondriaan_19391942__Composition_10_jpg, BinaryData::Piet_Mondriaan_19391942__Composition_10_jpgSize));
    //sourceImages.emplace_back(juce::Image{ juce::Image::ARGB, 1000, 1000, true });
#if 0
    {
        juce::Graphics g{ sourceImages.back() };
        g.setColour(juce::Colours::transparentBlack);
        g.getInternalContext().fillRect(sourceImages.back().getBounds(), true);

        g.fillCheckerBoard(sourceImages.back().getBounds().toFloat(), 50.0f, 50.0f, juce::Colours::transparentBlack, juce::Colours::lightgrey.withAlpha(0.5f));
    }
#endif

    juce::Rectangle<int> maxBounds;
    for (auto const& sourceImage : sourceImages)
    {
        maxBounds = maxBounds.getUnion(sourceImage.getBounds());
    }

    outputImage = juce::Image{ juce::Image::ARGB, maxBounds.getWidth(), maxBounds.getHeight(), true };

    addAndMakeVisible(propertyPanel);

    setSize(1024, 1024);
    setRepaintsOnMouseActivity(true);

    valueChanged(effectTypeValue);
    effectTypeValue.addListener(this);

    setOpaque(true);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    if (sourceImages.size() == 0)
        return;

    juce::Rectangle<int> imageArea{ 0, 0, propertyPanel.getX(), getHeight() };
    int height = getHeight() / (int)sourceImages.size();
    auto sourceImageArea = imageArea.withWidth(imageArea.getWidth() / 2);
    for (auto& image : sourceImages)
    {
        g.drawImage(image, sourceImageArea.removeFromTop(height).toFloat());
    }

    if (outputImage.isValid())
    {
        applyEffect();

        juce::Rectangle<int> outputImageArea{ sourceImageArea.getRight(), 0, propertyPanel.getX() - sourceImageArea.getRight(), getHeight() };
        //g.drawImage(sourceImages.front(), outputImageArea.toFloat());
        g.drawImage(outputImage, outputImageArea.toFloat());
    }
}

void MainComponent::resized()
{
    propertyPanel.setBounds(getLocalBounds().removeFromRight(300).reduced(10));
}

void MainComponent::valueChanged(juce::Value& value)
{
    effect = nullptr;
    mescal::Effect::Type effectType = (mescal::Effect::Type)((int)value.getValue() - 1);
    effect = std::make_unique<mescal::Effect>(effectType);
    updateEffectType();
}

void MainComponent::buildPropertyPanel()
{
    auto getFloatRange = [&](mescal::JSONObject const& propertyObject) -> juce::Range<float>
        {
            if (propertyObject.hasProperty("Range"))
            {
                auto rangeObject = propertyObject.get<mescal::JSONObject>("Range");
                auto min = rangeObject.get<float>("Min");
                auto max = rangeObject.get<float>("Max");
                return { min, max };
            }

            return { 0.0f, 1.0f };
        };

    propertyPanel.clear();
    propertyValueComponents.clear();

    //
    // Effect selection
    //
    {
        juce::Array<juce::PropertyComponent*> sectionComponents;
        juce::StringArray effectNames
        {
            "Gaussian blur",
            "Spot specular lighting",
            "Shadow",
            "Spot diffuse lighting",
            "Perspective transform 3D"
        };
        juce::Array<juce::var> values;
        for (int value = 1; value <= (int)mescal::Effect::Type::numEffectTypes; ++value)
        {
            values.add(value);
        }
        sectionComponents.add(new juce::ChoicePropertyComponent{ effectTypeValue,
            "Effect",
            effectNames,
            values });
        propertyPanel.addSection("Effect", sectionComponents);
    }

    //
    // Effect-specific controls
    //
    {
        auto const& effectProperties = effect->getProperties();
        juce::Array<juce::PropertyComponent*> sectionComponents;

        for (auto const& property : effectProperties)
        {
            EffectPropertyValueComponent* propertyComponent = nullptr;
            juce::Range<float> range = property.range.has_value() ? std::get<juce::Range<float>>(property.range.value()) : juce::Range<float>{ 0.0f, 1.0f };

            if (std::holds_alternative<int>(property.defaultValue))
            {
                auto defaultValue = std::get<int>(property.defaultValue);
                propertyComponent = new MultiSliderPropertyComponent{ property.name,
                    0,
                    defaultValue,
                    juce::Array<float>{ (float)defaultValue },
                    juce::StringArray{ {} },
                    juce::Range<float>{ (float)range.getStart(), (float)range.getEnd() } };
            }
            else if (std::holds_alternative<float>(property.defaultValue))
            {
                auto defaultValue = std::get<float>(property.defaultValue);
                propertyComponent = new MultiSliderPropertyComponent{ property.name,
                    0,
                    defaultValue,
                    juce::Array<float>{ defaultValue },
                    juce::StringArray{ {} },
                    range };
            }
            else if (std::holds_alternative<mescal::Vector2>(property.defaultValue))
            {
                auto defaultValue = std::get<mescal::Vector2>(property.defaultValue);
                propertyComponent = new MultiSliderPropertyComponent{ property.name,
                    0,
                    defaultValue[0],
                    juce::Array<float> { defaultValue[0], defaultValue[1] },
                    juce::StringArray{ "0", "1" },
                    juce::Range<float>{ 0.0f, 1.0f } };
            }
            else if (std::holds_alternative<mescal::Vector3>(property.defaultValue))
            {
                auto defaultValue = std::get<mescal::Vector3>(property.defaultValue);
                propertyComponent = new MultiSliderPropertyComponent{ property.name,
                    0,
                    defaultValue[0],
                    juce::Array<float>{ defaultValue[0], defaultValue[1], defaultValue[2] },
                    juce::StringArray{ "0", "1", "2"},
                    juce::Range<float>{ 0.0f, 1.0f } };
            }
            else if (std::holds_alternative<uint8_t>(property.defaultValue))
            {
                auto defaultValue = std::get<uint8_t>(property.defaultValue);
                juce::StringArray array;
                for (int i = 0; i < 3; ++i)
                {
                    array.add(juce::String{ i });
                }
                propertyComponent = new EnumPropertyComponent{ property.name,
                    0,
                    defaultValue,
                    array };
            }
            else
            {
                jassertfalse;
            }

            if (propertyComponent)
            {
                propertyValueComponents.emplace_back(propertyComponent);
                sectionComponents.add(propertyComponent);
            }
        }

        propertyPanel.addSection("Properties", sectionComponents);
    }

#if 0
    //
    // Effect-specific controls
    //
    auto effectsArray = effectProperties.getArray("Effects");
    if (effectsArray.size() == 0)
        return;

    juce::Array<juce::PropertyComponent*> sectionComponents;

    auto const& effectObject = effectsArray.getObject((int)effect->effectType);
    auto const& propertiesArray = effectObject.getArray("Properties");
    size_t propertyIndex = 0;
    for (auto const& propertyVar : propertiesArray)
    {
        mescal::JSONObject propertyObject{ propertyVar };
        auto propertyName = propertyObject.get<juce::String>("Name");

        DBG("Property: " << propertyName);

        EffectPropertyValueComponent* propertyComponent = nullptr;

        auto const& type = propertyObject.get<juce::String>("Type");
        if (type == "Float")
        {
            auto defaultValue = propertyObject.hasProperty("DefaultValue") ? propertyObject.get<float>("DefaultValue") : 0.0f;
            auto range = getFloatRange(propertyObject);
            propertyComponent = new MultiSliderPropertyComponent{propertyName,
                propertyIndex,
                defaultValue,
                juce::Array<float>{ defaultValue },
                juce::StringArray{ juce::String{} },
                range };
        }
        else if (type == "Enum")
        {
            auto defaultValue = propertyObject.hasProperty("DefaultValue") ? propertyObject.get<int>("DefaultValue") : 0;
            auto enumArray = propertyObject.getArray("Values");
        }
        else if (type == "Point")
        {
        }
        else if (type == "Color")
        {
        }
        else if (type == "Point3D")
        {
        }
        else if (type == "Vector2")
        {
        }
        else if (type == "Vector3")
        {
        }
        else
        {
            jassertfalse;
        }

        if (propertyComponent)
        {
            propertyValueComponents.emplace_back(propertyComponent);
            sectionComponents.add(propertyComponent);
        }

        ++propertyIndex;
    }

#endif

    for (auto& c : propertyValueComponents)
    {
        c->onChange = [this](size_t propertyIndex, const mescal::Effect::PropertyValue& propertyValue)
            {
                effect->setPropertyValue(propertyIndex, propertyValue);
                applyEffect();
                repaint();
            };
    }
}

void MainComponent::updateEffectType()
{
    buildPropertyPanel();
    applyEffect();
    repaint();
}

void MainComponent::applyEffect()
{
    effect->applyEffect(sourceImages.front(), outputImage, 1.0f, 1.0f, true);
}
