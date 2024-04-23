#include "ContentComponent.h"

ContentComponent::ContentComponent()
{
    addAndMakeVisible(meshEditor);

    setSize(1024, 1024);
}

void ContentComponent::resized()
{
    meshEditor.setBounds(getLocalBounds());
}
