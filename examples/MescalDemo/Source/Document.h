#pragma once

#include "Base.h"
#include "Settings.h"

class Document : juce::FileBasedDocument
{
public:
    Document(Settings& settings_);
    ~Document() override;

    juce::Result loadDocument(const juce::File& file) override;
    juce::Result saveDocument(const juce::File& file) override;
    juce::File getLastDocumentOpened() override;
    void setLastDocumentOpened(const juce::File& file) override;
    juce::String getDocumentTitle() override;

    Settings& settings;
};
