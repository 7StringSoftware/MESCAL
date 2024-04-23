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
    commandManager.setFirstCommandTarget(this);
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


#if 0
    static juce::String const fileCategory("juce::File");
    static juce::String const editCategory("Edit");

    switch (commandID)
    {
    case NEW_FILE:
        result.setInfo(juce::translate("New file"),
            juce::translate("Create a new JSON file"),
            fileCategory, 0);
        result.addDefaultKeypress('n', juce::ModifierKeys::commandModifier);
        break;

    case OPEN_FILE:
        result.setInfo(juce::translate("Open file"),
            juce::translate("Open a JSON file"),
            fileCategory, 0);
        result.addDefaultKeypress('o', juce::ModifierKeys::commandModifier);
        break;

    case SAVE_FILE:
        result.setInfo(juce::translate("Save file"),
            juce::translate("Save the current JSON file"),
            fileCategory, 0);
        result.addDefaultKeypress('s', juce::ModifierKeys::commandModifier);
        //            result.setActive(activeDocument != nullptr &&
        //                activeDocument->hasChangedSinceSaved() &&
        //                activeDocument->getFile().existsAsFile());
        break;

    case SAVE_FILE_AS:
        result.setInfo(juce::translate("Save file as..."),
            juce::translate("Save the current JSON file with a different name"),
            fileCategory, 0);
        //            result.setActive(activeDocument != nullptr);
        break;

    case SAVE_ALL:
        result.setInfo(juce::translate("Save all..."),
            juce::translate("Save all modified files"),
            fileCategory, 0);;
        //            {
        //                int modifiedDocumentCount = 0;
        //                auto const documentsArray{ documents.getDocuments() };
        //                for (auto const &document : documentsArray)
        //                {
        //                    if (document->hasChangedSinceSaved() && document->getFile().existsAsFile())
        //                    {
        //                        modifiedDocumentCount++;
        //                    }
        //                }
        //                result.setActive(modifiedDocumentCount > 0);
        //            }
        break;

    case CLOSE_FILE:
        result.setInfo(juce::translate("Close file..."),
            juce::translate("Close the current JSON file"),
            fileCategory, 0);
        //            result.setActive(activeDocument != nullptr);
        break;

    case CLOSE_ALL:
        result.setInfo(juce::translate("Close all files..."),
            juce::translate("Close all open JSON file"),
            fileCategory, 0);
        //			result.setActive(activeDocument != nullptr);
        break;

    case CLEAR_RECENT_FILES:
        result.setInfo(juce::translate("Clear recent file list"),
            juce::translate("Clear recent file list"),
            fileCategory, 0);
        result.setActive(mainMenu->recentlyOpenedFiles.getNumFiles() > 0);
        break;

    case EXPORT_SCHEMA:
        result.setInfo(juce::translate("Export schema..."),
            juce::translate("Export a JSON schema based on the current file"),
            fileCategory, 0);
        //            result.setActive(activeDocument != nullptr);
        break;

    case EXIT:
        result.setInfo(juce::translate("Exit"),
            juce::translate("Exit the program"),
            fileCategory, 0);
#if JUCE_MAC
        result.addDefaultKeypress('w', juce::ModifierKeys::commandModifier);
#endif
        break;

    case ADD_OBJECT:
        result.setInfo(juce::translate("Add object..."),
            juce::translate("Add JSON object"),
            editCategory, 0);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				result.setActive(activeDocument->canAddType(Identifiers::ObjectType));
        //			}
        break;

    case ADD_ARRAY:
        result.setInfo(juce::translate("Add array..."),
            juce::translate("Add JSON array"),
            editCategory, 0);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				result.setActive(activeDocument->canAddType(Identifiers::ArrayType));
        //			}
        break;

    case ADD_INT32:
        result.setInfo(juce::translate("Add 32-bit integer value..."),
            juce::translate("Add 32-bit integer value"),
            editCategory, 0);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				result.setActive(activeDocument->canAddType(Identifiers::ScalarType));
        //			}
        break;

    case ADD_INT64:
        result.setInfo(juce::translate("Add 64-bit integer value..."),
            juce::translate("Add 64-bit integer value"),
            editCategory, 0);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				result.setActive(activeDocument->canAddType(Identifiers::ScalarType));
        //			}
        break;

    case ADD_DOUBLE:
        result.setInfo(juce::translate("Add floating-point value..."),
            juce::translate("Add floating-point value"),
            editCategory, 0);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				result.setActive(activeDocument->canAddType(Identifiers::ScalarType));
        //			}
        break;

    case ADD_STRING:
        result.setInfo(juce::translate("Add string value..."),
            juce::translate("Add text string value"),
            editCategory, 0);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				result.setActive(activeDocument->canAddType(Identifiers::ScalarType));
        //			}
        break;

    case ADD_BOOLEAN:
        result.setInfo(juce::translate("Add Boolean value..."),
            juce::translate("Add Boolean value"),
            editCategory, 0);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				result.setActive(activeDocument->canAddType(Identifiers::ScalarType));
        //			}
        break;

    case REMOVE_PROPERTY:
        result.setInfo(juce::translate("Remove..."),
            juce::translate("Remove current JSON property"),
            editCategory, 0);
        result.addDefaultKeypress(juce::KeyPress::deleteKey, 0);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				result.setActive(activeDocument->selectedTree.isValid());
        //			}
        break;

    case UNDO:
        result.setInfo(juce::translate("Undo"),
            juce::translate("Undo last edit"),
            editCategory, 0);
        result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				result.setActive(activeDocument->canUndo());
        //			}
        break;

    case CUT_PROPERTY:
        result.setInfo(juce::translate("Cut"),
            juce::translate("Cut property"),
            editCategory, 0);
        result.addDefaultKeypress('x', juce::ModifierKeys::commandModifier);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				result.setActive(activeDocument->canCut());
        //			}
        break;

    case COPY_PROPERTY:
        result.setInfo(juce::translate("Copy"),
            juce::translate("Copy property"),
            editCategory, 0);
        result.addDefaultKeypress('c', juce::ModifierKeys::commandModifier);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				result.setActive(activeDocument->canCopy());
        //			}
        break;

    case PASTE_PROPERTY:
        result.setInfo(juce::translate("Paste"),
            juce::translate("Paste property"),
            editCategory, 0);
        result.addDefaultKeypress('v', juce::ModifierKeys::commandModifier);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				result.setActive(activeDocument->canPaste());
        //			}
        break;

    case EXPAND:
        result.setInfo(juce::translate("Expand"),
            juce::translate("Expand this item"),
            editCategory, 0);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				ValueTree &selectedTree(activeDocument->selectedTree);
        //				bool active = (selectedTree.hasType(Identifiers::ArrayType) || selectedTree.hasType(Identifiers::ObjectType)) &&
        //					!(bool)selectedTree[Identifiers::Open];
        //				result.setActive(active);
        //			}
        break;

    case EXPAND_CHILDREN:
        result.setInfo(juce::translate("Expand children"),
            juce::translate("Expand this item's children"),
            editCategory, 0);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				ValueTree &selectedTree(activeDocument->selectedTree);
        //
        //				selectedTree.setProperty(Identifiers::Open, true, nullptr);
        //
        //				for (int childIndex = 0; childIndex < selectedTree.getNumChildren(); ++childIndex)
        //				{
        //					ValueTree child(selectedTree.getChild(childIndex));
        //					if ((child.hasType(Identifiers::ArrayType) || child.hasType(Identifiers::ObjectType)) &&
        //						false == (bool)child[Identifiers::Open])
        //					{
        //						result.setActive(true);
        //						break;
        //					}
        //				}
        //			}
        break;

    case COLLAPSE:
        result.setInfo(juce::translate("Collapse"),
            juce::translate("Collapse this item"),
            editCategory, 0);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				ValueTree &selectedTree(activeDocument->selectedTree);
        //				bool active = (selectedTree.hasType(Identifiers::ArrayType) || selectedTree.hasType(Identifiers::ObjectType)) &&
        //					(bool)selectedTree[Identifiers::Open];
        //				result.setActive(active);
        //			}
        break;

    case COLLAPSE_CHILDREN:
        result.setInfo(juce::translate("Collapse children"),
            juce::translate("Collapse this item's children"),
            editCategory, 0);
        //			result.setActive(false);
        //			if (activeDocument != nullptr)
        //			{
        //				ValueTree &selectedTree(activeDocument->selectedTree);
        //				for (int childIndex = 0; childIndex < selectedTree.getNumChildren(); ++childIndex)
        //				{
        //					ValueTree child(selectedTree.getChild(childIndex));
        //					if ((child.hasType(Identifiers::ArrayType) || child.hasType(Identifiers::ObjectType)) &&
        //						(bool)child[Identifiers::Open])
        //					{
        //						result.setActive(true);
        //						break;
        //					}
        //				}
        //			}
        break;
    }
#endif
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
#if 0
    case NEW_FILE:
    {
        newFile();
        return true;
    }

    case OPEN_FILE:
    {
        loadFile();
        return true;
    }

    case SAVE_FILE:
    {
        saveFile();
        return true;
    }

    case SAVE_FILE_AS:
    {
        saveFileAs();
        return true;
    }

    case SAVE_ALL:
    {
        saveAll();
        return true;
    }

    case CLOSE_FILE:
    {
        closeFile();
        return true;
    }

    case CLOSE_ALL:
    {
        closeAllFiles();
        return true;
    }

    case CLEAR_RECENT_FILES:
    {
        clearRecentFiles();
        return true;
    }

    case EXPORT_SCHEMA:
    {
        exportSchema();
        return true;
    }

    case EXIT:
    {
        exit();
        return true;
    }

    case ADD_OBJECT:
    {
        addObject();
        return true;
    }

    case ADD_ARRAY:
    {
        addArray();
        return true;
    }

    case ADD_INT32:
    {
        int i32 = 0;
        addScalar(juce::var(i32));
        return true;
    }

    case ADD_INT64:
    {
        int64_t i64 = 0;
        addScalar(juce::var(i64));
        return true;
    }

    case ADD_DOUBLE:
    {
        double d = 0.0;
        addScalar(juce::var(d));
        return true;
    }

    case ADD_STRING:
    {
        juce::String text("value");
        addScalar(juce::var(text));
        return true;
    }

    case ADD_BOOLEAN:
    {
        bool b = true;
        addScalar(juce::var(b));
        return true;
    }

    case REMOVE_PROPERTY:
    {
        removeProperty();
        return true;
    }

    case UNDO:
    {
        undo();
        return true;
    }

    case CUT_PROPERTY:
    {
        cutProperty();
        return true;
    }

    case COPY_PROPERTY:
    {
        copyProperty();
        return true;
    }

    case PASTE_PROPERTY:
    {
        pasteProperty();
        return true;
    }

    case EXPAND:
    {
        setSelectedTreeOpenness(true);
        return true;
    }

    case EXPAND_CHILDREN:
    {
        setSelectedTreeChildOpenness(true);
        return true;
    }

    case COLLAPSE:
    {
        setSelectedTreeOpenness(false);
        return true;
    }

    case COLLAPSE_CHILDREN:
    {
        setSelectedTreeChildOpenness(false);
        return true;
    }
#endif
    }

    return false;
}

void Commander::quit()
{
    //controller->initiateShutdown();
}
