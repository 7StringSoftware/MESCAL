#include "Base.h"
#include "Controller.h"
#include "Commander.h"

Commander::Commander() :
    controller(nullptr)
{
}

void Commander::initialize(Controller* controller_)
{
    controller = controller_;

    commandManager.registerAllCommandsForTarget(this);
}

void Commander::shutdown()
{
}

void Commander::changeListenerCallback(juce::ChangeBroadcaster* /*source*/)
{
    commandManager.commandStatusChanged();
}

juce::ApplicationCommandTarget* Commander::getNextCommandTarget()
{
    return nullptr;
}

void Commander::getAllCommands(juce::Array<juce::CommandID>& commands)
{
    for (int commandID = firstCommand; commandID < lastCommand; commandID++)
    {
        commands.add(commandID);
    }
}

void Commander::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
    char constexpr applicationCategory[] = "Application";
    char constexpr settingsCategory[] = "Settings";

    switch (commandID)
    {

    case quitCommand:
    {
        result.setInfo(juce::translate("Quit"),
            juce::translate("Quit application"),
            applicationCategory, 0);
        break;
    }
    }
}

bool Commander::perform(const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case quitCommand:
    {
        quit();
        return true;
    }
    }

    return false;
}

void Commander::quit()
{
    //controller->initiateShutdown();
}
