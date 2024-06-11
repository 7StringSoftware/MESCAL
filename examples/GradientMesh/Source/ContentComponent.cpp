#include "ContentComponent.h"
#include "GradientMeshDemo.h"
#include "SpriteBatchDemo.h"

ContentComponent::ContentComponent()
{
    //demoComponent = std::make_unique<GradientMeshDemo>();
    demoComponent = std::make_unique<SpriteBatchDemo>();
    addAndMakeVisible(demoComponent.get());
    setSize(2048, 1024);
}

void ContentComponent::resized()
{
    if (demoComponent)
        demoComponent->setBounds(getLocalBounds());
}
