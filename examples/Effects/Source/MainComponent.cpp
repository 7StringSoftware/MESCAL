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
    updateSourceImages();

    addAndMakeVisible(propertyPanel);

    setSize(1024, 1024);
    setRepaintsOnMouseActivity(true);

    valueChanged(effectTypeValue);
    effectTypeValue.addListener(this);

    sourceImageValue.addListener(this);
    showSourceImageValue.addListener(this);

    setOpaque(true);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    g.setColour(juce::Colours::darkgrey);
    g.fillRect(propertyPanel.getBounds());

    if (sourceImages.size() == 0)
        return;

    juce::Rectangle<int> imageArea{ 0, 0, propertyPanel.getX(), getHeight() };
    int height = getHeight() / (int)sourceImages.size();
    auto sourceImageArea = imageArea.withWidth(proportionOfWidth(0.2f));
    for (auto& image : sourceImages)
    {
        g.drawImage(image, sourceImageArea.removeFromTop(height).toFloat(), juce::RectanglePlacement::centred);
    }

    if (outputImage.isValid())
    {
        applyEffect();

        juce::Rectangle<int> outputImageArea{ sourceImageArea.getRight(), 0, propertyPanel.getX() - sourceImageArea.getRight(), getHeight() };
        if (showSourceImageValue.getValue())
        {
            //g.drawImage(sourceImages.front(), outputImageArea.toFloat(), juce::RectanglePlacement::centred);
        }
        g.drawImage(outputImage, outputImageArea.toFloat(), juce::RectanglePlacement::centred);
    }
}

void MainComponent::resized()
{
    propertyPanel.setBounds(getLocalBounds().removeFromRight(400).reduced(10));
}

void MainComponent::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(effectTypeValue))
    {
        effect = nullptr;
        mescal::Effect::Type effectType = (mescal::Effect::Type)((int)value.getValue() - 1);
        effect = std::make_unique<mescal::Effect>(effectType);
        updateEffectType();
        return;
    }

    if (value.refersToSameSourceAs(sourceImageValue))
    {
        updateSourceImages();
        return;
    }

    if (value.refersToSameSourceAs(showSourceImageValue))
    {
        repaint();
        return;
    }
}

void MainComponent::buildPropertyPanel()
{
    auto getLabels = [&](mescal::JSONObject const& propertyObject, int numSliders)
        {
            if (propertyObject.hasProperty("Labels"))
            {
                juce::StringArray labels;

                auto labelsJSONArray = propertyObject.getArray("Labels");
                for (auto const& label : labelsJSONArray)
                {
                    labels.add(label.toString());
                }

                return labels;
            }

            juce::StringArray array;
            for (int i = 0; i < numSliders; ++i)
                array.add(juce::String{});
            return array;
        };
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

    auto effectsArray = effectInfoCollection.getArray("Effects");
    auto effectInfo = effectsArray.getObject((int)effect->effectType);
    auto jsonPropertiesArray = effectInfo.getArray("Properties");

    //
    // Effect selection
    //
    {
        juce::Array<juce::PropertyComponent*> sectionComponents;

        {
            juce::StringArray effectNames
            {
                "Gaussian blur",
                "Spot specular lighting",
                "Shadow",
                "Spot diffuse lighting",
                "Perspective transform 3D",
                "Blend"
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
        }

        {
            juce::StringArray strings
            {
                "M31 Galaxy",
                "Piet Mondriaan",
                "Van Gogh",
                "Checkerboard"
            };
            juce::Array<juce::var> values;
            for (int value = (int)m31Galaxy; value <= (int)checkerboard; ++value)
            {
                values.add(value);
            }

            sectionComponents.add(new juce::ChoicePropertyComponent{ sourceImageValue,
                "Image",
                strings,
                values });
        }

        sectionComponents.add(new juce::BooleanPropertyComponent{ showSourceImageValue, "Show source image", "" });

        propertyPanel.addSection("Effect", sectionComponents);
    }

    //
    // Effect-specific controls
    //
    {
        auto const& effectProperties = effect->getProperties();
        juce::Array<juce::PropertyComponent*> sectionComponents;

        size_t propertyIndex = 0;
        for (auto const& property : effectProperties)
        {
            EffectPropertyValueComponent* propertyComponent = nullptr;
            auto jsonPropertyInfo = jsonPropertiesArray.getObject(propertyIndex);

            if (std::holds_alternative<int>(property.defaultValue))
            {
                auto defaultIntValue = std::get<int>(property.defaultValue);
                propertyComponent = new MultiSliderPropertyComponent{ property.name,
                    propertyIndex,
                    property.defaultValue,
                    juce::Array<float> { (float)defaultIntValue },
                    juce::StringArray{ juce::String{} },
                    juce::Range<float>{ 0, 1 } };
            }
            else if (std::holds_alternative<float>(property.defaultValue))
            {
                auto defaultFloatValue = std::get<float>(property.defaultValue);
                auto range = getFloatRange(jsonPropertyInfo);

                propertyComponent = new MultiSliderPropertyComponent{ property.name,
                    propertyIndex,
                    property.defaultValue,
                    juce::Array<float> { defaultFloatValue },
                    juce::StringArray{ juce::String{} },
                    range };
            }
            else if (std::holds_alternative<mescal::Vector2>(property.defaultValue))
            {
                auto defaultValue = std::get<mescal::Vector2>(property.defaultValue);
                auto range = getFloatRange(jsonPropertyInfo);

                propertyComponent = new MultiSliderPropertyComponent{ property.name,
                    propertyIndex,
                    property.defaultValue,
                    juce::Array<float> { defaultValue[0], defaultValue[1] },
                    getLabels(jsonPropertyInfo, 2),
                    range };
            }
            else if (std::holds_alternative<mescal::Vector3>(property.defaultValue))
            {
                auto defaultValue = std::get<mescal::Vector3>(property.defaultValue);
                auto range = getFloatRange(jsonPropertyInfo);

                propertyComponent = new MultiSliderPropertyComponent{ property.name,
                    propertyIndex,
                    property.defaultValue,
                    juce::Array<float> { defaultValue[0], defaultValue[1], defaultValue[2] },
                    getLabels(jsonPropertyInfo, 3),
                    range };
            }
            else if (std::holds_alternative<mescal::Vector4>(property.defaultValue))
            {
                auto defaultValue = std::get<mescal::Vector4>(property.defaultValue);
                auto range = getFloatRange(jsonPropertyInfo);

                propertyComponent = new MultiSliderPropertyComponent{ property.name,
                    propertyIndex,
                    property.defaultValue,
                    juce::Array<float> { defaultValue[0], defaultValue[1], defaultValue[2], defaultValue[3] },
                    getLabels(jsonPropertyInfo, 4),
                    range };
            }
            else if (std::holds_alternative<uint8_t>(property.defaultValue))
            {
                auto defaultValue = std::get<uint8_t>(property.defaultValue);

                auto jsonStringArray = jsonPropertyInfo.getArray("Values");
                juce::StringArray stringArray;
                for (auto const& value : jsonStringArray)
                {
                    stringArray.add(value.toString());
                }

                propertyComponent = new EnumPropertyComponent{ property.name,
                    propertyIndex,
                    property.defaultValue,
                    stringArray };
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

        propertyPanel.addSection("Properties", sectionComponents);
    }

    for (auto& c : propertyValueComponents)
    {
        c->onChange = [this](size_t propertyIndex, const mescal::Effect::PropertyValue& propertyValue)
            {
                effect->setPropertyValue((int)propertyIndex, propertyValue);
                applyEffect();
                repaint();
            };
    }
}

void MainComponent::updateSourceImages()
{
    juce::NativeImageType imageType{};
    sourceImages.clear();

    switch ((int)sourceImageValue.getValue())
    {
    case m31Galaxy:
    {
        sourceImages.emplace_back(imageType.convert(juce::ImageFileFormat::loadFrom(BinaryData::M31Kennett_jpg, BinaryData::M31Kennett_jpgSize)));
        break;
    }

    case pietModriaan:
        sourceImages.emplace_back(imageType.convert(juce::ImageFileFormat::loadFrom(BinaryData::Piet_Mondriaan_19391942__Composition_10_jpg, BinaryData::Piet_Mondriaan_19391942__Composition_10_jpgSize)));
        break;

    case vanGogh:
        sourceImages.emplace_back(imageType.convert(juce::ImageFileFormat::loadFrom(BinaryData::VanGoghstarry_night_jpg, BinaryData::VanGoghstarry_night_jpgSize)));
        break;

    case checkerboard:
        sourceImages.emplace_back(juce::Image{ juce::Image::ARGB, 1000, 1000, true, imageType });
        {
            juce::Graphics g{ sourceImages.back() };
            g.setColour(juce::Colours::transparentBlack);
            g.getInternalContext().fillRect(sourceImages.back().getBounds(), true);

            g.fillCheckerBoard(sourceImages.back().getBounds().toFloat(), 50.0f, 50.0f, juce::Colours::transparentBlack, juce::Colours::lightgrey.withAlpha(0.5f));
        }
        break;
    }

    sourceImages.emplace_back(juce::Image{ juce::Image::ARGB, 1000, 1000, true, imageType });
    {
        juce::Graphics g{ sourceImages.back() };
        g.setColour(juce::Colours::transparentBlack);
        g.getInternalContext().fillRect(sourceImages.back().getBounds(), true);

        g.fillCheckerBoard(sourceImages.back().getBounds().toFloat(), 20.0f, 20.0f, juce::Colours::transparentBlack, juce::Colours::lightblue.withAlpha(0.5f));
    }

    juce::Rectangle<int> maxBounds;
    for (auto const& sourceImage : sourceImages)
    {
        maxBounds = maxBounds.getUnion(sourceImage.getBounds());
    }

    outputImage = juce::Image{ juce::Image::ARGB, maxBounds.getWidth(), maxBounds.getHeight(), true };

    repaint();
}

void MainComponent::updateEffectType()
{
    buildPropertyPanel();
    applyEffect();
    repaint();
}

void MainComponent::applyEffect()
{
    //effect->applyEffect(sourceImages.front(), outputImage, 1.0f, 1.0f, true);
    int index = 0;
    for (auto const& image : sourceImages)
    {
        effect->setInput(index++, image);
    }
    effect->applyEffect(outputImage, 1.0f, 1.0f, true);
}
