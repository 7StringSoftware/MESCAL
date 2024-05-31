#include "GradientMeshEditor.h"
#include "Commander.h"

GradientMeshEditor::GradientMeshEditor(juce::ApplicationCommandManager& commandManager_)
    : commandManager(commandManager_),
    displayComponent(*this)
{
    commandManager_.registerAllCommandsForTarget(this);

    setOpaque(false);

    addAndMakeVisible(displayComponent);

    auto patch = document.gradientMesh.getPatches().front();

    auto patchComponent = std::make_unique<PatchComponent>(*this, patch);
    addAndMakeVisible(patchComponent.get());
    patchComponents.emplace_back(std::move(patchComponent));

    auto onMoved = [this]()
        {
            if (auto p = selectedPatch.lock())
            {
                p->update();
            }
            positionControls();
            repaint();
        };

    for (size_t cornerPosition = GradientMesh::CornerPosition::topLeft; cornerPosition <= GradientMesh::CornerPosition::bottomLeft; ++cornerPosition)
    {
        auto vertex = patch->getCornerVertex(cornerPosition);
        auto controlPointComponent = std::make_unique<PatchCornerComponent>(cornerPosition, zoomTransform);
        controlPointComponent->setControlPointPosition(vertex->position);
        controlPointComponent->onMoved = onMoved;
        addAndMakeVisible(controlPointComponent.get());

        cornerControlComponents[cornerPosition] = std::move(controlPointComponent);
    }

    for (size_t edgePosition = GradientMesh::EdgePosition::top; edgePosition <= GradientMesh::EdgePosition::left; ++edgePosition)
    {
        auto group = std::make_unique<EdgeControlGroup>(*this, edgePosition, zoomTransform);

        addAndMakeVisible(group->edgeControl);
        addAndMakeVisible(group->bezierControlPair.first.get());
        addAndMakeVisible(group->bezierControlPair.second.get());

        group->bezierControlPair.first->onMoved = onMoved;
        group->bezierControlPair.second->onMoved = onMoved;

        edgeControlGroups[edgePosition] = std::move(group);
    }

    commandManager_.setFirstCommandTarget(this);

    selectPatch(patch);
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

void GradientMeshEditor::resized()
{
    displayComponent.setBounds(getLocalBounds());

    for (auto& patchComponent : patchComponents)
    {
        patchComponent->setBounds(getLocalBounds());
    }

    auto bounds = juce::Rectangle<float>{ getWidth() * 0.75f, getHeight() * 0.75f }.withCentre({ getWidth() * 0.5f, getHeight() * 0.5f });
    auto meshBounds = document.gradientMesh.getBounds();
    juce::RectanglePlacement placement{ juce::RectanglePlacement::centred };
    auto transform = placement.getTransformToFit(meshBounds, bounds);
    document.gradientMesh.applyTransform(transform);

    positionControls();

    selectPatch(selectedPatch);
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

    if (auto p = selectedPatch.lock())
    {
        const auto& colors = p->getColors();
        for (size_t corner = GradientMesh::CornerPosition::topLeft; corner <= GradientMesh::CornerPosition::bottomLeft; ++corner)
        {
            cornerControlComponents[corner]->vertex = p->getCornerVertex(corner);
            cornerControlComponents[corner]->color = colors[corner];
        }

        for (size_t edgePosition = GradientMesh::EdgePosition::top; edgePosition <= GradientMesh::EdgePosition::left; ++edgePosition)
        {
            auto halfedge = p->getHalfedges()[edgePosition];
            auto& group = edgeControlGroups[edgePosition];
            group->bezierControlPair.first->bezier = halfedge->b0;
            group->bezierControlPair.second->bezier = halfedge->b1;
        }
    }

#if 0
    if (auto p = selectedPatch.lock())
    {
        for (int row = 0; row < GradientMesh::Patch::numRows; ++row)
        {
            for (int column = 0; column < GradientMesh::Patch::numColumns; ++column)
            {
                controlPointComponents.get(row, column)->setControlPoint(p->getControlPoint(row, column));
            }
        }
    }

    std::array<size_t, 4> edgePositions{ GradientMesh::EdgePosition::top, GradientMesh::EdgePosition::right, GradientMesh::EdgePosition::bottom, GradientMesh::EdgePosition::left };
    for (size_t edgePosition : edgePositions)
    {
        auto& edgeComponent = edgeControlComponents[edgePosition];
        bool visible = false;
        if (auto p = patch.lock())
        {
            visible = true;
        }

        edgeComponent.setVisible(visible);
    }
#endif
}

juce::ApplicationCommandTarget* GradientMeshEditor::getNextCommandTarget()
{
    return nullptr;
}

void GradientMeshEditor::getAllCommands(Array<CommandID>& commands)
{
    commands.add(addConnectedPatchCommand);
}

void GradientMeshEditor::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
    char constexpr editCategory[] = "Edit";

    switch (commandID)
    {
    case addConnectedPatchCommand:
    {
        result.setInfo(juce::translate("Add connected patch"),
            juce::translate("Add a new patch to this patch edge"),
            editCategory, 0);
        break;
    }
    }
}

bool GradientMeshEditor::perform(const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case addConnectedPatchCommand:
    {
        addConnectedPatch(info);
        return true;
    }
    }

    return false;
}

void GradientMeshEditor::positionControls()
{
    zoomTransform = juce::AffineTransform::scale(zoom, zoom, getWidth() * 0.5f, getHeight() * 0.5f);

    auto setCompCenter = [&](Component* component, juce::Point<float> position)
        {
            auto transformedPosition = position.transformedBy(zoomTransform);
            component->setCentrePosition(transformedPosition.roundToInt());
        };

    if (auto p = selectedPatch.lock())
    {
        for (auto& cornerComponent : cornerControlComponents)
        {
            if (auto v = cornerComponent->vertex.lock())
            {
                setCompCenter(cornerComponent.get(), v->position);
            }
        }

        size_t edgePosition = GradientMesh::EdgePosition::top;
        for (auto const& halfedge : p->getHalfedges())
        {
            auto& group = edgeControlGroups[edgePosition++];
            auto& edgeControlComponent = group->edgeControl;
            auto line = juce::Line<float>{ halfedge->tail->position.transformedBy(zoomTransform), halfedge->head->position.transformedBy(zoomTransform) };
            auto lineCenter = line.getPointAlongLineProportionally(0.5f);
            auto angle = line.getAngle();
            auto center = lineCenter.getPointOnCircumference(100.0f, angle - juce::MathConstants<float>::halfPi);
            edgeControlComponent.setCentrePosition(center.roundToInt());

            setCompCenter(group->bezierControlPair.first.get(), halfedge->b0->position);
            setCompCenter(group->bezierControlPair.second.get(), halfedge->b1->position);
        }
    }
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
        return p->getPath().contains(Point<int>{ x, y }.toFloat());
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
        owner.selectPatch(patch);
    }
}

void GradientMeshEditor::PatchComponent::paint(juce::Graphics& g)
{
    if (selected)
    {
        if (auto p = patch.lock())
        {
            g.setColour(juce::Colours::white);
            auto transform = juce::AffineTransform::translation(-getControlPointPosition().toFloat()).followedBy(owner.zoomTransform);
            g.strokePath(p->getPath(), juce::PathStrokeType{ 3.0f }, transform);
        }
    }
}

GradientMeshEditor::ControlPointComponent::ControlPointComponent(juce::AffineTransform& zoomTransform_) :
    zoomTransform(zoomTransform_)
{
    setSize(120, 120);
    setCentrePosition(0, 0);
}

void GradientMeshEditor::ControlPointComponent::updateTransform(juce::Point<float> position)
{
    auto transformedPosition = position.transformedBy(zoomTransform);
    setTransform(juce::AffineTransform::translation(transformedPosition));
}

bool GradientMeshEditor::ControlPointComponent::hitTest(int x, int y)
{
    return getLocalBounds().getCentre().getDistanceFrom({ x, y }) < 15;
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
        startPosition = getControlPointPosition();
        dragging = true;
    }
}

void GradientMeshEditor::ControlPointComponent::mouseDrag(const MouseEvent& event)
{
    if (dragging)
    {
        auto position = startPosition + (event.position - event.mouseDownPosition);
        setControlPointPosition(position);
        updateTransform(position);

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

void GradientMeshEditor::addConnectedPatch(const InvocationInfo& /*info*/)
{
#if 0
    auto originalPatch = selectedPatch.lock();
    if (!originalPatch)
    {
        return;
    }

    auto edgePosition = GradientMesh::EdgePosition::top;
    for (auto& edgeControlComponent : edgeControlComponents)
    {
        if (&edgeControlComponent.addPatchButton == info.originatingComponent)
        {
            auto newPatch = originalPatch->createConnectedPatch(edgePosition);
            document.gradientMesh.addPatch(newPatch);

            auto patchComponent = std::make_unique<PatchComponent>(*this, newPatch);
            addAndMakeVisible(patchComponent.get());
            patchComponent->setBounds(getLocalBounds());
            patchComponents.emplace_back(std::move(patchComponent));

            repaint();
            positionControls();

            return;
        }

        ++edgePosition;
    }
#endif
}

void GradientMeshEditor::setEdgeType(size_t edgePosition, GradientMesh::EdgeType type)
{
#if 0
    auto patch = selectedPatch.lock();
    if (!patch)
    {
        return;
    }

    patch->setEdgeType(edgePosition, type);
    repaint();
    positionControls();
#endif
}

GradientMeshEditor::DisplayComponent::DisplayComponent(GradientMeshEditor& owner_) :
    owner(owner_)
{
}

void GradientMeshEditor::DisplayComponent::paint(juce::Graphics& g)
{
    if (meshImage.isNull() || meshImage.getWidth() != getWidth() || meshImage.getHeight() != getHeight())
    {
        meshImage = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
    }

    owner.document.gradientMesh.draw(meshImage, owner.zoomTransform);
    g.drawImageAt(meshImage, 0, 0);
}

GradientMeshEditor::EdgeControlComponent::EdgeControlComponent(GradientMeshEditor& owner_, size_t edgePosition_) :
    owner(owner_),
    edgePosition(edgePosition_)
{
    addPatchButton.path.addEllipse(0.1f, 0.1f, 0.8f, 0.8f);
    addPatchButton.path.addLineSegment({ 0.5f, 0.25f, 0.5f, 0.75f }, 0.1f);
    addPatchButton.path.addLineSegment({ 0.25f, 0.5f, 0.75f, 0.5f }, 0.1f);
    addAndMakeVisible(addPatchButton);
    addPatchButton.setCommandToTrigger(&owner.commandManager, addConnectedPatchCommand, true);

    lineButton.path.addLineSegment(juce::Line{ 0.0f, 0.7f, 1.0f, 0.3f }, 0.1f);
    addAndMakeVisible(lineButton);
    lineButton.setClickingTogglesState(true);
    lineButton.onClick = [this]
        {
            owner.setEdgeType(edgePosition, GradientMesh::EdgeType::straight);
        };
    lineButton.setRadioGroupId(1);

    quadraticButton.path.quadraticTo({ 0.5f, 1.0f }, { 1.0f, 0.0f });
    addAndMakeVisible(quadraticButton);
    quadraticButton.setClickingTogglesState(true);
    quadraticButton.onClick = [this]
        {
            owner.setEdgeType(edgePosition, GradientMesh::EdgeType::quadratic);
        };
    quadraticButton.setRadioGroupId(1);

    cubicButton.path.cubicTo({ 0.25f, 0.85f },
        { 0.75f, 0.15f },
        { 1.0f, 1.0f });
    addAndMakeVisible(cubicButton);
    cubicButton.setClickingTogglesState(true);
    cubicButton.onClick = [this]
        {
            owner.setEdgeType(edgePosition, GradientMesh::EdgeType::cubic);
        };
    cubicButton.setRadioGroupId(1);
    cubicButton.setToggleState(true, juce::dontSendNotification);

    setSize(30, 120);
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

void GradientMeshEditor::EdgeControlComponent::paint(juce::Graphics& g)
{
    //g.fillAll(juce::Colours::darkgrey);
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

    g.setColour(color);
    g.strokePath(path, juce::PathStrokeType{ strokeWidth }, transform);
}

GradientMeshEditor::PatchCornerComponent::PatchCornerComponent(size_t cornerPosition_, juce::AffineTransform& zoomTransform_) :
    ControlPointComponent(zoomTransform_),
    cornerPosition(cornerPosition_)
{

}

void GradientMeshEditor::PatchCornerComponent::paint(juce::Graphics& g)
{
    ControlPointComponent::paint(g);
}

GradientMeshEditor::BezierControlComponent::BezierControlComponent(juce::AffineTransform& zoomTransform_) :
    ControlPointComponent(zoomTransform_)
{
}

void GradientMeshEditor::BezierControlComponent::paint(juce::Graphics& g)
{
    ControlPointComponent::paint(g);
}

GradientMeshEditor::EdgeControlGroup::EdgeControlGroup(GradientMeshEditor& owner_, size_t edgePosition_, juce::AffineTransform& zoomTransform_) :
    edgeControl(owner_, edgePosition_),
    bezierControlPair
    (
        std::make_unique<BezierControlComponent>(zoomTransform_),
        std::make_unique<BezierControlComponent>(zoomTransform_)
    )
{
}
