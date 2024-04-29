#include <JuceHeader.h>
#include "Mesher.h"

static juce::String print(juce::Path::Iterator const& it)
{
    juce::String line;

    if (it.elementType == Path::Iterator::startNewSubPath)
    {
        line << "startNewSubPath " << it.x1 << ", " << it.y1;
    }
    else if (it.elementType == Path::Iterator::lineTo)
    {
        line << "lineTo " << it.x1 << ", " << it.y1;
    }
    else if (it.elementType == Path::Iterator::quadraticTo)
    {
        line << "quadraticTo " << it.x1 << ", " << it.y1 << " " << it.x2 << ", " << it.y2;
    }
    else if (it.elementType == Path::Iterator::cubicTo)
    {
        line << "cubicTo " << it.x1 << ", " << it.y1 << " " << it.x2 << ", " << it.y2 << " " << it.x3 << ", " << it.y3;
    }
    else if (it.elementType == Path::Iterator::closePath)
    {
        line << "closePath";
    }   

    return line;
}

Mesher::Mesher(Path&& p) :
    path(p)
{
    juce::Path::Iterator it{ p };

    auto bounds = p.getBounds();

    float lastX = bounds.getX(), lastY = bounds.getY();

    while (it.next())
    {
        DBG(print(it));

        if (it.elementType == Path::Iterator::startNewSubPath)
        {
            lastX = it.x1;
            lastY = it.y1;
            continue;
        }

        float x = 0.0f, y = 0.0f;
        if (it.elementType == Path::Iterator::lineTo)
        {
            x = it.x1;
            y = it.y1;
        }
        else if (it.elementType == Path::Iterator::quadraticTo)
        {
            x = it.x2;
            y = it.y2;
        }
        else if (it.elementType == Path::Iterator::cubicTo)
        {
            x = it.x3;
            y = it.y3;
        }
        else
        {
            continue;
        }

        DBG("    x : " << x << " y: " << y);

        rowHeights.add(x - lastX);
        columnWidths.add(y - lastY);
        lastX = x;
        lastY = y;
    }
}

