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
            : DocumentWindow (name, juce::Colour{ 0xff111111 }, DocumentWindow::allButtons)
        {
            setLookAndFeel(&mescalLookAndFeel);
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
        MescalLookAndFeel mescalLookAndFeel;
        //WidgetsComponent widgets;
        WidgetsDemo widgets;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (DemoApplication)
