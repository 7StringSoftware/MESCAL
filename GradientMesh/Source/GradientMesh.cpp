/*
  ==============================================================================

    GradientMesh.cpp
    Created: 22 Apr 2024 12:03:36am
    Author:  Matt Gonzalez

  ==============================================================================
*/

#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#include <JuceHeader.h>
#include <juce_graphics/native/juce_DirectX_windows.h>
#include <juce_graphics/native/juce_Direct2DImage_windows.h>
#include "GradientMesh.h"

juce::Value colorToValue(juce::Colour c)
{
    return juce::Value{ (int)c.getARGB() };
}

#if 0
struct GradientMeshTest::Pimpl
{
    static constexpr int numColumns = 4;
    static constexpr int numRows = 4;
    static constexpr int numControlPoints = numRows * numColumns;

    struct GridPosition
    {
        int row = 0, column = 0;
    };





    class ControlPoints
    {
    public:
        auto getPosition(int row, int column) const noexcept
        {
            return points[row * numColumns + column].position;
        }

        auto getPositionPOINT2F(int row, int column) const noexcept
        {
            auto p = getPosition(row, column);
            return D2D1::Point2F(p.x, p.y);
        }

        void setPosition(int row, int column, juce::Point<float> p) noexcept
        {
            points[row * numColumns + column].position = p;
        }

        juce::Value& getColorValue(int row, int column) noexcept
        {
            return points[row * numColumns + column].colorValue;
        }

        bool isOuterCorner(int row, int column) const noexcept
        {
            for (auto const& corner : outerCorners)
            {
                if (corner.corner.row == row && corner.corner.column == column)
                {
                    return true;
                }
            }

            return false;
        }

        std::vector<ControlPoint> points;;

    } controlPoints;

    Pimpl(GradientMeshTest& owner_) : owner(owner_)
    {
        for (int row = 0; row < numRows; ++row)
            for (int column = 0; column < numColumns; ++column)
            {
                controlPoints.points.push_back({ row, column });
            }

        controlPoints.getColorValue(0, 0) = (int)juce::Colours::red.getARGB();
        controlPoints.getColorValue(0, 3) = (int)juce::Colours::blue.getARGB();
        controlPoints.getColorValue(3, 0) = (int)juce::Colours::green.getARGB();
        controlPoints.getColorValue(3, 3) = (int)juce::Colours::aliceblue.getARGB();

        makeGradient();
    }

    void createResources()
    {
        if (image.isNull())
        {
            image = juce::Image(juce::Image::ARGB, 1024, 1024, true);
        }

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

                    deviceContext = deviceContext1.as<ID2D1DeviceContext2>();
                }
            }
        }
    }

    void makeGradient()
    {
        createResources();

        D2D1_GRADIENT_MESH_PATCH patch;
        patch.point00 = controlPoints.getPositionPOINT2F(0, 0);
        patch.point01 = controlPoints.getPositionPOINT2F(0, 1);
        patch.point02 = controlPoints.getPositionPOINT2F(0, 2);
        patch.point03 = controlPoints.getPositionPOINT2F(0, 3);
        patch.point10 = controlPoints.getPositionPOINT2F(1, 0);
        patch.point11 = controlPoints.getPositionPOINT2F(1, 1);
        patch.point12 = controlPoints.getPositionPOINT2F(1, 2);
        patch.point13 = controlPoints.getPositionPOINT2F(1, 3);
        patch.point20 = controlPoints.getPositionPOINT2F(2, 0);
        patch.point21 = controlPoints.getPositionPOINT2F(2, 1);
        patch.point22 = controlPoints.getPositionPOINT2F(2, 2);
        patch.point23 = controlPoints.getPositionPOINT2F(2, 3);
        patch.point30 = controlPoints.getPositionPOINT2F(3, 0);
        patch.point31 = controlPoints.getPositionPOINT2F(3, 1);
        patch.point32 = controlPoints.getPositionPOINT2F(3, 2);
        patch.point33 = controlPoints.getPositionPOINT2F(3, 3);

        auto getColorF = [&](int row, int column)
            {
                auto colorValue = controlPoints.getColorValue(row, column);
                return juce::D2DUtilities::toCOLOR_F(juce::Colour{ (uint32)(int)colorValue.getValue() });
            };

        patch.color00 = getColorF(0, 0);
        patch.color03 = getColorF(0, 3);
        patch.color30 = getColorF(3, 0);
        patch.color33 = getColorF(3, 3);

        patch.leftEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
        patch.topEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
        patch.rightEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;
        patch.bottomEdgeMode = D2D1_PATCH_EDGE_MODE_ANTIALIASED;

        if (deviceContext)
        {
            auto hr = deviceContext->CreateGradientMesh(&patch, 1, gradientMesh.put());
            jassert(SUCCEEDED(hr));
        }

        owner.repaint();
    }

    void paintGradient(juce::Graphics& g)
    {
        if (deviceContext && image.isValid())
        {
            if (auto pixelData = dynamic_cast<juce::Direct2DPixelData*>(image.getPixelData()))
            {
                if (auto bitmap = pixelData->getAdapterD2D1Bitmap())
                {
                    deviceContext->SetTarget(bitmap);
                    deviceContext->BeginDraw();
                    deviceContext->Clear({ 0.0f, 0.0f, 0.0f, 1.0f });

                    if (gradientMesh)
                    {
                        deviceContext->DrawGradientMesh(gradientMesh.get());
                    }

                    deviceContext->EndDraw();
                    deviceContext->SetTarget(nullptr);
                }
            }

            g.drawImageAt(image, 0, 0);
        }
    }

    GradientMeshTest& owner;
    winrt::com_ptr<ID2D1DeviceContext2> deviceContext;
    winrt::com_ptr<ID2D1GradientMesh> gradientMesh;
    juce::Image image;
};

GradientMeshTest::GradientMeshTest()
{
    pimpl = std::make_unique<Pimpl>(*this);

    int row = 0, column = 0;
    for (int controlPointIndex = 0; controlPointIndex < Pimpl::numControlPoints; ++controlPointIndex)
    {
        auto comp = std::make_unique<ControlPointComponent>(row, column, pimpl->controlPoints.getColorValue(row, column));

        juce::String name;
        name << row << column;
        comp->setName(name);

        comp->onMove = [=]
            {
                auto c = controlPointComponents[controlPointIndex];
                pimpl->controlPoints.setPosition(c->row, c->column, c->getBounds().getCentre().toFloat());
                pimpl->makeGradient();
            };

        addAndMakeVisible(comp.get());

        ++row;
        if (row >= Pimpl::numRows)
        {
            row = 0;
            ++column;
        }

        controlPointComponents.add(std::move(comp));
    }

    setSize(1024, 1024);
}

void GradientMeshTest::resized()
{
    for (auto& comp : controlPointComponents)
    {
        if (comp)
        {
            if (comp->getWidth() == 0)
            {
                auto xGap = getWidth() / (Pimpl::numColumns + 1);
                auto x = comp->column * xGap + xGap;
                auto yGap = getHeight() / (Pimpl::numRows + 1);
                auto y = comp->row * yGap + yGap;
                comp->setSize(32, 32);
                comp->setCentrePosition(x, y);

                pimpl->controlPoints.setPosition(comp->row, comp->column, comp->getBounds().getCentre().toFloat());
                continue;
            }

            auto p = pimpl->controlPoints.getPosition(comp->row, comp->column);
            comp->setCentrePosition(p.roundToInt());
        }
    }
}

void GradientMeshTest::paint(juce::Graphics& g)
{
    pimpl->createResources();

    g.fillAll(juce::Colour{ 0xff111111 });

    pimpl->paintGradient(g);

    auto getComp = [&](int row, int column) -> ControlPointComponent*
    {
        return controlPointComponents[row * Pimpl::numColumns + column];
    };

    auto drawLine = [&](Pimpl::GridPosition const& gp1, Pimpl::GridPosition const& gp2)
        {
            auto c1 = getComp(gp1.row, gp1.column)->getBounds().getCentre().toFloat();
            auto c2 = getComp(gp2.row, gp2.column)->getBounds().getCentre().toFloat();;


            g.setColour(juce::Colours::black);
            g.drawLine(juce::Line<float>{ c1, c2 }, 6.0f);
            g.setColour(juce::Colours::white);
            g.drawLine(juce::Line<float>{ c1, c2 }, 3.0f);
        };

    for (auto const& cornerPoint : pimpl->outerCorners)
    {
        drawLine(cornerPoint.corner, cornerPoint.clockwiseCubicSpineControl);
        drawLine(cornerPoint.corner, cornerPoint.counterclockWiseCubicSplineControl);
    }
}
#endif

struct GradientMesh::Patch::PatchPimpl
{
    PatchPimpl()
    {
        GridPosition gridPosition;
        Point<float> normalizedPosition;
        for (int i = 0; i < numControlPoints; ++i)
        {
            controlPoints.emplace_back(ControlPoint{ gridPosition, normalizedPosition });

            gridPosition.column++;
            normalizedPosition += { 1.0f / (float)(numColumns - 1), 0.0f };
            if (gridPosition.column >= numColumns)
            {
                gridPosition.column = 0;
                gridPosition.row++;
                normalizedPosition = { 0.0f, normalizedPosition.y + 1.0f / (float)(numRows - 1) };
            }
        }

        for (auto const& outerCorner : outerCorners)
        {
            auto controlPoint = getControlPoint(outerCorner.gridPosition);
            controlPoint.colorValue = colorToValue(juce::Colours::red);
        }
    }

    ~PatchPimpl()
    {
    }

    struct OuterCorner
    {
        GridPosition gridPosition;
        GridPosition clockwiseCubicSpineControl;
        GridPosition counterclockWiseCubicSplineControl;
    };

    static constexpr std::array<OuterCorner, 4> outerCorners
    {
        OuterCorner{ { 0, 0 }, { 0, 1 }, { 1, 0 } }, // top left
        { { 0, 3 }, { 0, 2 }, { 1, 3 }, }, // top right
        { { 3, 3 }, { 3, 2 }, { 2, 3 }, }, // bottom right
        { { 3, 0 }, { 3, 1 }, { 2, 0 }, }  // bottom left
    };

    struct ControlPoint
    {
        GridPosition const gridPosition;
        juce::Point<float> normalizedPosition{};
        std::optional<juce::Value> colorValue;
    };

    ControlPoint& getControlPoint(GridPosition gridPosition)
    {
        return controlPoints[gridPosition.row * numColumns + gridPosition.column];
    }

    std::vector<ControlPoint> controlPoints;
};

struct GradientMesh::Pimpl
{
    Pimpl(GradientMesh& owner_) : owner(owner_)
    {
    }

    GradientMesh& owner;
};

GradientMesh::GradientMesh() :
    pimpl(std::make_unique<Pimpl>(*this))
{
    patches.emplace_back(std::make_unique<Patch>());
}

GradientMesh::~GradientMesh()
{
}

void GradientMesh::draw(juce::Graphics& g, juce::AffineTransform transform)
{

}

GradientMesh::Patch::Patch() :
    pimpl(std::make_unique<PatchPimpl>())
{
}

GradientMesh::Patch::~Patch()
{
}

juce::Point<float> GradientMesh::Patch::getControlPointPosition(GridPosition gridPosition) const
{
    return pimpl->getControlPoint(gridPosition).normalizedPosition;
}

std::optional<juce::Value> GradientMesh::Patch::getControlPointColorValue(GridPosition gridPosition) const
{
    return pimpl->getControlPoint(gridPosition).colorValue;
}
