#include "InteractiveMeshGradient.h"

InteractiveMeshGradient::InteractiveMeshGradient()
{
    {
	    auto placementIterator = mescal::MeshGradient::cornerPlacements.begin();
	    for (auto& c : vertexComponents)
	    {
	        c.setSize(30, 30);
	        addChildComponent(c);
	        c.placement = *placementIterator;
	        ++placementIterator;
	        c.onChange = [this](VertexComponent& vertexComponent)
	            {
	                if (auto patch = vertexComponent.patch.lock())
	                {
	                    patch->setCornerPosition(vertexComponent.placement, vertexComponent.getBounds().toFloat().getCentre());
	                    updatePatchComponents();
	                    repaint();
	                }
	            };
	    }
    }

    {
        auto placementIterator = mescal::MeshGradient::cornerPlacements.begin();
        for (auto& c : interiorControlComponents)
        {
            c.setSize(30, 30);
            addChildComponent(c);
            c.placement = *placementIterator;
            ++placementIterator;
            c.onChange = [this](InteriorControlComponent& interiorControlComponent)
                {
                    if (auto patch = interiorControlComponent.patch.lock())
                    {
                        patch->setInteriorControlPointPosition(interiorControlComponent.placement, interiorControlComponent.getBounds().toFloat().getCentre());
                        updatePatchComponents();
                        repaint();
                    }
                };
        }
    }

    {
        auto componentIterator = bezierControlComponents.begin();
        for (auto edgePlacement : mescal::MeshGradient::edgePlacements)
        {
            for (auto bezierControlPointPlacement : 
                { mescal::MeshGradient::BezierControlPointPlacement::first, mescal::MeshGradient::BezierControlPointPlacement::second })
            {
                auto& c = *componentIterator;

                c.setSize(20, 20);
                addChildComponent(c);
                c.edgePlacement = edgePlacement;
                c.controlPointPlacement = bezierControlPointPlacement;
                c.onChange = [this](BezierControlComponent& controlComponent)
                    {
                        if (auto patch = controlComponent.patch.lock())
                        {
                            patch->setBezierControlPointPosition(controlComponent.edgePlacement, controlComponent.controlPointPlacement, controlComponent.getBounds().toFloat().getCentre());
                            updatePatchComponents();
                            repaint();
                        }
                    };

                ++componentIterator;
            }
        }
    }

    rowCountLabel.attachToComponent(&rowCountSlider, true);
    addAndMakeVisible(rowCountLabel);
    addAndMakeVisible(rowCountSlider);
    rowCountSlider.setRange(juce::Range<double>{ 1.0, 20.0 }, 1.0);
    rowCountSlider.onValueChange = [this]()
        {
            mesh = nullptr;
            selectPatch(nullptr);
            createMesh();
            createComponents();
            updatePatchComponents();
            resized();
            repaint();
        };
    rowCountSlider.setValue(1.0, juce::dontSendNotification);

    columnCountLabel.attachToComponent(&columnCountSlider, true);
    addAndMakeVisible(columnCountLabel);
    addAndMakeVisible(columnCountSlider);
    columnCountSlider.setRange(juce::Range<double>{ 1.0, 20.0 }, 1.0);
    columnCountSlider.onValueChange = rowCountSlider.onValueChange;
    columnCountSlider.setValue(1.0, juce::dontSendNotification);

    showControlsToggle.setToggleState(true, juce::dontSendNotification);
	showControlsToggle.onClick = [this]
		{
			bool visible = showControlsToggle.getToggleState();
			for (auto& vertexComponent : vertexComponents)
				vertexComponent.setVisible(visible);
			for (auto& bezierControlComponent : bezierControlComponents)
				bezierControlComponent.setVisible(visible);
			for (auto& patchComponent : patchComponents)
				patchComponent->setVisible(visible);
		};
	addAndMakeVisible(showControlsToggle);

    setSize(768, 768);
    selectPatch(patchComponents.front().get());
}

void InteractiveMeshGradient::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    paintMesh(g);
}

void InteractiveMeshGradient::resized()
{
    createMesh();
    createComponents();

    for (auto& patchComponent : patchComponents)
    {
        patchComponent->setBounds(getLocalBounds());
    }

    updatePatchComponents();

    int sliderWidth = 120;
	rowCountSlider.setBounds(100, 10, sliderWidth, 30);
    columnCountSlider.setBounds(rowCountSlider.getBounds().translated(sliderWidth + 100, 0));
	showControlsToggle.setSize(160, 30);
	showControlsToggle.setTopLeftPosition(getWidth() - showControlsToggle.getWidth() - 40, 10);
}

void InteractiveMeshGradient::createMesh()
{
    if (getLocalBounds().isEmpty())
        return;

    if (!mesh)
    {
        int numRows = (int)rowCountSlider.getValue();
        int numColumns = (int)columnCountSlider.getValue();
        mesh = std::make_unique<mescal::MeshGradient>(numRows, numColumns, getLocalBounds().toFloat().reduced(100.0f));

        for (auto patch : mesh->getPatches())
        {
            patch->setColor(mescal::MeshGradient::CornerPlacement::topLeft, juce::Colours::red);
            patch->setColor(mescal::MeshGradient::CornerPlacement::bottomLeft, juce::Colours::blue);
            patch->setColor(mescal::MeshGradient::CornerPlacement::bottomRight, juce::Colours::yellow);
            patch->setColor(mescal::MeshGradient::CornerPlacement::topRight, juce::Colours::green);

            for (auto edgePlacement : mescal::MeshGradient::edgePlacements)
            {
                auto edge = patch->getEdge(edgePlacement);

                juce::Line<float> line{ edge.tail, edge.head };
                auto angle = line.getAngle();
                edge.bezierControlPoints.first = line.getPointAlongLineProportionally(0.33f).getPointOnCircumference(20.0f, angle + juce::MathConstants<float>::halfPi);
                edge.bezierControlPoints.second = line.getPointAlongLineProportionally(0.66f).getPointOnCircumference(20.0f, angle - juce::MathConstants<float>::halfPi);
                patch->setEdge(edgePlacement, edge);
            }
        }
    }
}

void InteractiveMeshGradient::createComponents()
{
    if (mesh)
    {
        int numRows = (int)rowCountSlider.getValue();
        int numColumns = (int)columnCountSlider.getValue();

        if (patchComponents.size() == numRows * numColumns)
        {
            return;
        }

        patchComponents.clear();

        for (int row = 0; row < numRows; ++row)
        {
            for (int column = 0; column < numColumns; ++column)
            {
                auto patchComponent = std::make_unique<PatchComponent>(mesh->getPatch(row, column));
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

void InteractiveMeshGradient::updatePatchComponents()
{
    for (auto& patchComponent : patchComponents)
    {
        patchComponent->updateOutlinePath();
    }
}

void InteractiveMeshGradient::paintMesh(juce::Graphics& g)
{
    if (meshImage.isNull() || meshImage.getWidth() != getWidth() || meshImage.getHeight() != getHeight())
        meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);

    mesh->draw(meshImage, {}, juce::Colours::transparentBlack);
    g.drawImageAt(meshImage, 0, 0);
}

void InteractiveMeshGradient::selectPatch(PatchComponent* selectedPatchComponent)
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

    for (auto& interiorControlComponent : interiorControlComponents)
    {
        interiorControlComponent.setVisible(false);
    }

    if (!selectedPatchComponent)
        return;

    selectedPatchComponent->toFront(true);
    selectedPatchComponent->selected = true;

    auto patch = selectedPatchComponent->patch;
    
    {
        auto vertexComponentIterator = vertexComponents.begin();
        auto interiorControlComponentIterator = interiorControlComponents.begin();
        for (auto placement : mescal::MeshGradient::cornerPlacements)
        {
            auto position = patch->getCornerPosition(placement);

            (*vertexComponentIterator).patch = patch;
            (*vertexComponentIterator).setCentrePosition(position.roundToInt());
            (*vertexComponentIterator).setVisible(true);
            (*vertexComponentIterator).toFront(true);
            vertexComponentIterator++;

            (*interiorControlComponentIterator).patch = patch;
            (*interiorControlComponentIterator).setCentrePosition(patch->getInteriorControlPointPosition(placement)->roundToInt());
            (*interiorControlComponentIterator).setVisible(true);
            (*interiorControlComponentIterator).toFront(true);
            interiorControlComponentIterator++;
        }
    }

    {
        auto bezierControlComponentIterator = bezierControlComponents.begin();
        for (auto edgePlacement : mescal::MeshGradient::edgePlacements)
        {
            auto edge = patch->getEdge(edgePlacement);

            (*bezierControlComponentIterator).patch = patch;
            (*bezierControlComponentIterator).edgePlacement = edgePlacement;
            (*bezierControlComponentIterator).controlPointPlacement = mescal::MeshGradient::BezierControlPointPlacement::first;
            (*bezierControlComponentIterator).setCentrePosition(edge.bezierControlPoints.first->toInt());
            (*bezierControlComponentIterator).setVisible(true);
            (*bezierControlComponentIterator).toFront(true);

            bezierControlComponentIterator++;

            (*bezierControlComponentIterator).patch = patch;
            (*bezierControlComponentIterator).edgePlacement = edgePlacement;
            (*bezierControlComponentIterator).controlPointPlacement = mescal::MeshGradient::BezierControlPointPlacement::second;
            (*bezierControlComponentIterator).setCentrePosition(edge.bezierControlPoints.second->toInt());
            (*bezierControlComponentIterator).setVisible(true);
            (*bezierControlComponentIterator).toFront(true);

            bezierControlComponentIterator++;
        }
    }

    repaint();
}

InteractiveMeshGradient::VertexComponent& InteractiveMeshGradient::getVertexComponent(int row, int column)
{
    return vertexComponents[row * mesh->getNumColumns() + column];
}

void InteractiveMeshGradient::VertexComponent::mouseDown(const juce::MouseEvent& e)
{
    dragger.startDraggingComponent(this, e);
}

void InteractiveMeshGradient::VertexComponent::mouseDrag(const juce::MouseEvent& e)
{
    dragger.dragComponent(this, e, nullptr);
    if (onChange)
        onChange(*this);
}

void InteractiveMeshGradient::VertexComponent::mouseUp(const juce::MouseEvent& e)
{
}

void InteractiveMeshGradient::VertexComponent::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(2);
    g.setColour(juce::Colours::white);
    g.fillEllipse(r);
    g.setColour(juce::Colours::black);
    g.drawEllipse(r, 2.0f);

    if (auto p = patch.lock())
    {
        g.setColour(p->getColor(placement).toColour());
        g.fillEllipse(r.reduced(4.0f));
    }
}

void InteractiveMeshGradient::BezierControlComponent::mouseDown(const juce::MouseEvent& e)
{
    dragger.startDraggingComponent(this, e);
}

void InteractiveMeshGradient::BezierControlComponent::mouseDrag(const juce::MouseEvent& e)
{
    dragger.dragComponent(this, e, nullptr);
    if (onChange)
        onChange(*this);
}

void InteractiveMeshGradient::BezierControlComponent::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(2);
    g.setColour(juce::Colours::white);
    g.fillEllipse(r);
    g.setColour(juce::Colours::black);
    g.drawEllipse(r, 2.0f);
}

InteractiveMeshGradient::PatchComponent::PatchComponent(std::shared_ptr<mescal::MeshGradient::Patch> patch_) :
    patch(patch_)
{
    setRepaintsOnMouseActivity(true);
}

bool InteractiveMeshGradient::PatchComponent::hitTest(int x, int y)
{
    return path.contains(juce::Point<int>(x, y).toFloat());
}

void InteractiveMeshGradient::PatchComponent::paint(juce::Graphics& g)
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
        case juce::Path::Iterator::startNewSubPath:
        {
            lastPoint = { it.x1, it.y1 };
            break;
        }

        case juce::Path::Iterator::cubicTo:
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

void InteractiveMeshGradient::PatchComponent::mouseUp(const juce::MouseEvent&)
{
    if (isMouseOver(true) && onSelect)
    {
        onSelect(this);
    }
}

void InteractiveMeshGradient::PatchComponent::updateOutlinePath()
{
    path.clear();

    path.startNewSubPath(patch->getCornerPosition(mescal::MeshGradient::CornerPlacement::topRight));

    {
        auto edge = patch->getEdge(mescal::MeshGradient::EdgePlacement::top);
        path.cubicTo(*edge.bezierControlPoints.first, *edge.bezierControlPoints.second, patch->getCornerPosition(mescal::MeshGradient::CornerPlacement::topLeft));
    }

    {
        auto edge = patch->getEdge(mescal::MeshGradient::EdgePlacement::left);
        path.cubicTo(*edge.bezierControlPoints.first, *edge.bezierControlPoints.second, patch->getCornerPosition(mescal::MeshGradient::CornerPlacement::bottomLeft));
    }

    {
        auto edge = patch->getEdge(mescal::MeshGradient::EdgePlacement::bottom);
        path.cubicTo(*edge.bezierControlPoints.first, *edge.bezierControlPoints.second, patch->getCornerPosition(mescal::MeshGradient::CornerPlacement::bottomRight));
    }

    {
        auto edge = patch->getEdge(mescal::MeshGradient::EdgePlacement::right);
        path.cubicTo(*edge.bezierControlPoints.first, *edge.bezierControlPoints.second, patch->getCornerPosition(mescal::MeshGradient::CornerPlacement::topRight));
    }
}
