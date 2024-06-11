#include "GradientMeshDemo.h"
#include "Commander.h"

GradientMeshDemo::GradientMeshDemo()
    : displayComponent(*this),
    mesh(16, 16)
{
    setOpaque(true);

    //gradientMesh = std::make_unique<GradientMesh>();

    displayComponent.spriteAtlas = juce::Image{ juce::Image::ARGB, 1536, 768, true };
    {
        juce::Graphics g{ displayComponent.spriteAtlas };

        g.setColour(juce::Colours::black);
        g.setFont({ 400.0f, juce::Font::bold });
        //g.drawRect(displayComponent.spriteAtlas.getBounds(), 1.0f);
        g.setColour(juce::Colours::blue);
        g.fillEllipse(displayComponent.spriteAtlas.getBounds().toFloat() * 0.5f);
        g.setColour(juce::Colours::red);
        g.fillEllipse((displayComponent.spriteAtlas.getBounds().toFloat() * 0.5f).translated(displayComponent.spriteAtlas.getWidth() * 0.5f, 0.0f));
        //g.drawText("MESCAL", displayComponent.spriteAtlas.getBounds(), juce::Justification::centred);
    }
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
#if 0
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
#endif

}

void GradientMeshDemo::resized()
{
    displayComponent.setBounds(getLocalBounds());
}

void GradientMeshDemo::mouseDown(juce::MouseEvent const& event)
{
    updater.addAnimator(spriteAnimator);
    spriteAnimator.start();
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
            vertex->position = mousePos.getPointOnCircumference(distance, angle);

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

    if (!owner.spriteAnimator.isComplete())
    {
        owner.spriteBatch.setAtlas(spriteAtlas);
        std::vector<mescal::Sprite> sprites;
        int spriteWidth = spriteAtlas.getWidth() / 4;
        int spriteHeight = spriteAtlas.getHeight() / 4;
        auto center = getLocalBounds().getCentre().toFloat();
        juce::Point<float> offset{ (getWidth() - spriteAtlas.getWidth()) * 0.5f, (getHeight() - spriteAtlas.getHeight()) * 0.5f };
        for (int x = 0; x < spriteAtlas.getWidth(); x += spriteWidth)
        {
            for (int y = 0; y < spriteAtlas.getHeight(); y += spriteHeight)
            {
                juce::Rectangle<int> sourceR{ x, y, spriteWidth, spriteHeight };
                auto destR = sourceR.toFloat() + offset;
                auto angle = center.getAngleToPoint(destR.getCentre());
                destR.setCentre(center.getPointOnCircumference(distance, angle));
                    sprites.push_back({ destR, sourceR });
            }
        }

        owner.spriteBatch.draw(meshImage, sprites);
    }

    g.drawImageAt(meshImage, 0, 0);

    if (owner.spriteAnimator.isComplete())
    {
        g.drawImageAt(spriteAtlas, (getWidth() - spriteAtlas.getWidth()) / 2, (getHeight() - spriteAtlas.getHeight()) / 2);
    }
}


