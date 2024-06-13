#include "Base.h"
#include "ContentComponent.h"
#include "Controller.h"

class Application : public juce::JUCEApplication
{
public:
    Application() = default;

    const juce::String getApplicationName() override { return "MESCAL Demo"; }
    const juce::String getApplicationVersion() override { return ""; }

    void initialise(const juce::String&) override
    {
        controller = std::make_unique <Controller>();
        mainWindow.reset(new MainWindow("MESCAL Demo", new ContentComponent, *this));
    }

    void shutdown() override { mainWindow = nullptr; }

private:
    class MainWindow : public juce::DocumentWindow, public juce::LookAndFeel_V4
    {
    public:
        MainWindow(const juce::String& name, juce::Component* c, JUCEApplication& a)
            : DocumentWindow(name,
                juce::Colours::transparentBlack,
                juce::DocumentWindow::allButtons),
            app(a)
        {
            setUsingNativeTitleBar(false);
            setContentOwned(c, true);

            setResizable(true, true);
            setResizeLimits(300, 250, 10000, 10000);
            centreWithSize(getWidth(), getHeight());

            setOpaque(false);

            setVisible(true);

            setLookAndFeel(this);
        }

        ~MainWindow() override
        {
            setLookAndFeel(nullptr);
        }

        void closeButtonPressed() override
        {
            app.systemRequestedQuit();
        }

        void paint(juce::Graphics& g) override
        {
            DocumentWindow::paint(g);
        }

    private:
        JUCEApplication& app;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    std::unique_ptr<Controller> controller;
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(Application)
