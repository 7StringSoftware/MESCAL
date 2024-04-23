#pragma once

#include "Base.h"

class Document : juce::FileBasedDocument
{
public:
    Document();
    ~Document() override;

    juce::Result loadDocument(const juce::File& file) override;
    juce::Result saveDocument(const juce::File& file) override;
};
