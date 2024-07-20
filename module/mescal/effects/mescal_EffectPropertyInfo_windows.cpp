
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
        }

        return {};
    }
}

