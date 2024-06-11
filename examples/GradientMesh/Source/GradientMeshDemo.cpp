#include "GradientMeshDemo.h"
#include "Commander.h"

GradientMeshDemo::GradientMeshDemo()
    : displayComponent(*this),
    mesh(16, 16)
{
    setOpaque(true);

    //gradientMesh = std::make_unique<GradientMesh>();

    addAndMakeVisible(displayComponent);

    updater.addAnimator(fadeInAnimator);
    fadeInAnimator.start();
}

GradientMeshDemo::~GradientMeshDemo()
{
}

void GradientMeshDemo::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void GradientMeshDemo::paintOverChildren(Graphics& g)
{
    auto mousePos = getMouseXYRelative().toFloat();

    std::shared_ptr<mescal::GradientMesh::Vertex> selectedVertex;

    for (auto const& vertex : mesh.getVertices())
    {
        g.setColour(juce::Colours::white);
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

    g.setColour(juce::Colours::black);
    g.setFont({ getHeight() * 0.45f, juce::Font::bold });
    g.drawText("MESCAL", getLocalBounds(), juce::Justification::centred);
}

void GradientMeshDemo::resized()
{
    displayComponent.setBounds(getLocalBounds());
}

void GradientMeshDemo::createGradientMesh()
{
#if 0

    gradientMesh->clearPatches();

    juce::Point<float> topLeftCorner;
    float patchWidth = (float)getWidth() * 0.25;
    float patchHeight = (float)getHeight() * 0.25f;
    while (topLeftCorner.x <= (float)getWidth() && topLeftCorner.y <= (float)getHeight())
    {
        GradientMesh::Patch patch;

        auto setEdge = [&](GradientMesh::EdgePlacement edgePlacement, juce::Line<float> line)
            {
                auto& edge = patch.edges[(int)edgePlacement];
                edge.tail = line.getStart();

                auto colorX = line.getStartX() / (float)getWidth();
                auto normalizedY = line.getStart().y / (float)getHeight();

                auto hue = 0.5f * ((float)std::sin((colorX * 0.1f + normalizedY * 0.1f + displayComponent.timestamp * 0.0125) * juce::MathConstants<float>::twoPi) * 0.5f + 0.5f);
                auto saturation = std::cos(normalizedY * juce::MathConstants<float>::twoPi) * 0.5f + 0.5f;

                edge.tailColor = mescal::GradientMesh::Color128::fromHSV(hue,
                    saturation,
                    displayComponent.gradientOpacity,
                    displayComponent.gradientOpacity);

                float offset = 50.0f;
                auto angle = line.getAngle();
                if (edge.tail.x < patchWidth || edge.tail.x >(float)getWidth() - patchWidth ||
                    edge.tail.y < patchHeight || edge.tail.y >(float)getHeight() - patchHeight)
                {
                    offset = 0.0f;
                }
                angle += juce::MathConstants<float>::halfPi;
                edge.controlPoints =
                {
                    line.getPointAlongLineProportionally(0.25f).getPointOnCircumference(offset, angle + juce::MathConstants<float>::halfPi),
                    line.getPointAlongLineProportionally(0.75f).getPointOnCircumference(offset, angle - juce::MathConstants<float>::halfPi)
                };
            };

        std::array<juce::Point<float>, 4> patchCorners
        {
            topLeftCorner.translated(patchWidth, patchHeight),
            topLeftCorner.translated(patchWidth, 0.0f),
            topLeftCorner,
            topLeftCorner.translated(0.0f, patchHeight)
        };
        size_t patchCornerIndex = 0;
        juce::Line<float> line{ patchCorners[2], patchCorners[3] };
        for (auto edgePlacement : { GradientMesh::EdgePlacement::left, GradientMesh::EdgePlacement::bottom, GradientMesh::EdgePlacement::right, GradientMesh::EdgePlacement::top })
        {
            setEdge(edgePlacement, line);

            line = juce::Line<float>{ line.getEnd(), patchCorners[patchCornerIndex++] };
        }

        topLeftCorner += { patchWidth, 0.0f };
        if (topLeftCorner.x >= (float)getWidth())
        {
            topLeftCorner.x = 0.0f;
            topLeftCorner += { 0.0f, patchHeight};
        }

        gradientMesh->addPatch(patch);
    }
#endif
}

GradientMeshDemo::DisplayComponent::DisplayComponent(GradientMeshDemo& owner_) :
    owner(owner_)
{
    setInterceptsMouseClicks(false, false);
}

void GradientMeshDemo::DisplayComponent::paint(juce::Graphics& g)
{
    auto mousePos = getLocalBounds().reduced(200).getConstrainedPoint(getMouseXYRelative()).toFloat();
    auto center = getLocalBounds().getCentre().toFloat();
    float columnWidth = (float)getWidth() / (float)(owner.mesh.getNumColumns() - 1);
    float rowHeight = (float)getHeight() / (float)(owner.mesh.getNumRows() - 3);
    owner.mesh.configureVertices([=](int row, int column, std::shared_ptr<mescal::GradientMesh::Vertex> vertex)
        {
            float x = column * columnWidth;
            float offset = std::sin(x / (float)getWidth() * juce::MathConstants<float>::twoPi * 2.0f) * 25.0f;
            float y = row * rowHeight + offset;
            juce::Point<float> p{ x, y };
            auto distance = p.getDistanceFrom(mousePos);
            auto angle = mousePos.getAngleToPoint(p);
            distance *= std::pow(1.001f, distance);
            //distance = juce::roundToInt(distance / 100.0f) * 100.0f;
            vertex->position = mousePos.getPointOnCircumference(distance, angle);

#if 0
            float x = column * columnWidth;
            float y = row * rowHeight + rowHeight * 0.4f * std::sin(juce::MathConstants<float>::twoPi * (float)column / (float)mesh.getNumColumns());
            vertex->position = { x, y };
#endif
            angle = angle < 0.0f ? (juce::MathConstants<float>::twoPi + angle) : angle;

            float phase = (0.25f * (x / (float)getWidth()) * juce::MathConstants<float>::twoPi) + 0.0125f * timestamp * juce::MathConstants<float>::twoPi;
            if (row > owner.mesh.getNumRows() / 2)
            {
                phase += juce::MathConstants<float>::pi;
            }

            auto hue = std::sin(phase) * 0.5f + 0.5f;
            vertex->color = juce::Colour::fromHSV(hue, y / (float)getHeight(), 1.0f, gradientOpacity);
        });

    if (meshImage.isNull() || meshImage.getWidth() != getWidth() || meshImage.getHeight() != getHeight())
    {
        meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    }

    owner.mesh.draw(meshImage, {});

#if 0
    if (spriteAtlas.isNull())
    {
        spriteAtlas = juce::Image{ juce::Image::ARGB, 16, 16, true };
        {
            juce::Graphics g{ spriteAtlas };
            g.setColour(juce::Colours::white);
            g.fillEllipse(0.0f, 0.0f, 16.0f, 16.0f);
        }
        owner.spriteBatch.setAtlas(spriteAtlas);
    }

    std::vector<mescal::Sprite> sprites{ 16 };

    float x = 32.0f;
    for (int i = 0; i < 16; ++i)
    {
        sprites[i] = mescal::Sprite
        {
            { x, x, 16.0f, 16.0f },
            { 0, 0, 16, 16 }
        };
        x += 40.0f;
    }
#endif

    //owner.spriteBatch.draw(meshImage, sprites);

    g.drawImageAt(meshImage, 0, 0);
}


