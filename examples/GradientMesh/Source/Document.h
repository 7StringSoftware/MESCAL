#pragma once

#include "Base.h"

class Document : juce::FileBasedDocument
{
public:
    Document();
    ~Document() override;

    juce::Result loadDocument(const juce::File& file) override;
    juce::Result saveDocument(const juce::File& file) override;
    juce::File getLastDocumentOpened() override;
    void setLastDocumentOpened(const juce::File& file) override;
    juce::String getDocumentTitle() override;

    std::unique_ptr<GradientMesh> gradientMesh;
};
