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

    void sendRequest(int requestType)
    {
        juce::MemoryBlock block{ sizeof(int) };
        auto message = (int*)block.getData();
        *message = requestType;
        sendMessage(block);
    }

    void getMetricsValues()
    {
        sendRequest(juce::Direct2DMetricsHub::getValuesRequest);
    }

    void resetAllMetrics()
    {
        sendRequest(juce::Direct2DMetricsHub::resetValuesRequest);
    }

    void getMaximumTextureMemory()
    {
        sendRequest(juce::Direct2DMetricsHub::getMaximumTextureMemoryRequest);
    }

    void setMaximumTextureMemory(uint64_t bytes)
    {
        juce::MemoryBlock block{ sizeof(juce::Direct2DMetricsHub::SetMaximumTextureMemoryRequest) };
        auto message = (juce::Direct2DMetricsHub::SetMaximumTextureMemoryRequest*)block.getData();
        message->requestType = juce::Direct2DMetricsHub::setMaximumTextureMemoryRequest;
        message->maximumTextureMemory = bytes;
        sendMessage(block);
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

        case juce::Direct2DMetricsHub::getMaximumTextureMemoryRequest:
        {
            auto response = (juce::Direct2DMetricsHub::GetMaximumTextureMemoryResponse*)message.getData();

            if (onMaxTextureMemoryResponse) onMaxTextureMemoryResponse(response);

            break;
        }

        }
    }

    juce::String const pipeName;
    std::function<void(juce::Direct2DMetricsHub::GetValuesResponse* response)> onAllMetricsResponse;
    std::function<void(juce::Direct2DMetricsHub::GetMaximumTextureMemoryResponse* response)> onMaxTextureMemoryResponse;
    std::function<void()> onConnectionLost;

    juce::Time lastUpdateTime = juce::Time::getCurrentTime();
};
