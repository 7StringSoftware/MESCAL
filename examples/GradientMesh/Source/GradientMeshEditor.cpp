#include "GradientMeshEditor.h"

GradientMeshEditor::GradientMeshEditor()
{
    for (auto& c : vertexComponents)
    {
        c.setSize(30, 30);
        addChildComponent(c);
        c.onChange = [this](VertexComponent& vertexComponent)
            {
                if (auto vertex = vertexComponent.vertex.lock())
                {
                    vertex->position = vertexComponent.getBounds().toFloat().getCentre();
                    buildPatches();
                    repaint();
                }
            };
    }

    for (auto& c : bezierControlComponents)
    {
        c.setSize(20, 20);
        addChildComponent(c);
        c.onChange = [this](BezierControlComponent& controlComponent)
            {
                if (auto vertex = controlComponent.vertex.lock())
                {
                    vertex->bezier.setControlPoint(controlComponent.placement, controlComponent.getBounds().toFloat().getCentre());
                    buildPatches();
                    repaint();
                }
            };
    }

    rowCountLabel.attachToComponent(&rowCountSlider, true);
    addAndMakeVisible(rowCountLabel);
    addAndMakeVisible(rowCountSlider);
    rowCountSlider.setRange(juce::Range<double>{ 2.0, 20.0 }, 1.0);
    rowCountSlider.onValueChange = [this]()
        {
            mesh = nullptr;
            selectPatch(nullptr);
            createMesh();
            createComponents();
            buildPatches();
            resized();
        };
    rowCountSlider.setValue(4.0, juce::dontSendNotification);

    columnCountLabel.attachToComponent(&columnCountSlider, true);
    addAndMakeVisible(columnCountLabel);
    addAndMakeVisible(columnCountSlider);
    columnCountSlider.setRange(juce::Range<double>{ 2.0, 20.0 }, 1.0);
    columnCountSlider.onValueChange = rowCountSlider.onValueChange;
    columnCountSlider.setValue(4.0, juce::dontSendNotification);
}

void GradientMeshEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    paintMesh(g);
}

void GradientMeshEditor::resized()
{
    createMesh();
    createComponents();
    buildPatches();

    for (auto& vertexComponent : vertexComponents)
    {
        if (auto lock = vertexComponent.vertex.lock())
        {
            auto center = lock->position.toInt();
            vertexComponent.setCentrePosition({ center.x, center.y });
        }
    }

    for (auto& controlComponent : bezierControlComponents)
    {
        if (auto lock = controlComponent.vertex.lock())
        {
            auto controlPoint = lock->bezier.getControlPoint(controlComponent.placement);
            if (controlPoint.has_value())
                controlComponent.setCentrePosition(controlPoint->toInt());
        }
    }

    for (auto& patchComponent : patchComponents)
    {
        patchComponent->setBounds(getLocalBounds());
    }

    int sliderWidth = 120;
    rowCountSlider.setBounds(getWidth() / 2 - sliderWidth - 10, 20, sliderWidth, 30);
    columnCountSlider.setBounds(getWidth() / 2 + 100, 20, sliderWidth, 30);
}

void GradientMeshEditor::createMesh()
{
    if (getLocalBounds().isEmpty())
        return;

    if (!mesh)
    {
        int numRows = (int)rowCountSlider.getValue();
        int numColumns = (int)columnCountSlider.getValue();
        mesh = std::make_unique<mescal::GradientMesh>(numRows, numColumns, getLocalBounds().toFloat().reduced(200));

        std::array<mescal::GradientMesh::Placement, 4> placements
        {
            mescal::GradientMesh::Placement::top,
            mescal::GradientMesh::Placement::left,
            mescal::GradientMesh::Placement::bottom,
            mescal::GradientMesh::Placement::right
        };

        juce::Point<float> center{ (float)numRows * 0.5f, (float)numColumns * 0.5f };
        for (auto& vertex : mesh->getVertices())
        {
            juce::Point<float> point{ (float)vertex->row, (float)vertex->column };

            float red = (float)vertex->row / ((float)numRows - 1);
            float green = (float)vertex->column / ((float)numColumns - 1);
            float blue = center.getDistanceFrom(point) / center.getDistanceFrom({ 0.0f, 0.0f });
            vertex->color = mescal::Color128{ red, green, blue, 1.0f };

            for (auto placement : placements)
            {
                if (auto adjacentVertex = vertex->getAdjacentVertex(placement))
                {
                    juce::Line<float> line{ vertex->position, adjacentVertex->position };

                    auto controlPoint = line.getPointAlongLineProportionally(0.33f)
                        .getPointOnCircumference(line.getLength() * 0.1f, line.getAngle() + juce::MathConstants<float>::halfPi);

                    vertex->bezier.setControlPoint(placement, controlPoint);
                }
            }
        }
    }
}

void GradientMeshEditor::createComponents()
{
    if (mesh)
    {
        int numRows = (int)rowCountSlider.getValue();
        int numColumns = (int)columnCountSlider.getValue();

        if (patchComponents.size() == (numRows - 1) * (numColumns - 1))
        {
            return;
        }

        patchComponents.clear();

        for (int row = 0; row < numRows - 1; ++row)
        {
            for (int column = 0; column < numColumns - 1; ++column)
            {
                auto topLeftVertex = mesh->getVertex(row, column);
                auto patchComponent = std::make_unique<PatchComponent>(row, column);
                addAndMakeVisible(patchComponent.get());
                patchComponent->toBack();
                patchComponent->onSelect = [this](PatchComponent* patchComponent)
                    {
                        selectPatch(patchComponent);
                    };
                patchComponents.push_back(std::move(patchComponent));
            }
        }
    }
}

void GradientMeshEditor::buildPatches()
{
    for (auto& patch : patchComponents)
    {
        auto topLeftVertex = mesh->getVertex(patch->row, patch->column);
        patch->build(topLeftVertex, *this);
    }
}

void GradientMeshEditor::paintMesh(juce::Graphics& g)
{
    if (meshImage.isNull() || meshImage.getWidth() != getWidth() || meshImage.getHeight() != getHeight())
        meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);

    mesh->draw(meshImage, {}, juce::Colours::transparentBlack);
    g.drawImageAt(meshImage, 0, 0);
}

void GradientMeshEditor::selectPatch(PatchComponent* patch)
{
    for (auto& patchComponent : patchComponents)
    {
        patchComponent->selected = false;
        patchComponent->toBack();
    }

    for (auto& vertexComponent : vertexComponents)
    {
        vertexComponent.setVisible(false);
    }

    for (auto& bezierControlComponent : bezierControlComponents)
    {
        bezierControlComponent.setVisible(false);
    }

    if (!patch)
        return;

    patch->toFront(true);
    patch->selected = true;

    auto vertexComponentIterator = vertexComponents.begin();
    auto bezierControlComponentIterator = bezierControlComponents.begin();

    auto tail = mesh->getVertex(patch->row, patch->column);

    for (auto placement :
        {
            mescal::GradientMesh::Placement::bottom,
            mescal::GradientMesh::Placement::right,
            mescal::GradientMesh::Placement::top,
            mescal::GradientMesh::Placement::left
        })
    {
        if (!tail)
            return;

        (*vertexComponentIterator).vertex = tail;
        (*vertexComponentIterator).setCentrePosition(tail->position.toInt());
        (*vertexComponentIterator).setVisible(true);
        (*vertexComponentIterator).toFront(true);
        vertexComponentIterator++;

        {
            auto bezierControlPoint = tail->bezier.getControlPoint(placement);
            if (bezierControlPoint.has_value())
            {
                (*bezierControlComponentIterator).vertex = tail;
                (*bezierControlComponentIterator).placement = placement;
                (*bezierControlComponentIterator).setCentrePosition(bezierControlPoint.value().toInt());
                (*bezierControlComponentIterator).setVisible(true);
                (*bezierControlComponentIterator).toFront(true);
                bezierControlComponentIterator++;
            }
        }

        auto head = tail->getAdjacentVertex(placement);
        if (!head)
            return;

        {
            auto opposite = mescal::GradientMesh::opposite(placement);
            auto bezierControlPoint = head->bezier.getControlPoint(opposite);
            if (bezierControlPoint.has_value())
            {
                (*bezierControlComponentIterator).vertex = head;
                (*bezierControlComponentIterator).placement = opposite;
                (*bezierControlComponentIterator).setCentrePosition(bezierControlPoint.value().toInt());
                (*bezierControlComponentIterator).setVisible(true);
                (*bezierControlComponentIterator).toFront(true);
                bezierControlComponentIterator++;
            }
        }

        tail = head;
    }

    repaint();
}

GradientMeshEditor::VertexComponent& GradientMeshEditor::getVertexComponent(int row, int column)
{
    return vertexComponents[row * mesh->getNumColumns() + column];
}

void GradientMeshEditor::VertexComponent::mouseDown(const juce::MouseEvent& e)
{
    dragger.startDraggingComponent(this, e);
}

void GradientMeshEditor::VertexComponent::mouseDrag(const juce::MouseEvent& e)
{
    dragger.dragComponent(this, e, nullptr);
    if (onChange)
        onChange(*this);
}

void GradientMeshEditor::VertexComponent::mouseUp(const juce::MouseEvent& e)
{
    if (!e.mouseWasDraggedSinceMouseDown())
    {
        if (auto v = vertex.lock())
        {
            auto content = std::make_unique<ColorCalloutContent>(v->color.toColour());
            content->setSize(200, 200);
            content->onChange = [=](juce::Colour color)
                {
                    v->color = mescal::Color128{ color };
                    if (onChange)
                        onChange(*this);
                };
            juce::CallOutBox::launchAsynchronously(std::move(content), getScreenBounds(), nullptr);
        }
    }
}

void GradientMeshEditor::VertexComponent::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(2);
    g.setColour(juce::Colours::white);
    g.fillEllipse(r);
    g.setColour(juce::Colours::black);
    g.drawEllipse(r, 2.0f);

    if (auto v = vertex.lock())
    {
        g.setColour(v->color.toColour());
        g.fillEllipse(r.reduced(4.0f));
    }
}

void GradientMeshEditor::BezierControlComponent::mouseDown(const juce::MouseEvent& e)
{
    dragger.startDraggingComponent(this, e);
}

void GradientMeshEditor::BezierControlComponent::mouseDrag(const juce::MouseEvent& e)
{
    dragger.dragComponent(this, e, nullptr);
    if (onChange)
        onChange(*this);
}

void GradientMeshEditor::BezierControlComponent::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(2);
    g.setColour(juce::Colours::white);
    g.fillEllipse(r);
    g.setColour(juce::Colours::black);
    g.drawEllipse(r, 2.0f);
}

GradientMeshEditor::PatchComponent::PatchComponent(int row_, int column_) :
    row(row_), column(column_)
{
    setRepaintsOnMouseActivity(true);
}

bool GradientMeshEditor::PatchComponent::hitTest(int x, int y)
{
    return path.contains(juce::Point<int>(x, y).toFloat());
}

void GradientMeshEditor::PatchComponent::paint(juce::Graphics& g)
{
    if (!selected && !isMouseOver(true))
        return;

    if (selected)
    {
        g.setColour(juce::Colours::black);
        g.strokePath(path, juce::PathStrokeType(4.0f));
    }

    g.setColour(selected ? juce::Colours::white : juce::Colours::darkgrey);
    g.strokePath(path, juce::PathStrokeType(2.0f));

    if (!selected)
        return;

    juce::Path::Iterator it{ path };
    juce::Point<float> lastPoint;
    while (it.next())
    {
        switch (it.elementType)
        {
        case Path::Iterator::startNewSubPath:
        {
            lastPoint = { it.x1, it.y1 };
            break;
        }

        case Path::Iterator::cubicTo:
        {
            g.setColour(juce::Colours::black);
            g.drawLine({ lastPoint, { it.x1, it.y1 } }, 3.0f);
            g.drawLine({ { it.x2, it.y2 }, { it.x3, it.y3 } }, 3.0f);
            g.setColour(juce::Colours::white);
            g.drawLine({ lastPoint, { it.x1, it.y1 } }, 2.0f);
            g.drawLine({ { it.x2, it.y2 }, { it.x3, it.y3 } }, 2.0f);
            lastPoint = { it.x3, it.y3 };
            break;
        }
        }
    }
}

void GradientMeshEditor::PatchComponent::build(std::shared_ptr<mescal::GradientMesh::Vertex> topLeftVertex, GradientMeshEditor& owner)
{
    juce::Path outline;

    auto tail = topLeftVertex;

    outline.startNewSubPath(topLeftVertex->position);

    for (auto placement :
        {
            mescal::GradientMesh::Placement::bottom,
            mescal::GradientMesh::Placement::right,
            mescal::GradientMesh::Placement::top,
            mescal::GradientMesh::Placement::left
        })
    {
        auto head = tail->getAdjacentVertex(placement);

        auto bezeirControlPoint0 = tail->bezier.getControlPoint(placement);
        auto bezierControlPoint1 = head->bezier.getControlPoint(mescal::GradientMesh::opposite(placement));

        if (bezeirControlPoint0.has_value() && bezierControlPoint1.has_value())
        {
            outline.cubicTo(bezeirControlPoint0.value(), bezierControlPoint1.value(), head->position);
        }
        else
        {
            outline.lineTo(head->position);
        }

        tail = head;
    }

    path = outline;
}

void GradientMeshEditor::PatchComponent::mouseUp(const juce::MouseEvent& e)
{
    if (isMouseOver(true) && onSelect)
    {
        onSelect(this);
    }
}
