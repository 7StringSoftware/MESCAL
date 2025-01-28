#pragma once

class MetricsContentComponent : public juce::Component, public juce::Timer
{
public:
    MetricsContentComponent()
    {
        setOpaque(false);

        startTimer(200);

        setSize(100, 100);

        statTable.resetStatsButton.onClick = [this]
            {
                if (pipeClient)
                {
                    pipeClient->resetAllMetrics();
                }
            };
        addAndMakeVisible(statTable);

        addAndMakeVisible(pipeComboBox);
        pipeComboBox.onChange = [this]
            {
                if (pipeClient && pipeComboBox.getText() != pipeClient->pipeName)
                {
                    pipeClient.reset();
                }

                if (!pipeClient && pipeComboBox.getText().isNotEmpty())
                    addClient(pipeComboBox.getText());
            };

        //addAndMakeVisible(windowHandleComboBox);
    }

    ~MetricsContentComponent() override
    {
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour{ 0xff111111 });

#if 0
        g.setColour(juce::Colours::white);
        if (statTable.isEnabled() && statTable.isVisible())
        {
            juce::String text;
            auto selectedWindowHandle = getSelectedWindowHandle();
            if (selectedWindowHandle)
            {
                TCHAR windowName[64] = {};
                GetWindowText((HWND)getSelectedWindowHandle(), windowName, 64);

                text << juce::String{ windowName };
                text << " - HWND 0x" << juce::String::toHexString((juce::pointer_sized_int)selectedWindowHandle);
            }
            else
            {
                text << "Images";
            }

            g.drawText(text,
                15, 0, getWidth() - 30, 25,
                juce::Justification::centredRight);
        }
        else
        {
            g.drawText("No metrics available", getLocalBounds(), juce::Justification::centred);
        }
#endif
    }

    int getDesiredWidth()
    {
        return statTable.getDesiredWidth();
    }

    int getDesiredHeight()
    {
        return statTable.getDesiredHeight() + 75;
    }

    void resized() override
    {
        statTable.setBounds(getLocalBounds().withTrimmedTop(25));
        pipeComboBox.setBounds(0, 0, getWidth(), 25);
        //windowHandleComboBox.setBounds(0, pipeComboBox.getBottom(), getWidth(), 25);
    }

    void timerCallback() override
    {
        auto now = juce::Time::getCurrentTime();

        WIN32_FIND_DATA findData{};
        auto processID = (int)GetCurrentProcessId();

        juce::StringArray foundPipeNames;
        if (auto pipeHandle = FindFirstFileA("\\\\.\\pipe\\JUCEDirect2DMetrics*", &findData); pipeHandle != INVALID_HANDLE_VALUE)
        {
            auto pipeName = juce::String{ findData.cFileName, juce::numElementsInArray(findData.cFileName) };

            auto suffix = pipeName.getLastCharacters(4);
            auto pipeProcessID = suffix.getHexValue32();
            if (pipeProcessID != processID)
                foundPipeNames.add(pipeName);

            while (FindNextFile(pipeHandle, &findData))
            {
                pipeName = juce::String{ findData.cFileName, juce::numElementsInArray(findData.cFileName) };
                pipeProcessID = pipeName.getTrailingIntValue();
                if (pipeProcessID != processID)
                    foundPipeNames.add(pipeName);
            }

            FindClose(pipeHandle);
        }

        if (pipeNames != foundPipeNames)
        {
            pipeNames = foundPipeNames;
            pipeComboBox.clear(juce::dontSendNotification);
            int id = 1;
            for (auto const& pipeName : foundPipeNames)
            {
                pipeComboBox.addItem(pipeName, id++);
            }
        }

#if 0
        if (pipeComboBox.getNumItems() == 0)
        {
            windowHandleComboBox.clear(juce::dontSendNotification);
        }
#endif

        if (pipeComboBox.getSelectedId() == 0 && foundPipeNames.size() > 0 || pipeClient == nullptr)
        {
            pipeComboBox.setSelectedId(1, juce::sendNotification);
        }

        if (pipeClient)
        {
            pipeClient->getWindowHandles();
            pipeClient->getMetricsValues();
        }

        statTable.setEnabled(true);
        repaint();
    }

    void addClient(juce::StringRef pipeName)
    {
        pipeClient = std::make_unique<PipeClient>(pipeName);
        pipeClient->onConnectionLost = [this]
            {
                pipeClient.reset();
            };

        pipeClient->onAllMetricsResponse = [this](juce::Direct2DMetricsHub::GetValuesResponse* response)
            {
                for (int i = 0; i < juce::Direct2DMetrics::numStats; ++i)
                {
                    statTable.setMetricValues(i, response->values[i]);
                }

                statTable.update();
                repaint();
            };

#if 0
        pipeClient->onGetWindowHandlesResponse = [this](juce::Direct2DMetricsHub::GetWindowHandlesResponse* response)
            {
                bool changed = false;
                for (auto const& windowHandle : response->windowHandles)
                {
                    if (std::find(windowHandles.begin(), windowHandles.end(), windowHandle) == windowHandles.end())
                    {
                        changed = true;
                        break;
                    }
                }

                if (!changed)
                    return;

                windowHandles.clear();
                for (auto const& windowHandle : response->windowHandles)
                {
                    windowHandles.emplace_back(windowHandle);
                }

                auto id = windowHandleComboBox.getSelectedId();
                windowHandleComboBox.clear(juce::dontSendNotification);

                for (size_t j = 0; j < response->maxNumWindowHandles; ++j)
                {
                    if (auto windowHandle = response->windowHandles[j])
                    {
                        auto topWindowHandle = GetTopWindow((HWND)windowHandle);
                        if (topWindowHandle == nullptr)
                            topWindowHandle = (HWND)windowHandle;

                        TCHAR windowName[64] = {};
                        GetWindowText(topWindowHandle, windowName, juce::numElementsInArray(windowName));

                        juce::String text{ windowName };
                        text << " - HWND 0x";
                        text << juce::String::toHexString((juce::pointer_sized_int)topWindowHandle);
                        windowHandleComboBox.addItem(text, (int)(juce::pointer_sized_int)windowHandle);
                    }
                }

                windowHandleComboBox.addItem("Images", -1);

                if (id == 0)
                {
                    id = (int)(juce::pointer_sized_int)response->windowHandles[0];
                    if (id == 0)
                        id = -1;
                }

                windowHandleComboBox.setSelectedId(id, juce::dontSendNotification);
            };
#endif
    }

#if 0
    void* getSelectedWindowHandle()
    {
        auto id = windowHandleComboBox.getSelectedId();
        return id < 0 ? nullptr : (void*)(juce::pointer_sized_int)id;
    }
#endif

private:
    std::unique_ptr<PipeClient> pipeClient;
    juce::StringArray pipeNames;
    std::vector<void*> windowHandles;
    StatTable statTable;
    juce::ComboBox pipeComboBox;
    //juce::ComboBox windowHandleComboBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MetricsContentComponent)
};
