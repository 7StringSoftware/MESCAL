#include "GradientMeshDemo.h"
#include "Commander.h"

GradientMeshDemo::GradientMeshDemo()
{
    setOpaque(true);

    addChildComponent(showMeshToggle);

    backgroundCombo.addItem("Gradient mesh background", gradientMeshBackground);
    backgroundCombo.addItem("Snowfall", snowfallBackground);
    backgroundCombo.setSelectedId(gradientMeshBackground, juce::dontSendNotification);
    addChildComponent(backgroundCombo);

    logoImage = juce::Image{ juce::Image::ARGB, 1536, 768, true };
    {
        juce::Graphics g{ logoImage };
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions{ 400.0f, juce::Font::bold});
        g.drawText("MESCAL", logoImage.getBounds(), juce::Justification::centred);
    }

    updater.addAnimator(animator);
    animator.start();
}

GradientMeshDemo::~GradientMeshDemo()
{
}

void GradientMeshDemo::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    //g.reduceClipRegion(getLocalBounds().reduced(juce::roundToInt(maskBoundarySize)));

    switch (backgroundCombo.getSelectedId())
    {
    case gradientMeshBackground:
        paintMesh(g);
        break;

    case snowfallBackground:
        paintSnowfall(g);
        break;
    }

    juce::Point<float> offset
    {
        ((float)getWidth() - logoImage.getWidth()) * 0.5f,
        ((float)getHeight() - logoImage.getHeight()) * 0.5f
    };
    g.addTransform(juce::AffineTransform::translation(offset));
    glowEffect.setGlowProperties(50.0f, juce::Colours::black);
    glowEffect.applyEffect(logoImage, g, 1.0f, gradientOpacity);
}

void GradientMeshDemo::resized()
{
    int w = 200;
    int h = 30;
    showMeshToggle.setBounds(getWidth() - w - 10, getHeight() - h - 10, w, h);
    mesh = std::make_unique<mescal::MeshGradient>(16, 16, getLocalBounds().toFloat());
}

void GradientMeshDemo::paintMesh(juce::Graphics& g)
{
    if (meshImage.isNull() || meshImage.getWidth() != getWidth() || meshImage.getHeight() != getHeight())
    {
        meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    }

    auto mousePos = getLocalBounds().reduced(200).getConstrainedPoint(getMouseXYRelative()).toFloat();
    auto center = getLocalBounds().getCentre().toFloat();
    float columnWidth = (float)getWidth() / (float)(mesh->getNumColumns());
    float rowHeight = (float)getHeight() / (float)(mesh->getNumRows());
    float xScale = 1.0f / (float)getWidth();
    float yScale = 1.0f / (float)getHeight();
    auto normalizedPhase = 0.025f * timestamp;

    for (auto& patch : mesh->getPatches())
    {
        float x = patch->column * columnWidth;
        float offset = std::sin(x * xScale * juce::MathConstants<float>::twoPi * 2.0f) * 25.0f;
        float y = patch->row * rowHeight + offset;

        float left = 0.0f, right = 0.0f, top = 0.0f, bottom = 0.0f;
        {
            left = std::sin((normalizedPhase + x * xScale) * juce::MathConstants<float>::twoPi * 0.25f) * 0.5f + 0.5f;
            right = std::sin((normalizedPhase + (x + columnWidth) * xScale) * juce::MathConstants<float>::twoPi * 0.25f) * 0.5f + 0.5f;

            top = std::sin((4.0f * normalizedPhase + y * yScale) * juce::MathConstants<float>::twoPi * 0.5f) * 0.25f + 0.75f;
            bottom = std::sin((4.0f * normalizedPhase + (y + rowHeight) * yScale) * juce::MathConstants<float>::twoPi * 0.5f) * 0.25f + 0.75f;
        }

        //top *= top;

        patch->setColor(mescal::MeshGradient::CornerPlacement::topLeft, juce::Colour::fromHSV(left, top, top, gradientOpacity));
        patch->setColor(mescal::MeshGradient::CornerPlacement::topRight, juce::Colour::fromHSV(right, top, top, gradientOpacity));

        //bottom *= bottom;

        patch->setColor(mescal::MeshGradient::CornerPlacement::bottomLeft, juce::Colour::fromHSV(left, bottom, bottom, gradientOpacity));
        patch->setColor(mescal::MeshGradient::CornerPlacement::bottomRight, juce::Colour::fromHSV(right, bottom, bottom, gradientOpacity));
    }

    mesh->draw(meshImage, {});
    g.setColour(juce::Colours::black);
    g.drawImageAt(meshImage, 0, 0);

    if (showMeshToggle.getToggleState())
    {
        //paintMeshWireframe(g);
    }
}

void GradientMeshDemo::paintMeshWireframe(juce::Graphics& g)
{
#if 0
    auto mousePos = getMouseXYRelative().toFloat();

    std::shared_ptr<mescal::GradientMesh::Vertex> selectedVertex;

    for (auto const& vertex : mesh->getVertices())
    {
        g.setColour(juce::Colours::black);
        g.fillEllipse(juce::Rectangle<float>{ 16.0f, 16.0f }.withCentre(vertex->position));

        if (vertex->position.getDistanceFrom(mousePos) < 50.0f)
        {
            selectedVertex = vertex;
        }
    }

    for (int row = 0; row < mesh->getNumRows() - 1; ++row)
    {
        for (int column = 0; column < mesh->getNumColumns() - 1; ++column)
        {
            auto vertex = mesh->getVertex(row, column);
            if (!vertex)
                continue;

            if (auto vertexRight = mesh->getVertex(row, column + 1))
            {
                g.drawLine({ vertex->position, vertexRight->position }, 2.0f);
            }

            if (auto vertexDown = mesh->getVertex(row + 1, column))
            {
                g.drawLine({ vertex->position, vertexDown->position }, 2.0f);
            }
        }
    }
#endif
}

void GradientMeshDemo::paintSnowfall(juce::Graphics& g)
{

}
