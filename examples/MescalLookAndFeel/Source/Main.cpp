#include <JuceHeader.h>
#include "MescalLookAndFeel.h"
#include "ContentComponent.h"

class DemoApplication : public juce::JUCEApplication
{
public:
    DemoApplication() {}

    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        mescalLookAndFeel = std::make_unique<MescalLookAndFeel>();
        mescalLookAndFeel->setColour(juce::ResizableWindow::backgroundColourId, juce::Colour::greyLevel(0.95f));
        mescalLookAndFeel->setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
        mescalLookAndFeel->setColour(juce::Slider::ColourIds::textBoxTextColourId, juce::Colours::black);
        mescalLookAndFeel->setColour(juce::ToggleButton::ColourIds::textColourId, juce::Colours::black);
        mescalLookAndFeel->setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::black);
        mescalLookAndFeel->setColour(juce::ToggleButton::ColourIds::tickDisabledColourId, juce::Colours::lightgrey);
        mescalLookAndFeel->setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::black);
        mescalLookAndFeel->setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::black);
        mescalLookAndFeel->setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
        mescalLookAndFeel->setColour(juce::GroupComponent::ColourIds::textColourId, juce::Colours::black);
        mescalLookAndFeel->setColour(juce::DrawableButton::ColourIds::textColourId, juce::Colours::black);
        mescalLookAndFeel->setColour(juce::DrawableButton::ColourIds::textColourOnId, juce::Colours::white);
        LookAndFeel::setDefaultLookAndFeel(mescalLookAndFeel.get());

//         lookAndFeelV4 = std::make_unique<juce::LookAndFeel_V4>();
//         LookAndFeel::setDefaultLookAndFeel(lookAndFeelV4.get());

        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
        mescalLookAndFeel = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String&) override
    {
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name)
            : DocumentWindow(name, juce::Colour{ 0xffffffff }, DocumentWindow::allButtons)
        {
#if 0
            auto convertToAlpha = mescal::Effect::create(mescal::Effect::Type::luminanceToAlpha);

            auto lighting = mescal::Effect::SpotSpecularLighting::create()
                .withLightPosition(0.0f, 0.0f, 100.0f);
            //             mescal::Effect::Ptr lightingEffect = new mescal::Effect{ mescal::Effect::Type::spotSpecularLighting };
            //             lightingEffect->setPropertyValue(mescal::Effect::SpotSpecularLighting::lightPosition, mescal::Vector3{ 0.0f, 0.0f, 100.0f });
            //             lightingEffect->setPropertyValue(mescal::Effect::SpotSpecularLighting::pointsAt, mescal::Vector3{ 500.0f, 500.0f, 0.0f });
            //             lightingEffect->setPropertyValue(mescal::Effect::SpotSpecularLighting::color, juce::Colours::pink);
            lighting->addInput(convertToAlpha);

            auto spotDiffuse = mescal::Effect::create(mescal::Effect::Type::spotDiffuseLighting);
            spotDiffuse->setPropertyValue(mescal::Effect::SpotDiffuseLighting::lightPosition, mescal::Vector3{ 0.0f, 0.0f, 100.0f });
            spotDiffuse->addInput(convertToAlpha);

            auto composite = mescal::Effect::Composite::create(mescal::Effect::Composite::sourceOver);
            composite->setInput(1, spotDiffuse);

            imageEffectFilter = std::make_unique<mescal::MescalImageEffectFilter>(composite);
            content.widgets.setComponentEffect(imageEffectFilter.get());
            content.effectGraph.setOutputEffect(composite, 2048, 1024);

            setLookAndFeel(&lookAndFeelV4);
            lookAndFeelV4.setColourScheme(lookAndFeelV4.getLightColourScheme());
            lookAndFeelV4.setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::white);
            lookAndFeelV4.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
#endif

            setUsingNativeTitleBar(true);
            setContentOwned(new ContentComponent, true);

            setResizable(true, true);

            centreWithSize(2048, 1024);

            setVisible(true);
    }

        ~MainWindow()
        {
            setLookAndFeel(nullptr);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        std::unique_ptr<mescal::MescalImageEffectFilter> imageEffectFilter;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

private:
    std::unique_ptr<juce::LookAndFeel_V4> lookAndFeelV4;
    std::unique_ptr<MescalLookAndFeel> mescalLookAndFeel;
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(DemoApplication)

