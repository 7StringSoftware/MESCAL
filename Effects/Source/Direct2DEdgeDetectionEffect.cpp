
void Direct2DEdgeDetectionEffect::applyEffect(juce::Image& sourceImage, juce::Graphics& destContext, float scaleFactor, float alpha)
{
    auto sourcePixelData = dynamic_cast<juce::Direct2DPixelData*>(sourceImage.getPixelData());
    if (!sourcePixelData)
    {
        return;
    }

    pimpl->createResources(sourceImage);
    if (!pimpl->deviceContext || !pimpl->adapter || !pimpl->adapter->dxgiAdapter)
    {
        return;
    }

    auto& outputPixelData = pimpl->outputPixelData;
    if (!outputPixelData || outputPixelData->width < sourceImage.getWidth() || outputPixelData->height < sourceImage.getHeight())
    {
        outputPixelData = juce::Direct2DPixelData::make(juce::Image::ARGB, sourceImage.getWidth(), sourceImage.getHeight(), true, pimpl->adapter);
    }

    if (auto hr = pimpl->deviceContext->CreateEffect(CLSID_D2D1EdgeDetection, pimpl->d2dEffect.put()); FAILED(hr))
    {
        jassertfalse;
        return;
    }

    pimpl->d2dEffect->SetInput(0, sourcePixelData->getAdapterD2D1Bitmap());
    pimpl->deviceContext->SetTarget(outputPixelData->getAdapterD2D1Bitmap());
    pimpl->deviceContext->BeginDraw();
    pimpl->deviceContext->DrawImage(pimpl->d2dEffect.get());
    pimpl->deviceContext->EndDraw();

    auto outputImage = juce::Image{ pimpl->outputPixelData }.getClippedImage(sourceImage.getBounds());
    destContext.drawImageAt(outputImage, 0, 0);
}
