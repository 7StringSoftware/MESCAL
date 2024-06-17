#pragma once

#include "Base.h"

class Controller;

class Commander : public juce::ApplicationCommandTarget, public juce::ChangeListener
{
public:
    Commander();
    ~Commander() override = default;

    void initialize(Controller* controller_);
    void shutdown();

    juce::ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands(juce::Array<juce::CommandID>& commands) override;
    void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    enum
    {
        firstCommand = 1,

        quitCommand = firstCommand,

        lastCommand
    };

    juce::ApplicationCommandManager commandManager;

private:
    Controller* controller;

    void quit();
};
