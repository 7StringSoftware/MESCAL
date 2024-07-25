
namespace mescal
{
    static const char* const effectData =
        R"(
            {
                "gaussianBlur": {
                    "properties": [
                        {
                            "name": "Standard deviation",
                            "defaultValue": 3.0,
                            "range": {
                                "min": 0.0,
                                "max": 100.0
                            }
                        },
                        {
                            "name": "Optimization mode",
                            "defaultValue": 1,
                            "values": [ "Speed", "Balanced", "Quality" ]
                        },
                        {
                            "name": "Border mode",
                            "defaultValue": 1,
                            "values": [ "Soft", "Hard" ]
                        }
                    ]
                },
                "spotSpecularLighting": {
                    "properties": [
                        {
                            "name": "Light position",
                            "defaultValue": [0.0, 0.0, 0.0]
                        },
                        {
                            "name": "Focus point position",
                            "defaultValue": [0.0, 0.0, 0.0]
                        },
                        {
                            "name": "Focus",
                            "defaultValue": 1.0,
                            "range": {
                                "min": 0.0,
                                "max": 200.0
                            }
                        },
                        {
                            "name": "Limiting cone angle",
                            "defaultValue": 1.0,
                            "range": {
                                "min": 0.0,
                                "max": 90.0
                            }
                        },
                        {
                            "name": "Specular exponent",
                            "defaultValue": 1.0,
                            "range": {
                                "min": 1.0,
                                "max": 128.0
                            }
                        },
                        {
                            "name": "Specular constant",
                            "defaultValue": 1.0,
                            "range": {
                                "min": 0.0,
                                "max": 10000.0
                            }
                        },
                        {
                            "name": "Surface scale",
                            "defaultValue": 1.0,
                            "range": {
                                "min": 0.0,
                                "max": 10000.0
                            }
                        },
                        {
                            "name": "Light color",
                            "defaultValue": [1.0, 1.0, 1.0]
                        }
                    ]
                }
            }
        )";

    template <typename T> struct ValueRange
    {
        T min;
        T max;
    };

    struct StringTableSection
    {
        size_t start;
        size_t numStrings;
    };

    void Effect::initProperties()
    {
        std::array<std::string_view, 5> stringTable
        {
            "Speed", "Balanced", "Quality",
            "Soft", "Hard"
        };

        struct PropertyInfo
        {
            char const* const name;
            Effect::PropertyValue defaultValue;
            std::optional<std::variant<ValueRange<int>, ValueRange<float>>> range;
        };

        static constexpr std::array<PropertyInfo, 3> gaussianBlurProperties
        {
            PropertyInfo{ "Standard deviation", 3.0f, ValueRange<float>{ 0.0f, 100.0f } },
            PropertyInfo{ "Optimization mode", 1, ValueRange<int>{ 0, 3 } },
            PropertyInfo{ "Border mode", 0, ValueRange<int>{ 0, 2 } }
        };

        static constexpr std::array<PropertyInfo, 8> spotSpecularLightingProperties
        {
            PropertyInfo{ "Light position", Point3D{ 0.0f, 0.0f, 0.0f } },
            PropertyInfo{ "Focus point position", Point3D{ 0.0f, 0.0f, 0.0f } },
            PropertyInfo{ "Focus", 1.0f, ValueRange<float>{ 0.0f, 200.0f } },
            PropertyInfo{ "Limiting cone angle", 1.0f, ValueRange<float>{ 0.0f, 90.0f } },
            PropertyInfo{ "Specular exponent", 1.0f, ValueRange<float>{ 1.0f, 128.0f } },
            PropertyInfo{ "Specular constant", 1.0f, ValueRange<float>{ 0.0f, 10000.0f } },
            PropertyInfo{ "Surface scale", 1.0f, ValueRange<float>{ 0.0f, 10000.0f } },
            PropertyInfo{ "Light color", RGBColor{ 1.0f, 1.0f, 1.0f } }
        };

        struct PropertyGroup
        {
            PropertyInfo const* const properties;
            size_t numProperties;
        };

        static constexpr std::array<PropertyGroup, 2> propertyGroups
        {
            PropertyGroup{ gaussianBlurProperties.data(), gaussianBlurProperties.size() },
            PropertyGroup{ spotSpecularLightingProperties.data(), spotSpecularLightingProperties.size() }
        };

        auto const& propertyGroup = propertyGroups[static_cast<int>(effectType)];
        for (size_t index = 0; index < propertyGroup.numProperties; ++index)
        {
            auto const& propertyInfo = propertyGroup.properties[index];

            Effect::Property property
            {
                propertyInfo.name,
                propertyInfo.defaultValue
            };

            if (propertyInfo.range.has_value())
            {
                if (std::holds_alternative<ValueRange<int>>(*propertyInfo.range))
                {
                    auto const& valueRange = std::get<ValueRange<int>>(*propertyInfo.range);
                    property.range = juce::Range<int>{ valueRange.min, valueRange.max };
                }
                else
                {
                    auto const& valueRange = std::get<ValueRange<float>>(*propertyInfo.range);
                    property.range = juce::Range<float>{ valueRange.min, valueRange.max };
                }
            }

            pimpl->properties.push_back(property);
        }
    }

}
