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
        auto blue = (float)std::sin(phase) * 0.5f + 0.5f;
        auto patch = meshGradient->getPatch(0, 0);
        for (auto& color : patch->colors)
        {
            color.blue = blue;
        }

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

    juce::VBlankAttachment vblank
    {
        this,
        [this]
        {
            auto msec = juce::Time::getMillisecondCounterHiRes();
            auto elapsedMsec = msec - lastMsec;
            lastMsec = msec;

            phase += elapsedMsec * 0.001 * 0.1 * juce::MathConstants<double>::twoPi;
            while (phase >= juce::MathConstants<double>::twoPi)
                phase -= juce::MathConstants<double>::twoPi;

            repaint();
        }
    };
    double lastMsec = juce::Time::getMillisecondCounterHiRes();
    double phase = 0.0;

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
        auto patch = meshGradient->getPatch(0, 0);

        for (int i = 0; i < 4; ++i)
        {
            patch->colors[i] = mescal::Color128{ *colorIterator++ };
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleMeshGradient)
};
