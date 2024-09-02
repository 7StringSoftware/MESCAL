#include <JuceHeader.h>
#include "WidgetsComponent.h"

using namespace juce;
#define PIP_JUCE_EXAMPLES_DIRECTORY_STRING "c:/JUCE/examples"
#include "c:\JUCE\examples\Assets\DemoUtilities.h"
#include "C:\JUCE\examples\GUI\WidgetsDemo.h"

class DemoApplication  : public juce::JUCEApplication
{
public:
    DemoApplication() {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise (const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow> (getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String&) override
    {
    }

    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name)
            : DocumentWindow (name, juce::Colour{ 0xffffffff }, DocumentWindow::allButtons)
        {
            setLookAndFeel(&mescalLookAndFeel);

#if 0
#if 0
            mescal::Effect::Ptr lightingEffect = new mescal::Effect{ mescal::Effect::Type::spotDiffuseLighting };
            lightingEffect->setPropertyValue(mescal::Effect::SpotDiffuseLighting::lightPosition, mescal::Vector3{ 0.0f, 0.0f, 100.0f });
            lightingEffect->setPropertyValue(mescal::Effect::SpotDiffuseLighting::pointsAt, mescal::Vector3{ 500.0f, 500.0f, 0.0f });
            lightingEffect->setPropertyValue(mescal::Effect::SpotDiffuseLighting::color, juce::Colours::pink);
#endif
            mescal::Effect::Ptr lightingEffect = new mescal::Effect{ mescal::Effect::Type::spotSpecularLighting };
            lightingEffect->setPropertyValue(mescal::Effect::SpotSpecularLighting::lightPosition, mescal::Vector3{ 0.0f, 0.0f, 100.0f });
            lightingEffect->setPropertyValue(mescal::Effect::SpotSpecularLighting::pointsAt, mescal::Vector3{ 500.0f, 500.0f, 0.0f });
            lightingEffect->setPropertyValue(mescal::Effect::SpotSpecularLighting::color, juce::Colours::pink);

            mescal::Effect::Ptr composite = new mescal::Effect{ mescal::Effect::Type::composite };
            composite->setInput(1, lightingEffect);
            composite->setPropertyValue(mescal::Effect::Composite::mode, mescal::Effect::Composite::sourceOver);

            mescal::Effect::Ptr blur = new mescal::Effect{ mescal::Effect::Type::gaussianBlur };


            imageEffectFilter = std::make_unique<mescal::MescalImageEffectFilter>(composite);
            widgets.setComponentEffect(imageEffectFilter.get());

            setLookAndFeel(&lookAndFeelV4);
            lookAndFeelV4.setColourScheme(lookAndFeelV4.getLightColourScheme());
            lookAndFeelV4.setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::white);
#endif

            lookAndFeelV4.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);

            setUsingNativeTitleBar (true);
            setContentNonOwned(&widgets, true);

            setResizable (true, true);

            centreWithSize(2048, 1024);

            setVisible (true);
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
        juce::LookAndFeel_V4 lookAndFeelV4;
        MescalLookAndFeel mescalLookAndFeel;
        //WidgetsComponent widgets;
        WidgetsDemo widgets;
        std::unique_ptr<mescal::MescalImageEffectFilter> imageEffectFilter;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (DemoApplication)
