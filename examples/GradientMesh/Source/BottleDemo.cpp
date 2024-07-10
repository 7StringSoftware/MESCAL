#include "BottleDemo.h"

BottleDemo::BottleDemo()
{
    auto drawable = juce::Drawable::createFromImageData(BinaryData::Wine_bottle_svg, BinaryData::Wine_bottle_svgSize);
    bottlePath = drawable->getOutlineAsPath();
}

BottleDemo::~BottleDemo()
{
}

void BottleDemo::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    g.setColour(juce::Colours::black);
    g.fillPath(transformedBottlePath);
    g.reduceClipRegion(transformedBottlePath);

    mesh->draw(meshImage, {}, juce::Colours::black);
    g.drawImageAt(meshImage, 0, 0);
}

void BottleDemo::resized()
{
    auto pathTransform = juce::RectanglePlacement{}.getTransformToFit(originalBottlePath.getBounds(), getLocalBounds().toFloat());
    transformedBottlePath = originalBottlePath;
    transformedBottlePath.applyTransform(pathTransform);

    mesh = std::make_unique<mescal::GradientMesh>(8, 8);
    std::array<juce::Colour, 8> colors
    {
        juce::Colours::darkgreen,
        juce::Colours::green,
        juce::Colours::limegreen,
        juce::Colours::green,
        juce::Colours::green,
        juce::Colours::darkgreen,
        juce::Colours::darkgreen,
        juce::Colours::darkgrey
    };
    std::array<float, 8> shade
    {
        0.0f, 0.0f, 0.2f, 0.4f, 1.0f, 1.0f, 1.0f, 0.0f
    };
    auto bounds = transformedBottlePath.getBounds();
    float xStep = bounds.getWidth() / (float)(mesh->getNumColumns() - 1);
    float yStep = bounds.getHeight() / (float)(mesh->getNumRows() - 1);
    mesh->configureVertices([=](int row, int column, std::shared_ptr<mescal::GradientMesh::Vertex> vertex)
        {
            juce::Point<float> p{ (float)column * xStep, (float)row * yStep };
            vertex->position = p;

            vertex->setColor(colors[column].withAlpha(shade[row]));
            DBG(p.toString());
        });

    meshImage = juce::Image{ juce::Image::ARGB, (int)bounds.getWidth(), (int)bounds.getHeight(), true };
}
