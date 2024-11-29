#pragma once

class StatTable : public juce::TableListBoxModel, public juce::Component
{
private:
    juce::TableListBox table;

    struct AccumulatorInfo
    {
        juce::String name;
        int index;
        size_t lastCount = 0;

        juce::Direct2DMetricsHub::MetricValues values;
    };

    std::vector<AccumulatorInfo> accumulatorsInfo
    {
#define DIRECT2D_PAINT_STAT(name) AccumulatorInfo{ # name, juce::Direct2DMetrics::name, 0 },
#define DIRECT2D_LAST_PAINT_STAT(name) AccumulatorInfo{ # name, juce::Direct2DMetrics::name, 0 }
        DIRECT2D_PAINT_STAT_LIST
#undef DIRECT2D_PAINT_STAT
#undef DIRECT2D_LAST_PAINT_STAT
    };

    enum
    {
        nameColumn = 1,
        countColumn,
        totalColumn,
        averageColumn,
        standardDeviationColumn,
        percentColumn,
        maxColumn
    };

public:
    StatTable()
    {
        setOpaque(true);
        setAlwaysOnTop(true);

        table.setModel(this);
        addAndMakeVisible(table);

        auto& header = table.getHeader();
        header.addColumn({}, nameColumn, 200, 200, 200, juce::TableHeaderComponent::notResizableOrSortable);
        header.addColumn("#", countColumn, 80, 80, 80, juce::TableHeaderComponent::notResizableOrSortable);
        header.addColumn("Avg", averageColumn, 100, 100, 100, juce::TableHeaderComponent::notResizableOrSortable);
        header.addColumn("Std-dev", standardDeviationColumn, 100, 100, 100, juce::TableHeaderComponent::notResizableOrSortable);
        header.addColumn("%", percentColumn, 100, 100, 100, juce::TableHeaderComponent::notResizableOrSortable);
        header.addColumn("Max", maxColumn, 100, 100, 100, juce::TableHeaderComponent::notResizableOrSortable);

        table.addAndMakeVisible(resetStatsButton);

        setSize(100, 100);
    }

    ~StatTable() override
    {
        table.setModel(nullptr);
    }

    void setMetricValues(int index, juce::Direct2DMetricsHub::MetricValues values)
    {
        if (index >= accumulatorsInfo.size())
            return;

        accumulatorsInfo[index].values = values;

        if (index == 0 && values.total > 0.0)
        {
            totalFrameTime = values.total;
        }
    }

    int getDesiredHeight() noexcept
    {
        return table.getHeader().getHeight() + table.getRowHeight() * getNumRows() + table.getViewport()->getScrollBarThickness() + 10;
    }

    int getDesiredWidth()
    {
        return 682;
    }

    void resized() override
    {
        table.setBounds(getLocalBounds());
        resetStatsButton.setBounds(45, 2, 60, 25);
    }

    int getNumRows() override
    {
        return (int)accumulatorsInfo.size();
    }

    void paintRowBackground(juce::Graphics&, int /*rowNumber*/, int /*width*/, int /*height*/, bool /*rowIsSelected*/) override
    {
    }

    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour(juce::Colours::white.withAlpha(isEnabled() ? 1.0f : 0.5f));

        if (rowNumber >= accumulatorsInfo.size())
            return;

        auto const& info = accumulatorsInfo[rowNumber];

        switch (columnId)
        {
        case nameColumn:
            g.drawText(info.name, 0, 0, width - 5, height, juce::Justification::centredRight);
            break;

        case countColumn:
            g.drawText(juce::String{ info.values.count }, 5, 0, width, height, juce::Justification::centredLeft);
            break;

        case totalColumn:
            g.drawText(juce::String{ info.values.total }, 5, 0, width, height, juce::Justification::centredLeft);
            break;

        case averageColumn:
            g.drawText(juce::String{ info.values.average, 3 }, 5, 0, width, height, juce::Justification::centredLeft);
            break;

        case standardDeviationColumn:
            g.drawText(juce::String{ info.values.stdDev, 3 }, 5, 0, width, height, juce::Justification::centredLeft);
            break;

        case percentColumn:
        {
            if (rowNumber == juce::Direct2DMetrics::messageThreadPaintDuration || rowNumber == juce::Direct2DMetrics::frameInterval)
                break;

            if (totalFrameTime > 0.0)
            {
                double percent = 100.0 * info.values.total / totalFrameTime;
                if (0.0 <= percent && percent <= 100.0)
                {
                    g.drawText(juce::String{ percent, 1 } + " %", 5, 0, width, height, juce::Justification::centredLeft);
                }
            }
            break;
        }

        case maxColumn:
            g.drawText(juce::String{ info.values.maximum, 3 }, 5, 0, width, height, juce::Justification::centredLeft);
            break;
        }
    }

    void setAccumulatorInfo(juce::StringArray names)
    {
        accumulatorsInfo.clear();

        for (int index = 0; index < names.size(); ++index)
        {
            accumulatorsInfo.emplace_back(AccumulatorInfo{ names[index], index });
        }

        update();
    }

    void update()
    {
        setVisible(true);
        table.updateContent();
        table.repaint();
    }

    juce::TextButton resetStatsButton{ "Reset" };
    double totalFrameTime = 0.0;
};
