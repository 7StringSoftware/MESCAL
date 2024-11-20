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
                for (auto client : processClients)
                {
                    client->resetAllMetrics();
                }
            };
        addChildComponent(statTable);
    }

    ~MetricsContentComponent() override
    {
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour{ 0xff111111 });

        g.setColour(juce::Colours::white);
        if (statTable.isEnabled() && statTable.isVisible())
        {
            TCHAR windowName[64] = {};
            GetWindowText((HWND)windowHandle, windowName, 64);

            juce::String text{ windowName };
            text << " - HWND 0x" << juce::String::toHexString((juce::pointer_sized_int)windowHandle);
            g.drawText(text,
                15, 0, getWidth() - 30, 25,
                juce::Justification::centredRight);
        }
        else
        {
            g.drawText("No metrics available", getLocalBounds(), juce::Justification::centred);
        }
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
    }

    void timerCallback() override
    {
        auto now = juce::Time::getCurrentTime();

        for (int i = processClients.size() - 1; i >= 0; --i)
        {
            auto elapsed = now - processClients[i]->lastUpdateTime;
            if (elapsed.inMilliseconds() >= 1000)
            {
                processClients.remove(i);
            }
        }

        findJUCEProcesses();

        auto foregroundWindow = GetForegroundWindow();
        DWORD foregroundProcessID = 0;

        if (foregroundWindow)
        {
            GetWindowThreadProcessId(foregroundWindow, &foregroundProcessID);

            for (auto processClient : processClients)
            {
                if (processClient->processID == foregroundProcessID)
                {
                    lastClient = processClient;
                    processClient->getMetricsValues();
                    statTable.setEnabled(true);
                    return;
                }
            }
        }

        if (lastClient && processClients.contains(lastClient))
        {
            lastClient->getMetricsValues();
            statTable.setEnabled(true);
            return;
        }

        statTable.setEnabled(false);
        repaint();
    }

    static BOOL enumWindowsCallback(HWND hwnd, LPARAM lParam)
    {
        auto that = (MetricsContentComponent*)lParam;

        static char juceWindowName[] = "JUCEWindow";
        char windowName[64] = {};

        GetWindowTextA(hwnd, windowName, 64);
        if (0 == memcmp(windowName, juceWindowName, sizeof(juceWindowName) - sizeof(TCHAR)))
        {
            DWORD processID = 0;
            if (GetWindowThreadProcessId(hwnd, &processID))
            {
                if (processID == GetCurrentProcessId()) return TRUE;

                that->addClient(processID);
            }
        }

        return TRUE;
    }

    void findJUCEProcesses()
    {
        EnumWindows(enumWindowsCallback, (LPARAM)this);
    }

    void addClient(uint32_t processID)
    {
        for (auto client : processClients)
        {
            if (client->processID == processID)
            {
                return;
            }
        }

        auto client = new PipeClient{ processID };
        client->onConnectionLost = [this, client]
            {
                processClients.removeObject(client);
            };

        client->onAllMetricsResponse = [this](juce::Direct2DMetricsHub::GetValuesResponse* response)
            {
                for (int i = 0; i < juce::Direct2DMetrics::numStats; ++i)
                {
                    statTable.setMetricValues(i, response->values[i]);
                }

                statTable.update();

                if (windowHandle != response->windowHandle)
                {
                    repaint();
                }
                windowHandle = response->windowHandle;
            };

        processClients.add(client);
    }

private:
    juce::ReferenceCountedArray<PipeClient> processClients;
    PipeClient* lastClient = nullptr;
    StatTable statTable;
    void* windowHandle = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MetricsContentComponent)
};
