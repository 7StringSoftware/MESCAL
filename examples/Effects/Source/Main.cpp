#include <JuceHeader.h>
#include "MainComponent.h"
#include "FormatConverter.h"
#include <memory>

class Direct2DEffectsApplication  : public juce::JUCEApplication
{
public:
    Direct2DEffectsApplication() {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise (const juce::String&) override
    {
#if MESCAL_UNIT_TESTS
        juce::UnitTestRunner runner;
        runner.runAllTests();
#endif

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
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentNonOwned(&mainComponent, true);

            setResizable (true, true);

            if (auto display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
            {
                setBounds (display->userArea.reduced (100));
                mainComponent.initEffect();
            }

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        MainComponent mainComponent;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (Direct2DEffectsApplication)
