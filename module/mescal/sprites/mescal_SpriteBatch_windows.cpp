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

        void createResources()
        {
            resources->create();
        }

        void draw(juce::Image destinationImage, const std::vector<Sprite>& sprites, bool clearImage)
        {
            jassert(atlas.isValid());

            if (sprites.size() == 0)
                return;

            createResources();

            juce::ComSmartPtr<ID2D1DeviceContext3> deviceContext3;
            resources->deviceContext->QueryInterface<ID2D1DeviceContext3>(deviceContext3.resetAndGetPointerAddress());
            if (!deviceContext3)
            {
                return;
            }

            if (spriteBatch && spriteBatch->GetSpriteCount() != sprites.size())
            {
                spriteBatch = {};
            }

            if (!spriteBatch)
            {
                if (const auto hr = deviceContext3->CreateSpriteBatch(spriteBatch.put());
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

            deviceContext3->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

            auto destinationRectangle = destinationRectangles.getData();
            auto sourceRectangle = sourceRectangles.getData();
            for (auto const& sprite : sprites)
            {
                *destinationRectangle = D2D1_RECT_F
                {
                    sprite.drawArea.getX(),
                    sprite.drawArea.getY(),
                    sprite.drawArea.getRight(),
                    sprite.drawArea.getBottom()
                };

                *sourceRectangle = D2D1_RECT_U
                {
                    static_cast<UINT32>(sprite.atlasSourceArea.getX()),
                    static_cast<UINT32>(sprite.atlasSourceArea.getY()),
                    static_cast<UINT32>(sprite.atlasSourceArea.getRight()),
                    static_cast<UINT32>(sprite.atlasSourceArea.getBottom())
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

            if (destinationImage.isValid() && spriteBatch)
            {
                auto atlasPixelData = dynamic_cast<juce::Direct2DPixelData*>(atlas.getPixelData());
                auto destinationPixelData = dynamic_cast<juce::Direct2DPixelData*>(destinationImage.getPixelData());

                if (atlasPixelData && destinationPixelData)
                {
                    auto atlasBitmap = atlasPixelData->getFirstPageForContext(deviceContext3);
                    auto destinationBitmap = destinationPixelData->getFirstPageForContext(deviceContext3);
                    if (atlasBitmap && destinationBitmap)
                    {
                        deviceContext3->SetTarget(destinationBitmap);
                        deviceContext3->BeginDraw();
                        deviceContext3->SetTransform(D2D1::Matrix3x2F::Identity());
                        if (clearImage)
                        {
                            deviceContext3->Clear();
                        }

                        deviceContext3->DrawSpriteBatch(spriteBatch.get(),
                            atlasBitmap,
                            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                            D2D1_SPRITE_OPTIONS_NONE);

                        deviceContext3->EndDraw();
                        deviceContext3->SetTarget(nullptr);
                    }
                }
            }
        }

        SpriteBatch& owner;
        int numSprites = 0;
        juce::Image atlas;
        juce::SharedResourcePointer<DirectXResources> resources;
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

