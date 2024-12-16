#include <JuceHeader.h>
#include "InteractiveMeshGradient.h"

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
            : DocumentWindow (name, juce::Colour{ 0xff111111 }, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentNonOwned(&interactiveMeshGradient, true);

            setResizable (true, true);

            auto area = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.reduced(40);

            centreWithSize(area.getWidth(), area.getHeight());

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        InteractiveMeshGradient interactiveMeshGradient;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (DemoApplication)
