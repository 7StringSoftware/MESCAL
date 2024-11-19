#pragma once
#include "juce_graphics/native/juce_Direct2DMetrics_windows.h"

struct PipeClient : public juce::InterprocessConnection, public juce::ReferenceCountedObject
{
    using Ptr = juce::ReferenceCountedObjectPtr<PipeClient>;

    PipeClient(uint32_t processID_) :
        juce::InterprocessConnection(true, juce::Direct2DMetricsHub::magicNumber),
        processID(processID_),
        pipeName("JUCEDirect2DMetricsHub:" + juce::String::toHexString((juce::pointer_sized_int)processID_))
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

        }
    }

    uint32_t const processID;
    juce::String const pipeName;
    std::function<void(juce::Direct2DMetricsHub::GetValuesResponse* response)> onAllMetricsResponse;
    std::function<void()> onConnectionLost;

    static constexpr struct FlagNames
    {
        static constexpr std::array<std::string_view, 5> names
        {
           "effectsEnabled",
           "fillRectListEnabled",
           "drawImageEnabled",
           "softwareImageBackupEnabled",
           "gradientEnabled"
        };

        juce::String getName(size_t i) const
        {
            return juce::String::createStringFromData(names[i].data(), (int)names[i].size());
        }

    } flagNames;

    juce::Time lastUpdateTime = juce::Time::getCurrentTime();
};
