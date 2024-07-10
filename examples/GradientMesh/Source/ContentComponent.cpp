#include "ContentComponent.h"
#include "GradientMeshDemo.h"
#include "SpriteBatchDemo.h"
#include "ConicGradientDemo.h"
#include "BottleDemo.h"

ContentComponent::ContentComponent()
{
    //demoComponent = std::make_unique<GradientMeshDemo>();
    //demoComponent = std::make_unique<SpriteBatchDemo>();
    demoComponent = std::make_unique<BottleDemo>();
    //demoComponent = std::make_unique<ConicGradientDemo>();
    addAndMakeVisible(demoComponent.get());
    setSize(2048, 1024);

//     addAndMakeVisible(demoSelector);
//     addAndMakeVisible(demoSelector2);
//     addAndMakeVisible(demoSelector3);
}

void ContentComponent::resized()
{
    if (demoComponent)
        demoComponent->setBounds(getLocalBounds());

    demoSelector.setBounds(getLocalBounds());
    demoSelector2.setBounds(getLocalBounds());
    demoSelector3.setBounds(getLocalBounds());

    int height = 150;
    int width = proportionOfWidth(0.22f);
    int x = proportionOfWidth(0.33f) - width / 2;
    int y = getBottom() - height - 20;
    demoSelector.setHitBox(Rectangle<int>{ x, y, width, height });
    demoSelector2.setHitBox(Rectangle<int>{ x + width + 10, y, width, height });
    demoSelector3.setHitBox(Rectangle<int>{ x + width * 2 + 10, y, width, height });
}
