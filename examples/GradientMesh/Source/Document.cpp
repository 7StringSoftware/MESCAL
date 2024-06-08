#include "Document.h"

Document::Document(Settings& settings_) :
    settings(settings_),
    juce::FileBasedDocument(".mescal.gradientmesh", "*.mescal.gradientmesh", "Open a gradient mesh file", "Save gradient mesh")
{
    if (auto gradientMeshJSON = settings.settingsTree.getProperty("GradientMesh"); gradientMeshJSON.isString())
    {
        DBG(gradientMeshJSON.toString());

        juce::var jsonVar;
        auto result = juce::JSON::parse(gradientMeshJSON, jsonVar);
        if (result.wasOk())
        {
            gradientMesh.loadFromJSON(mescal::JSONObject{ jsonVar });

            auto check = gradientMesh.toJSON().toString();
            DBG(check);

            jassert(check == gradientMeshJSON.toString());
        }
        return;
    }

    gradientMesh.addPatch({ 100.0f, 100.0f });
}

Document::~Document()
{
    settings.settingsTree.setProperty("GradientMesh", gradientMesh.toJSON().toString(), nullptr);
}

juce::Result Document::loadDocument(const juce::File& file)
{
    return juce::Result::ok();
}

juce::Result Document::saveDocument(const juce::File& file)
{
    return juce::Result::ok();
}

juce::File Document::getLastDocumentOpened()
{
    return {};
}

void Document::setLastDocumentOpened(const File&)
{
}

juce::String Document::getDocumentTitle()
{
    return "Gradient Mesh";
}
