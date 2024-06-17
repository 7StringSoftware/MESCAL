#include <JuceHeader.h>
#include "Settings.h"

static void setDefaultProperty(ValueTree tree, juce::Identifier const& propertyName, juce::var const value)
{
    if (false == tree.hasProperty(propertyName))
    {
        tree.setProperty(propertyName, value, nullptr);
    }
}

Settings::Settings() :
    settingsTree(settingsID)
{
    juce::PropertiesFile::Options options;
    options.applicationName = "GradientMesh";
    options.folderName = "MESCAL";
    options.filenameSuffix = "xml";
    options.osxLibrarySubFolder = "Application Support";
    propfile = std::make_unique<juce::PropertiesFile>(options);

    std::unique_ptr<juce::XmlElement> xml{ propfile->getXmlValue(settingsTree.getType().toString()) };
    if (xml)
    {
        auto xmlTree{ ValueTree::fromXml(*xml) };
        settingsTree.copyPropertiesAndChildrenFrom(xmlTree, nullptr);
    }

    setMissingDefaults();
}

Settings::~Settings()
{
    save();
}

void Settings::save()
{
    std::unique_ptr<juce::XmlElement> xml{ settingsTree.createXml() };
    if (xml)
    {
        propfile->setValue(settingsTree.getType().toString(), xml.get());
        propfile->save();
    }
}

void Settings::setMissingDefaults()
{
}
