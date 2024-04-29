#pragma once

struct Mesher
{
    Mesher(Path&& p);

    juce::SortedSet<float> rowHeights;
    juce::SortedSet<float> columnWidths;
    juce::Path path;
};


