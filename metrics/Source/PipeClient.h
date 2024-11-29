#pragma once
#include "juce_graphics/native/juce_Direct2DMetrics_windows.h"

struct PipeClient : public juce::InterprocessConnection, public juce::ReferenceCountedObject
{
    using Ptr = juce::ReferenceCountedObjectPtr<PipeClient>;

    PipeClient(juce::StringRef pipeNameIn) :
        juce::InterprocessConnection(true, juce::Direct2DMetricsHub::magicNumber),
        pipeName(pipeNameIn)
    {
        connectToPipe(pipeName, -1);
    }

    ~PipeClient() override
    {
        disconnect();
    }

    void connectionMade() override
    {
    }

    void connectionLost() override
    {
        if (onConnectionLost) onConnectionLost();
    }

    void sendRequest(int requestType, void *windowHandle)
    {
        juce::MemoryBlock block{ sizeof(juce::Direct2DMetricsHub::Request) };
        auto message = (juce::Direct2DMetricsHub::Request*)block.getData();
        message->requestType = requestType;
        message->windowHandle = windowHandle;
        sendMessage(block);
    }

    void getMetricsValues(void* windowHandle)
    {
        sendRequest(juce::Direct2DMetricsHub::getValuesRequest, windowHandle);
    }

    void resetAllMetrics(void* windowHandle)
    {
        sendRequest(juce::Direct2DMetricsHub::resetValuesRequest, windowHandle);
    }

    void getWindowHandles()
    {
        sendRequest(juce::Direct2DMetricsHub::getWindowHandlesRequest, nullptr);
    }

    void messageReceived(const juce::MemoryBlock& message) override
    {
        lastUpdateTime = juce::Time::getCurrentTime();

        int responseType = *(int*)message.getData();

        switch (responseType)
        {
        case juce::Direct2DMetricsHub::getValuesRequest:
        {
            auto response = (juce::Direct2DMetricsHub::GetValuesResponse*)message.getData();

            if (onAllMetricsResponse) onAllMetricsResponse(response);

            break;
        }

        case juce::Direct2DMetricsHub::getWindowHandlesRequest:
        {
            auto response = (juce::Direct2DMetricsHub::GetWindowHandlesResponse*)message.getData();
            if (onGetWindowHandlesResponse) onGetWindowHandlesResponse(response);
            break;
        }
        }
    }

    juce::String const pipeName;
    std::function<void(juce::Direct2DMetricsHub::GetValuesResponse* response)> onAllMetricsResponse;
    std::function<void(juce::Direct2DMetricsHub::GetWindowHandlesResponse* response)> onGetWindowHandlesResponse;
    std::function<void()> onConnectionLost;

    juce::Time lastUpdateTime = juce::Time::getCurrentTime();
};
