
namespace mescal
{
    Effect::PropertyInfo Effect::getPropertyInfo(int index)
    {
        switch (effectType)
        {
        case Effect::Type::gaussianBlur:
        {
            switch (index)
            {
            case GaussianBlurProperty::standardDeviation:
                return Effect::PropertyInfo{ "Standard deviation", 3.0f, juce::Range<float>{ 0.0f, 100.0f } };

            case GaussianBlurProperty::optimization:
                return Effect::PropertyInfo{ "Optimization mode", 1, juce::StringArray{ "Speed", "Balanced", "Quality" } };

            case GaussianBlurProperty::borderMode:
                return Effect::PropertyInfo{ "Border mode", 0, juce::StringArray{ "Soft", "Hard" } };
            }
        }

        case Effect::Type::spotSpecularLighting:
        {
            switch (index)
            {
            case SpotSpecularLightingProperty::lightPosition:
                return Effect::PropertyInfo{ "Light position", Point3D{ 0.0f, 0.0f, 0.0f } };

            case SpotSpecularLightingProperty::focusPointPosition:
                return Effect::PropertyInfo{ "Focus point position", Point3D{ 0.0f, 0.0f, 0.0f } };

            case SpotSpecularLightingProperty::focus:
                return Effect::PropertyInfo{ "Focus", 1.0f, juce::Range<float>{ 0.0f, 200.0f } };

            case SpotSpecularLightingProperty::limitingConeAngle:
                return Effect::PropertyInfo{ "Limiting cone angle", 1.0f, juce::Range<float>{ 0.0f, 90.0f } };

            case SpotSpecularLightingProperty::specularExponent:
                return Effect::PropertyInfo{ "Specular exponent", 1.0f, juce::Range<float>{ 1.0f, 128.0f } };

            case SpotSpecularLightingProperty::specularConstant:
                return Effect::PropertyInfo{ "Specular constant", 1.0f, juce::Range<float>{ 0.0f, 10000.0f } };

            case SpotSpecularLightingProperty::surfaceScale:
                return Effect::PropertyInfo{ "Surface scale", 1.0f, juce::Range<float>{ 0.0f, 10000.0f } };

            case SpotSpecularLightingProperty::lightColor:
                return Effect::PropertyInfo{ "Light color", RGBColor{ 1.0f, 1.0f, 1.0f } };

            case SpotSpecularLightingProperty::kernelUnitLength:
                return Effect::PropertyInfo{ "Kernel unit length", juce::Point<float>{ 1.0f, 1.0f } };

            case SpotSpecularLightingProperty::scaleMode:
                return Effect::PropertyInfo{ "Scale mode", 1, juce::StringArray{ "Nearest neighbor", "Linear", "Cubic", "Multi-sample linear", "Anisotropic", "High-quality cubic" } };
            }
        }

        case Effect::Type::shadow:
        {
            switch (index)
            {
            case ShadowProperty::standardDeviation:
                return Effect::PropertyInfo{ "Standard deviation", 3.0f, juce::Range<float>{ 0.0f, 100.0f } };

            case ShadowProperty::color:
                return Effect::PropertyInfo{ "Color", juce::Colours::black };

            case ShadowProperty::optimization:
                return Effect::PropertyInfo{ "Optimization mode", 1, juce::StringArray{ "Speed", "Balanced", "Quality" } };
            }
        }

        }

        return {};
    }
}
