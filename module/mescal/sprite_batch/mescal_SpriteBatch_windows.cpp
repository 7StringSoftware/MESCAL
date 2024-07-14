namespace mescal
{

#ifdef __INTELLISENSE__

#include "mescal_SpriteBatch_windows.h"

#endif

    struct SpriteBatch::Pimpl
    {
        Pimpl(SpriteBatch& owner_) : owner(owner_)
        {
        }

        void createResources(juce::Image image)
        {
            if (!deviceContext)
            {
                if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
                {
                    if (auto adapter = pixelData->getAdapter())
                    {
                        winrt::com_ptr<ID2D1DeviceContext1> deviceContext1;
                        if (const auto hr = adapter->direct2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
                            deviceContext1.put());
                            FAILED(hr))
                        {
                            jassertfalse;
                            return;
                        }

                        deviceContext = deviceContext1.as<ID2D1DeviceContext3>();
                    }
                }
            }
        }

        void draw(juce::Image destinationImage, const std::vector<Sprite>& sprites, bool clearImage)
        {
            jassert(atlas.isValid());

            if (sprites.size() == 0)
                return;

            createResources(destinationImage);

            if (spriteBatch && spriteBatch->GetSpriteCount() != sprites.size())
            {
                spriteBatch = {};
            }

            if (!spriteBatch)
            {
                if (const auto hr = deviceContext->CreateSpriteBatch(spriteBatch.put());
                    FAILED(hr))
                {
                    jassertfalse;
                    return;
                }

                destinationRectangles.realloc(sprites.size());
                sourceRectangles.realloc(sprites.size());
            }

            if (destinationRectangles.getData() == nullptr || sourceRectangles.getData() == nullptr)
            {
                return;
            }

            deviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

            auto destinationRectangle = destinationRectangles.getData();
            auto sourceRectangle = sourceRectangles.getData();
            for (auto const& sprite : sprites)
            {
                *destinationRectangle = D2D1_RECT_F
                {
                    sprite.destination.getX(),
                    sprite.destination.getY(),
                    sprite.destination.getRight(),
                    sprite.destination.getBottom()
                };

                *sourceRectangle = D2D1_RECT_U
                {
                    static_cast<UINT32>(sprite.source.getX()),
                    static_cast<UINT32>(sprite.source.getY()),
                    static_cast<UINT32>(sprite.source.getRight()),
                    static_cast<UINT32>(sprite.source.getBottom())
                };

                ++destinationRectangle;
                ++sourceRectangle;
            }

            if (spriteBatch->GetSpriteCount() != 0)
            {
                spriteBatch->SetSprites(0, (uint32_t)sprites.size(),
                    destinationRectangles.getData(), sourceRectangles.getData());
            }
            else
            {
                spriteBatch->AddSprites((uint32_t)sprites.size(),
                    destinationRectangles.getData(), sourceRectangles.getData());
            }

            if (deviceContext && destinationImage.isValid() && spriteBatch)
            {
                auto atlasPixelData = dynamic_cast<juce::Direct2DPixelData*>(atlas.getPixelData());
                auto destinationPixelData = dynamic_cast<juce::Direct2DPixelData*>(destinationImage.getPixelData());

                if (atlasPixelData && destinationPixelData)
                {
                    auto atlasBitmap = atlasPixelData->getAdapterD2D1Bitmap();
                    auto destinationBitmap = destinationPixelData->getAdapterD2D1Bitmap();
                    if (atlasBitmap && destinationBitmap)
                    {
                        deviceContext->SetTarget(destinationBitmap);
                        deviceContext->BeginDraw();
                        deviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
                        if (clearImage)
                        {
                            deviceContext->Clear();
                        }

                        deviceContext->DrawSpriteBatch(spriteBatch.get(),
                            atlasBitmap,
                            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                            D2D1_SPRITE_OPTIONS_NONE);

                        deviceContext->EndDraw();
                        deviceContext->SetTarget(nullptr);
                    }
                }
            }
        }

        SpriteBatch& owner;
        int numSprites;
        juce::Image atlas;
        winrt::com_ptr<ID2D1DeviceContext3> deviceContext;
        winrt::com_ptr<ID2D1SpriteBatch> spriteBatch;
        juce::HeapBlock<D2D1_RECT_F> destinationRectangles;
        juce::HeapBlock<D2D1_RECT_U> sourceRectangles;
    };

    SpriteBatch::SpriteBatch() :
        pimpl(std::make_unique<Pimpl>(*this))
    {
    }

    SpriteBatch::~SpriteBatch()
    {
    }

    void SpriteBatch::setAtlas(juce::Image atlas)
    {
        pimpl->atlas = atlas;
    }

    void SpriteBatch::draw(juce::Image destinationImage, const std::vector<Sprite>& sprites, bool clearImage)
    {
        pimpl->draw(destinationImage, sprites, clearImage);
    }

} // namespace mescal

