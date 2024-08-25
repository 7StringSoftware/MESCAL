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
        // Paint the conic gradient
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
        meshGradient = std::make_unique<mescal::MeshGradient>(2, 2, getLocalBounds().toFloat());

        std::array<juce::Colour, 4> colors
        {
            juce::Colours::red,
            juce::Colours::yellow,
            juce::Colours::purple,
            juce::Colours::cyan
        };
        auto colorIterator = colors.begin();
        for (int row = 0; row < meshGradient->getNumRows(); ++row)
        {
            for (int column = 0; column < meshGradient->getNumColumns(); ++column)
            {
                meshGradient->getVertex(row, column)->color = mescal::Color128{ *colorIterator++ };
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleMeshGradient)
};
