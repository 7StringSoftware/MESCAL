#include "GradientMeshDemo.h"
#include "Commander.h"

GradientMeshDemo::GradientMeshDemo() :
    mesh(16, 16)
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
        g.setColour(juce::Colours::black);
        g.setFont({ 400.0f, juce::Font::bold });
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

    g.reduceClipRegion(getLocalBounds().reduced(juce::roundToInt(maskBoundarySize)));

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
    glowEffect.setGlowProperties(50.0f, juce::Colours::white);
    glowEffect.applyEffect(logoImage, g, 1.0f, gradientOpacity);
}

void GradientMeshDemo::resized()
{
    int w = 200;
    int h = 30;
    showMeshToggle.setBounds(getWidth() - w - 10, getHeight() - h - 10, w, h);
}

void GradientMeshDemo::paintMesh(juce::Graphics& g)
{
    if (meshImage.isNull() || meshImage.getWidth() != getWidth() || meshImage.getHeight() != getHeight())
    {
        meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    }

    auto mousePos = getLocalBounds().reduced(200).getConstrainedPoint(getMouseXYRelative()).toFloat();
    auto center = getLocalBounds().getCentre().toFloat();
    float columnWidth = (float)getWidth() / (float)(mesh.getNumColumns() - 1);
    float rowHeight = (float)getHeight() / (float)(mesh.getNumRows() - 3);
    mesh.configureVertices([=](int row, int column, std::shared_ptr<mescal::GradientMesh::Vertex> vertex)
        {
            float x = column * columnWidth;
            float offset = std::sin(x / (float)getWidth() * juce::MathConstants<float>::twoPi * 2.0f) * 25.0f;
            float y = row * rowHeight + offset;
            juce::Point<float> p{ x, y };
            auto distance = p.getDistanceFrom(mousePos);

            auto angle = mousePos.getAngleToPoint(p);
            distance *= std::pow(1.001f, distance);
            vertex->position = mousePos.getPointOnCircumference(distance, angle);

            float phase = (0.25f * (x / (float)getWidth()) * juce::MathConstants<float>::twoPi) + 0.0125f * timestamp * juce::MathConstants<float>::twoPi;
            if (row > mesh.getNumRows() / 2)
            {
                phase += juce::MathConstants<float>::pi;
            }

            auto hue = std::sin(phase) * 0.5f + 0.5f;
            vertex->color = juce::Colour::fromHSV(hue, y / (float)getHeight(), 1.0f, gradientOpacity);
        });

    mesh.draw(meshImage, {});
    g.drawImageAt(meshImage, 0, 0);

    if (showMeshToggle.getToggleState())
    {
        paintMeshWireframe(g);
    }
}

void GradientMeshDemo::paintMeshWireframe(juce::Graphics& g)
{
    auto mousePos = getMouseXYRelative().toFloat();

    std::shared_ptr<mescal::GradientMesh::Vertex> selectedVertex;

    for (auto const& vertex : mesh.getVertices())
    {
        g.setColour(vertex->color.contrasting());
        g.fillEllipse(juce::Rectangle<float>{ 22.0f, 22.0f }.withCentre(vertex->position));
        g.setColour(vertex->color);
        g.fillEllipse(juce::Rectangle<float>{ 16.0f, 16.0f }.withCentre(vertex->position));

        if (vertex->position.getDistanceFrom(mousePos) < 50.0f)
        {
            selectedVertex = vertex;
        }
    }

    for (auto const& halfedge : mesh.getHalfedges())
    {
        auto tail = halfedge->tail.lock();
        auto head = halfedge->head.lock();
        if (tail && head)
        {
            auto color = juce::Colours::white.withAlpha(0.5f);
            if (tail.get() == selectedVertex.get() || head.get() == selectedVertex.get())
                color = juce::Colours::white;

            g.setColour(color);
            auto angle = tail->position.getAngleToPoint(head->position) + juce::MathConstants<float>::halfPi;
            juce::Line<float> line{ tail->position.getPointOnCircumference(3.0f, angle), head->position.getPointOnCircumference(3.f, angle) };
            g.drawArrow(line.withShortenedStart(5.0f).withShortenedEnd(5.0f), 2.0f, 12.0f, 12.0f);
        }
    }

    if (selectedVertex)
    {
        if (auto halfedge = selectedVertex->halfedge.lock())
        {
            g.setColour(juce::Colours::white);

            auto tail = halfedge->tail.lock();
            auto head = halfedge->head.lock();
            if (tail && head)
                g.drawArrow({ tail->position, head->position }, 4.0f, 14.0f, 12.0f);
        }
    }
}

void GradientMeshDemo::paintSnowfall(juce::Graphics& g)
{

}
