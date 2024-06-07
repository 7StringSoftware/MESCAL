#pragma once

class Settings
{
private:
    std::unique_ptr<juce::PropertiesFile> propfile;

    void setMissingDefaults();

public:
    Settings();
    ~Settings();

    void save();

    static void setDefaultValues(ValueTree& tree, juce::NamedValueSet const& defaultValues)
    {
        for (auto const& defaultValue : defaultValues)
        {
            if (!tree.hasProperty(defaultValue.name))
            {
                tree.setProperty(defaultValue.name, defaultValue.value, nullptr);
            }
        }
    }

    ValueTree settingsTree;

    static constexpr char defaultSubdirectoryName[] = "MESCAL";
    Identifier const settingsID{ "Settings" };

};
