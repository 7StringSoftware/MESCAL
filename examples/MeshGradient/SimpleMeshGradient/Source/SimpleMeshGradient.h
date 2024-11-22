#pragma once

class SimpleMeshGradient : public juce::Component
{
public:
    SimpleMeshGradient()
    {
        setSize(768, 768);
    }

    void paint(juce::Graphics& g) override
    {
        //
        // Paint the gradient
        //
        meshGradient->draw(outputImage, {});

        //
        // Paint the output image
        //
        g.drawImageAt(outputImage, 0, 0);
    }

    void resized() override
    {
        createMeshGradient();

        outputImage = juce::Image{ juce::Image::ARGB, getWidth(), getHeight(), true };
    }

private:
    juce::Image outputImage;
    std::unique_ptr<mescal::MeshGradient> meshGradient;

    void createMeshGradient()
    {
        //
        // Make a mesh gradient with a single patch (1 row, 1 column)
        //
        meshGradient = std::make_unique<mescal::MeshGradient>(1, 1, getLocalBounds().toFloat());

        std::array<juce::Colour, 4> colors
        {
            juce::Colours::red,
            juce::Colours::yellow,
            juce::Colours::purple,
            juce::Colours::cyan
        };
        auto colorIterator = colors.begin();

        auto patch = meshGradient->getPatches().front();
        for (auto corner : meshGradient->cornerPlacements)
        {
            patch->setColor(corner, *colorIterator);
            ++colorIterator;
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleMeshGradient)
};
