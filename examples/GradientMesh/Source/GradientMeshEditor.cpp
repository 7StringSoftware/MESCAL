#include "GradientMeshEditor.h"
#include "Commander.h"

GradientMeshEditor::GradientMeshEditor(juce::ApplicationCommandManager& commandManager_)
    : commandManager(commandManager_),
    displayComponent(*this)
{
    commandManager_.registerAllCommandsForTarget(this);

    setOpaque(false);

    addAndMakeVisible(displayComponent);

    if (document.gradientMesh)
    {
        auto& patches = document.gradientMesh->getPatches();
        for (auto const& patch : patches)
        {
            auto patchComponent = std::make_unique<PatchComponent>(*this, patch);
            addAndMakeVisible(patchComponent.get());
            patchComponents.emplace_back(std::move(patchComponent));
        }
    }

    auto onMoved = [this]()
        {
            if (auto p = selectedPatch.lock())
            {
                p->update();
            }
            positionControls();
            repaint();
        };

    for (auto const cornerIndex : GradientMesh::corners)
    {
        GradientMesh::CornerPlacement const corner{ cornerIndex };
        auto controlPointComponent = std::make_unique<PatchCornerComponent>(corner, patchToZoomedDisplayTransform);
        controlPointComponent->onMoved = onMoved;

        controlPointComponent->onColorChanged = [this](GradientMesh::CornerPlacement corner, juce::Colour color)
            {
                if (auto p = selectedPatch.lock())
                {
                    p->setColor(corner, color);
                    repaint();
                }
            };

        addAndMakeVisible(controlPointComponent.get());

        cornerControlComponents[(int)corner] = std::move(controlPointComponent);
    }

    size_t direction = 0;
    for (auto& edgeControlGroup : edgeControlGroups)
    {
        auto group = std::make_unique<EdgeControlGroup>(*this, (GradientMesh::EdgePlacement)direction++, patchToZoomedDisplayTransform);

        //         addAndMakeVisible(group->edgeControl);
        //         addAndMakeVisible(group->bezierControlPair.first.get());
        //         addAndMakeVisible(group->bezierControlPair.second.get());

        group->bezierControlPair.first->onMoved = onMoved;
        group->bezierControlPair.second->onMoved = onMoved;

        edgeControlGroup = std::move(group);
    }

    commandManager_.setFirstCommandTarget(this);

    auto trashCan = drawables.trashCan->createCopy();
    trashButton.setSize(40, 40);
    trashButton.setCommandToTrigger(&commandManager, removePatchCommand, true);
    //addAndMakeVisible(trashButton);

    if (document.gradientMesh)
    {
        //if (document.gradientMesh->getPatches().size() > 0)
            //selectPatch(document.gradientMesh->getPatches().front());
    }
}

GradientMeshEditor::~GradientMeshEditor()
{
}

juce::Rectangle<int> GradientMeshEditor::getPreferredSize()
{
    return { 2048, 1024 };
}

void GradientMeshEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void GradientMeshEditor::paintOverChildren([[maybe_unused]] juce::Graphics& g)
{
    std::array<juce::Colour, 20> const colors
    {
        juce::Colours::yellow, juce::Colours::green, juce::Colours::blue, juce::Colours::red,
        juce::Colours::cyan, juce::Colours::magenta, juce::Colours::orange, juce::Colours::purple,
        juce::Colours::pink, juce::Colours::turquoise, juce::Colours::limegreen, juce::Colours::indigo,
        juce::Colours::darkred, juce::Colours::darkgreen, juce::Colours::darkblue, juce::Colours::darkcyan,
        juce::Colours::darkmagenta, juce::Colours::darkorange, juce::Colours::darkgoldenrod, juce::Colours::darkorchid
    };

    auto mousePos = getMouseXYRelative().toFloat().transformedBy(patchToZoomedDisplayTransform.inverted());

    size_t colorIndex = 0;

#if 0
    for (auto const& patch : document.gradientMesh->getPatches())
    {
        colorIndex = (colorIndex + 1) % colors.size();
        bool over = false;

        juce::Path path;
        auto const& halfedges = patch->getHalfedges();
        if (auto first = halfedges.front().lock())
        {
            auto tail = first->tail.lock();
            auto head = first->head.lock();

            if (tail && head)
            {
                path.startNewSubPath(tail->position);
                path.lineTo(head->position);

                for (auto it = halfedges.begin() + 1; it != halfedges.end(); ++it)
                {
                    auto halfedge = it->lock();
                    if (!halfedge)
                        continue;

                    auto head = halfedge->head.lock();
                    if (head)
                        path.lineTo(head->position);
                }

                path.closeSubPath();

                over = path.contains(mousePos);
                g.setColour(colors[colorIndex].withAlpha(over ? 1.0f : 0.5f));
                g.fillPath(path);
            }
        }

        if (!over)
            continue;

        g.setColour(juce::Colours::white);

        for (auto const& halfedgeWeakPtr : patch->getHalfedges())
        {
            auto halfedge = halfedgeWeakPtr.lock();
            if (!halfedge)
                continue;

            auto tail = halfedge->tail.lock();
            auto head = halfedge->head.lock();

            if (tail && head)
                g.drawArrow(juce::Line<float>{ tail->position.transformedBy(patchToZoomedDisplayTransform), head->position.transformedBy(patchToZoomedDisplayTransform) }, 5.0f, 20.0f, 20.0f);
        }
    }
#endif

#if 0
    colorIndex = 0;
    float vertexDisplayMinDistance = 10000.0f, edgeDisplayMinDistance = 20.0f;
    for (auto& vertex : document.gradientMesh->getVertices())
    {
        auto distance = vertex->position.getDistanceFrom(mousePos);
        if (distance > vertexDisplayMinDistance)
            continue;

        g.setColour(vertex->color);
        auto size = 16.0f;
        g.fillEllipse(juce::Rectangle<float>{ size, size }.withCentre(vertex->position.transformedBy(patchToZoomedDisplayTransform)));

#if 0
        for (auto const& halfedgeWeakPtr : vertex->halfedges)
        {
            auto halfedge = halfedgeWeakPtr.lock();
            if (!halfedge)
                continue;

            auto tail = halfedge->tail.lock();
            auto head = halfedge->head.lock();

            if (tail && head)
                g.drawArrow(juce::Line<float>{ tail->position.transformedBy(patchToZoomedDisplayTransform), head->position.transformedBy(patchToZoomedDisplayTransform) }, 5.0f, 20.0f, 20.0f);
        }
#endif
    }

    colorIndex = 0;
    for (auto const& halfedge : document.gradientMesh->getHalfedges())
    {
        if (!halfedge)
            continue;

        auto tail = halfedge->tail.lock();
        auto head = halfedge->head.lock();
        if (!tail || !head)
            continue;

        auto distance = tail->position.getDistanceFrom(mousePos);
        if (distance > edgeDisplayMinDistance)
            continue;

        g.setColour(colors[colorIndex].withAlpha(0.75f));
        colorIndex = (colorIndex + 1) % colors.size();
        auto size = 16.0f;

        auto b0 = halfedge->b0.lock();
        auto b1 = halfedge->b1.lock();

        if (b0 && b1)
        {
            Path p;
            p.startNewSubPath(tail->position);
            p.cubicTo(b0->position, b1->position, head->position);
            g.strokePath(p, PathStrokeType{ 5.0f }, patchToZoomedDisplayTransform);
        }

        g.drawArrow(juce::Line<float>{ tail->position.transformedBy(patchToZoomedDisplayTransform), head->position.transformedBy(patchToZoomedDisplayTransform) }, 5.0f, size, size);
    }
#endif
}

void GradientMeshEditor::resized()
{
    displayComponent.setBounds(getLocalBounds());

    for (auto& patchComponent : patchComponents)
    {
        patchComponent->setBounds(getLocalBounds());
    }

    auto bounds = juce::Rectangle<float>{ getWidth() * 0.75f, getHeight() * 0.75f }.withCentre({ getWidth() * 0.5f, getHeight() * 0.5f });
    auto meshBounds = document.gradientMesh->getBounds();
    juce::RectanglePlacement placement{ juce::RectanglePlacement::centred };
    auto transform = placement.getTransformToFit(meshBounds, bounds);
    document.gradientMesh->applyTransform(transform);

    //selectPatch(selectedPatch);
}

void GradientMeshEditor::mouseWheelMove(const juce::MouseEvent& /*mouseEvent*/, const juce::MouseWheelDetails& wheel)
{
    wheel.deltaY > 0 ? zoom *= 1.1f : zoom *= 0.9f;
    zoom = juce::jlimit(0.1f, 10.0f, zoom);
    //setTransform(juce::AffineTransform::scale(zoom, zoom, mouseEvent.x, mouseEvent.y));

    positionControls();

    repaint();
}

void GradientMeshEditor::mouseMove(const MouseEvent&)
{
}

void GradientMeshEditor::selectPatch(std::weak_ptr<GradientMesh::Patch> patch)
{
    selectedPatch = patch;

    for (auto& patchComponent : patchComponents)
    {
        patchComponent->selected = patchComponent->patch.lock() == patch.lock();
    }

    if (auto patchLock = selectedPatch.lock())
    {
        const auto& colors = patchLock->getColors();
        for (auto const corner : GradientMesh::corners)
        {
            auto& cornerComponent = cornerControlComponents[(int)corner];
            if (cornerComponent)
            {
                cornerComponent->vertex = patchLock->getCornerVertex(corner);
                if (auto vertexLock = cornerComponent->vertex.lock())
                {
                    cornerComponent->color = vertexLock->color;
                    cornerComponent->setVisible(true);
                }
            }
        }

        size_t controlGroupIndex = 0;
        for (auto const& halfedge : patchLock->getHalfedges())
        {
            auto halfedgeLock = halfedge.lock();
            if (halfedgeLock)
            {
                auto& group = edgeControlGroups[controlGroupIndex];
                if (group)
                {
                    group->bezierControlPair.first->halfedge = halfedge;
                    group->bezierControlPair.first->corner = halfedgeLock->tail;
                    group->bezierControlPair.first->bezier = halfedgeLock->b0;
                    group->bezierControlPair.second->halfedge = halfedge;
                    group->bezierControlPair.second->corner = halfedgeLock->head;
                    group->bezierControlPair.second->bezier = halfedgeLock->b1;
                    group->bezierControlPair.first->setVisible(true);
                    group->bezierControlPair.second->setVisible(true);
                    group->edgeControl.setVisible(true);

                    switch (halfedgeLock->edgeType)
                    {
                    case GradientMesh::EdgeType::straight:
                        group->edgeControl.lineButton.setToggleState(true, juce::dontSendNotification);
                        break;

                    case GradientMesh::EdgeType::approximateQuadratic:
                        group->edgeControl.quadraticButton.setToggleState(true, juce::dontSendNotification);
                        break;

                    case GradientMesh::EdgeType::cubic:
                        group->edgeControl.cubicButton.setToggleState(true, juce::dontSendNotification);
                        break;
                    }
                }
            }

            ++controlGroupIndex;
        }

        trashButton.setVisible(true);
    }
    else
    {
        for (auto& cornerComponent : cornerControlComponents)
        {
            if (cornerComponent)
                cornerComponent->setVisible(false);
        }

        for (auto& group : edgeControlGroups)
        {
            if (group)
            {
                if (group->bezierControlPair.first)
                    group->bezierControlPair.first->setVisible(false);
                if (group->bezierControlPair.second)
                    group->bezierControlPair.second->setVisible(false);
                group->edgeControl.setVisible(false);
            }
        }

        trashButton.setVisible(false);
    }

    positionControls();
}

juce::ApplicationCommandTarget* GradientMeshEditor::getNextCommandTarget()
{
    return nullptr;
}

void GradientMeshEditor::getAllCommands(Array<CommandID>& commands)
{
    commands.add(addNorthConnectedPatchCommand);
    commands.add(addEastConnectedPatchCommand);
    commands.add(addSouthConnectedPatchCommand);
    commands.add(addWestConnectedPatchCommand);
    commands.add(removePatchCommand);
}

void GradientMeshEditor::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
    char constexpr editCategory[] = "Edit";

    switch (commandID)
    {
    case addNorthConnectedPatchCommand:
    case addEastConnectedPatchCommand:
    case addSouthConnectedPatchCommand:
    case addWestConnectedPatchCommand:
    {
        result.setInfo(juce::translate("Add connected patch"),
            juce::translate("Add a new patch to this patch edge"),
            editCategory, 0);
        break;
    }

    case removePatchCommand:
    {
        result.setInfo(juce::translate("Remove patch"),
            juce::translate("Remove this patch"),
            editCategory, 0);
        break;
    }
    }
}

bool GradientMeshEditor::perform(const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case addEastConnectedPatchCommand:
    case addNorthConnectedPatchCommand:
    case addSouthConnectedPatchCommand:
    case addWestConnectedPatchCommand:
    {
        addConnectedPatch(info);
        return true;
    }

    case removePatchCommand:
    {
        removeSelectedPatch();
        return true;
    }
    }

    return false;
}

void GradientMeshEditor::positionControls()
{
#if 1
    patchToZoomedDisplayTransform = juce::AffineTransform::scale(zoom, zoom, getWidth() * 0.5f, getHeight() * 0.5f);

    if (auto p = selectedPatch.lock())
    {
        for (auto& cornerComponent : cornerControlComponents)
        {
            if (cornerComponent)
                cornerComponent->updatePosition();
        }

        auto direction = 0;
        for (auto& halfedgeWeakPtr : p->getHalfedges())
        {
            auto halfedge = halfedgeWeakPtr.lock();
            if (!halfedge)
                continue;

            auto& group = edgeControlGroups[(int)direction];
            if (!group)
                continue;

            auto& edgeControlComponent = group->edgeControl;
            auto tail = halfedge->tail.lock();
            auto head = halfedge->head.lock();
            if (tail && head)
            {
                auto line = juce::Line<float>{ tail->position.transformedBy(patchToZoomedDisplayTransform), head->position.transformedBy(patchToZoomedDisplayTransform) };
                auto lineCenter = line.getPointAlongLineProportionally(0.5f);
                auto angle = line.getAngle();
                auto center = lineCenter.getPointOnCircumference(100.0f, angle - juce::MathConstants<float>::halfPi);
                edgeControlComponent.setCentrePosition(center.roundToInt());
            }

            group->edgeControl.addPatchButton.setVisible(!p->isConnected((GradientMesh::EdgePlacement)direction));
            group->bezierControlPair.first->updatePosition();
            group->bezierControlPair.second->updatePosition();

            juce::Line<float> bezierLine{ group->bezierControlPair.first->getControlPointPosition(), group->bezierControlPair.second->getControlPointPosition() };
            auto bezierCenter = bezierLine.getPointAlongLineProportionally(0.5f);

            group->edgeControl.setVisible(true);
            group->bezierControlPair.first->setVisible(halfedge->edgeType != GradientMesh::EdgeType::straight);
            group->bezierControlPair.second->setVisible(group->bezierControlPair.first->isVisible());

            direction = (direction + 1) & 3;
        }
    }

    displayComponent.toBack();

    PatchComponent* selectedPatchComponent = nullptr;
    for (auto& patchComponent : patchComponents)
    {
        if (patchComponent->selected)
            selectedPatchComponent = patchComponent.get();

        patchComponent->toFront(false);
    }

    if (selectedPatchComponent)
    {
        selectedPatchComponent->toFront(false);
        if (auto p = selectedPatchComponent->patch.lock())
        {
            auto center = p->getPath().getBounds().getCentre().transformedBy(patchToZoomedDisplayTransform);
            center = getLocalPoint(selectedPatchComponent, center);
            trashButton.setCentrePosition(center.roundToInt());
        }
    }

    for (auto& cornerComponent : cornerControlComponents)
    {
        if (cornerComponent)
            cornerComponent->toFront(false);
    }

    for (auto& group : edgeControlGroups)
    {
        if (group)
        {
            group->edgeControl.toFront(false);
            group->bezierControlPair.first->toFront(false);
            group->bezierControlPair.second->toFront(false);
        }
    }

    trashButton.toFront(false);
#endif
}

GradientMeshEditor::PatchComponent::PatchComponent(GradientMeshEditor& owner_, std::weak_ptr<GradientMesh::Patch> patch_) :
    owner(owner_),
    patch(patch_)
{
    setOpaque(false);
    setRepaintsOnMouseActivity(true);
}

bool GradientMeshEditor::PatchComponent::hitTest(int x, int y)
{
    if (auto p = patch.lock())
    {
        auto const& path = p->getPath();
        auto bounds = path.getBounds().transformedBy(owner.patchToZoomedDisplayTransform);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.35f;
        auto center = bounds.getCentre();
        return center.getDistanceFrom({ (float)x, (float)y }) < radius;
    }

    return false;
}

void GradientMeshEditor::PatchComponent::mouseEnter(const juce::MouseEvent&)
{

}

void GradientMeshEditor::PatchComponent::mouseExit(const MouseEvent&)
{

}

void GradientMeshEditor::PatchComponent::mouseUp(const MouseEvent& event)
{
    if (event.mods.isLeftButtonDown())
    {
        if (owner.selectedPatch.lock() != patch.lock())
            owner.selectPatch(patch);
        else
            owner.selectPatch(std::weak_ptr<GradientMesh::Patch>{});
    }
}

void GradientMeshEditor::PatchComponent::paint(juce::Graphics& g)
{
    if (selected || isMouseOver(true))
    {
        if (auto p = patch.lock())
        {
            auto b = p->getPath().getBounds();
            g.setColour(selected ? juce::Colours::white : juce::Colours::lightgrey);
            g.strokePath(p->getPath(), juce::PathStrokeType{ 3.0f }, owner.patchToZoomedDisplayTransform);
        }
    }
}

GradientMeshEditor::ControlPointComponent::ControlPointComponent(juce::AffineTransform& zoomTransform_) :
    patchToZoomedDisplayTransform(zoomTransform_)
{
    setSize(120, 120);
    setCentrePosition(0, 0);
}

void GradientMeshEditor::ControlPointComponent::updateTransform(juce::Point<float> position)
{
    auto transformedPosition = position.transformedBy(patchToZoomedDisplayTransform);
    setTransform(juce::AffineTransform::translation(transformedPosition));
}

bool GradientMeshEditor::ControlPointComponent::hitTest(int x, int y)
{
    auto bounds = getLocalBounds().withSizeKeepingCentre(30, 30);
    return bounds.contains(x, y);
}

void GradientMeshEditor::ControlPointComponent::mouseEnter(const juce::MouseEvent&)
{
}

void GradientMeshEditor::ControlPointComponent::mouseExit(const MouseEvent&)
{
}

void GradientMeshEditor::ControlPointComponent::mouseDown(const MouseEvent& event)
{
    if (event.mods.isLeftButtonDown())
    {
        startPosition = getControlPointPosition().transformedBy(patchToZoomedDisplayTransform);
        dragging = true;
    }
}

void GradientMeshEditor::ControlPointComponent::mouseDrag(const MouseEvent& event)
{
    if (dragging)
    {
        auto position = startPosition + (event.position - event.mouseDownPosition);
        setControlPointPosition(position.transformedBy(patchToZoomedDisplayTransform.inverted()));
        updateTransform(event.position);

        moved();
    }
}

void GradientMeshEditor::ControlPointComponent::mouseUp(const MouseEvent&)
{
    dragging = false;
}

void GradientMeshEditor::ControlPointComponent::moved()
{
    if (onMoved)
    {
        onMoved();
    }
}

void GradientMeshEditor::ControlPointComponent::paint(juce::Graphics& g)
{
#if 0
    int size = isMouseOver(true) ? 24 : 18;
#if 0
    if (auto cp = controlPoint.lock())
    {
        if (cp->hasColor())
        {
            size = isMouseOver(true) ? 32 : 26;
            color = cp->getColor();
        }
}
#endif

    g.setColour(color.contrasting());
    g.fillEllipse(getLocalBounds().withSizeKeepingCentre(size, size).toFloat());
    g.setColour(color);
    g.fillEllipse(getLocalBounds().withSizeKeepingCentre(size - 4, size - 4).toFloat());
#endif
}

GradientMeshEditor::AddPatchButton::AddPatchButton() :
    juce::Button("Add Patch")
{
}

void GradientMeshEditor::AddPatchButton::paintButton(Graphics& g, bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/)
{
    g.setColour(juce::Colours::white);

    g.drawEllipse(getLocalBounds().toFloat().reduced(4.0f), 2.0f);

    juce::Rectangle<float> r{ getLocalBounds().toFloat().reduced(10.0f) };
    g.drawLine({ r.getCentreX(), r.getY(), r.getCentreX(), r.getBottom() }, 2.0f);
    g.drawLine({ r.getX(), r.getCentreY(), r.getRight(), r.getCentreY() }, 2.0f);
}

void GradientMeshEditor::addConnectedPatch(const InvocationInfo& info)
{
#if 0
    auto originalPatch = selectedPatch.lock();
    if (!originalPatch)
    {
        return;
    }

    auto direction = GradientMesh::Direction::north;
    for (auto& group : edgeControlGroups)
    {
        if (&group->edgeControl.addPatchButton == info.originatingComponent)
        {
            auto newPatch = document.gradientMesh->addConnectedPatch(originalPatch.get(), direction);

            auto patchComponent = std::make_unique<PatchComponent>(*this, newPatch);
            addAndMakeVisible(patchComponent.get());
            patchComponent->setBounds(getLocalBounds());
            patchComponents.emplace_back(std::move(patchComponent));

            repaint();
            positionControls();

            return;
        }

        direction = GradientMesh::clockwiseFrom(direction);
    }
#endif
}

void GradientMeshEditor::removeSelectedPatch()
{
    if (auto p = selectedPatch.lock())
    {
        for (auto& patchComponent : patchComponents)
        {
            if (patchComponent->patch.lock() == p)
            {
                patchComponents.erase(std::remove_if(patchComponents.begin(), patchComponents.end(), [p](auto& patchComponent)
                    {
                        return patchComponent->patch.lock() == p;
                    }), patchComponents.end());
                break;
            }
        }

        document.gradientMesh->removePatch(p.get());
        selectPatch(std::weak_ptr<GradientMesh::Patch>{});
        positionControls();
        repaint();
    }
}

void GradientMeshEditor::setEdgeType(GradientMesh::EdgePlacement edgePlacement, GradientMesh::EdgeType type)
{
#if 0
    {
        if (auto halfedge = patch->getHalfedges()[(int)direction].lock())
        {
            document.gradientMesh->setEdgeType(halfedge.get(), type);
            patch->update();
            repaint();
            positionControls();
        }
    }
#endif
}

#if 0
void GradientMeshEditor::setEdgeType(GradientMesh::Direction direction, GradientMesh::EdgeType type)
{
    if (auto patch = selectedPatch.lock())

}
#endif

GradientMeshEditor::DisplayComponent::DisplayComponent(GradientMeshEditor& owner_) :
    owner(owner_)
{
    setInterceptsMouseClicks(false, false);
}

void GradientMeshEditor::DisplayComponent::paint(juce::Graphics& g)
{
    if (meshImage.isNull() || meshImage.getWidth() != getWidth() || meshImage.getHeight() != getHeight())
    {
        meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    }

    owner.document.gradientMesh->draw(meshImage, owner.patchToZoomedDisplayTransform);
    g.drawImageAt(meshImage, 0, 0);

    g.setColour(juce::Colours::white);
    g.drawText(String{ frameCount++ }, getLocalBounds(), juce::Justification::topRight);
}

GradientMeshEditor::EdgeControlComponent::EdgeControlComponent(GradientMeshEditor& owner_, GradientMesh::EdgePlacement edgePlacement_) :
    owner(owner_),
    edgePlacement(edgePlacement_)
{
#if 1
    addPatchButton.path.addEllipse(0.1f, 0.1f, 0.8f, 0.8f);
    addPatchButton.path.addLineSegment({ 0.5f, 0.25f, 0.5f, 0.75f }, 0.1f);
    addPatchButton.path.addLineSegment({ 0.25f, 0.5f, 0.75f, 0.5f }, 0.1f);
    addAndMakeVisible(addPatchButton);
    addPatchButton.setCommandToTrigger(&owner.commandManager, addNorthConnectedPatchCommand + (int)edgePlacement_, true);

    lineButton.path.addLineSegment(juce::Line{ 0.0f, 0.7f, 1.0f, 0.3f }, 0.1f);
    addAndMakeVisible(lineButton);
    lineButton.setClickingTogglesState(true);
    lineButton.onClick = [this]
        {
            owner.setEdgeType(edgePlacement, GradientMesh::EdgeType::straight);
        };
    lineButton.setRadioGroupId(1);

    quadraticButton.path.quadraticTo({ 0.5f, 1.0f }, { 1.0f, 0.0f });
    addAndMakeVisible(quadraticButton);
    quadraticButton.setClickingTogglesState(true);
    quadraticButton.onClick = [this]
        {
            owner.setEdgeType(edgePlacement, GradientMesh::EdgeType::approximateQuadratic);
        };
    quadraticButton.setRadioGroupId(1);

    cubicButton.path.cubicTo({ 0.25f, 0.85f },
        { 0.75f, 0.15f },
        { 1.0f, 1.0f });
    addAndMakeVisible(cubicButton);
    cubicButton.setClickingTogglesState(true);
    cubicButton.onClick = [this]
        {
            owner.setEdgeType(edgePlacement, GradientMesh::EdgeType::cubic);
        };
    cubicButton.setRadioGroupId(1);
    cubicButton.setToggleState(true, juce::dontSendNotification);

    setSize(30, 120);
#endif
}

void GradientMeshEditor::EdgeControlComponent::resized()
{
    auto r = getLocalBounds().removeFromTop(30);
    addPatchButton.setBounds(r);

    r.translate(0, 30);
    lineButton.setBounds(r);

    r.translate(0, 30);
    quadraticButton.setBounds(r);

    r.translate(0, 30);
    cubicButton.setBounds(r);
}

void GradientMeshEditor::PathButton::paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto color = juce::Colours::white;
    float strokeWidth = 2.0f;

    auto transform = juce::AffineTransform::scale(0.9f * (float)getWidth(), 0.9f * (float)getHeight());
    if (shouldDrawButtonAsHighlighted)
        strokeWidth = 3.0f;

    if (shouldDrawButtonAsDown)
        transform = transform.scaled(0.8f, 0.8f, getWidth() * 0.5f, getHeight() * 0.5f);

    if (getToggleState())
    {
        strokeWidth = 4.0f;
        color = juce::Colours::aqua;
    }

    g.setColour(juce::Colours::black);
    g.strokePath(path, juce::PathStrokeType{ strokeWidth + 2.0f }, transform);
    g.setColour(color);
    g.strokePath(path, juce::PathStrokeType{ strokeWidth }, transform);
}

GradientMeshEditor::PatchCornerComponent::PatchCornerComponent(GradientMesh::CornerPlacement corner_, juce::AffineTransform& zoomTransform_) :
    ControlPointComponent(zoomTransform_),
    corner(corner_)
{
}

void GradientMeshEditor::PatchCornerComponent::paint(juce::Graphics& g)
{
    ControlPointComponent::paint(g);
}

void GradientMeshEditor::PatchCornerComponent::mouseUp(const juce::MouseEvent& event)
{
    ControlPointComponent::mouseUp(event);

    if (!event.mouseWasDraggedSinceMouseDown())
    {
        auto colourSelector = std::make_unique<juce::ColourSelector>(juce::ColourSelector::showSliders |
            juce::ColourSelector::showColourAtTop |
            juce::ColourSelector::editableColour |
            juce::ColourSelector::showColourspace |
            juce::ColourSelector::showAlphaChannel);
        auto position = getBounds().getCentre();
        int offset = 200;
        switch (corner)
        {
        case GradientMesh::CornerPlacement::topLeft:
            position += { -offset, -offset };
            break;

        case GradientMesh::CornerPlacement::topRight:
            position += { offset, -offset };
            break;

        case GradientMesh::CornerPlacement::bottomLeft:
            position += { -offset, offset };
            break;

        case GradientMesh::CornerPlacement::bottomRight:
            position += { offset, offset };
            break;
        }

        colourSelector->setCurrentColour(color);
        colourSelector->addChangeListener(this);
        colourSelector->setSize(400, 400);
        colourSelectorSafePointer = colourSelector.get();

        juce::CallOutBox::launchAsynchronously(std::move(colourSelector), juce::Rectangle<int>{ 10, 10 }.withCentre(event.getScreenPosition()), nullptr);
    }
}

void GradientMeshEditor::PatchCornerComponent::changeListenerCallback(ChangeBroadcaster*)
{
    if (colourSelectorSafePointer)
    {
        color = colourSelectorSafePointer->getCurrentColour();
        if (onColorChanged)
        {
            onColorChanged(corner, color);
        }
        getParentComponent()->repaint();
    }
}

void GradientMeshEditor::ControlPointComponent::updatePosition()
{
    auto position = getControlPointPosition().transformedBy(patchToZoomedDisplayTransform);
    setCentrePosition(position.roundToInt());
}

GradientMeshEditor::BezierControlComponent::BezierControlComponent(juce::AffineTransform& zoomTransform_) :
    ControlPointComponent(zoomTransform_)
{
}

void GradientMeshEditor::BezierControlComponent::setControlPointPosition(juce::Point<float> position) noexcept
{
    if (auto cp = bezier.lock())
    {
        cp->position = position;

        if (buddy)
        {
            auto halfedgeLock = halfedge.lock();
            auto cornerLock = corner.lock();
            auto buddyCornerLock = buddy->corner.lock();
            auto buddyBezierLock = buddy->bezier.lock();
            if (halfedgeLock && cornerLock && buddyCornerLock && buddyBezierLock)
            {
                if (halfedgeLock->edgeType == GradientMesh::EdgeType::approximateQuadratic)
                {
                    juce::Line<float> cornerToBezier{ cornerLock->position, position };
                    juce::Line<float> cornerToCorner{ cornerLock->position, buddyCornerLock->position };

                    auto theta = cornerToCorner.getAngle() - cornerToBezier.getAngle();
                    auto cornerToCornerReverseAngle = cornerToCorner.getAngle() + juce::MathConstants<float>::pi;
                    auto angle = cornerToCornerReverseAngle + theta;
                    buddyBezierLock->position = buddyCornerLock->position.getPointOnCircumference(cornerToBezier.getLength(), angle);
                    buddy->updatePosition();
                }
            }
        }
    }
}

void GradientMeshEditor::BezierControlComponent::paint(juce::Graphics& g)
{
    ControlPointComponent::paint(g);
}

GradientMeshEditor::EdgeControlGroup::EdgeControlGroup(GradientMeshEditor& owner_, GradientMesh::EdgePlacement edgePlacement_, juce::AffineTransform& zoomTransform_) :
    edgeControl(owner_, edgePlacement_),
    bezierControlPair
    (
        std::make_unique<BezierControlComponent>(zoomTransform_),
        std::make_unique<BezierControlComponent>(zoomTransform_)
    )
{
    bezierControlPair.first->buddy = bezierControlPair.second.get();
    bezierControlPair.second->buddy = bezierControlPair.first.get();
}

GradientMeshEditor::EdgeControlGroup::~EdgeControlGroup()
{
}

