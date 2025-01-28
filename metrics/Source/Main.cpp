#include <windows.h>
#include <Psapi.h>
#include <JuceHeader.h>
#include "PipeClient.h"
#include "StatTable.h"
#include "MetricsContentComponent.h"

class Direct2DMetricsApplication : public juce::JUCEApplication
{
public:
    Direct2DMetricsApplication() {}

    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
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
            : DocumentWindow(name,
                juce::Colours::white,
                DocumentWindow::allButtons)
        {
            setAlpha(0.8f);
            setOpaque(true);
            setDropShadowEnabled(false);
            setUsingNativeTitleBar(false);
            setContentNonOwned(&contentComponent, false);

            setResizable(true, true);
            setSize(contentComponent.getDesiredWidth(), contentComponent.getDesiredHeight() + getTitleBarHeight());

            auto const& userArea = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
            setTopLeftPosition(userArea.getWidth() - getWidth(), userArea.getY() + (userArea.getHeight() - getHeight()) / 2);

            setAlwaysOnTop(true);

            setVisible(true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        MetricsContentComponent contentComponent;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(Direct2DMetricsApplication)
