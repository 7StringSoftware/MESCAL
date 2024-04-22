
struct Direct2DEdgeDetectionEffect::EdgeDetectionPimpl : public Direct2DEffect::Pimpl
{
    EdgeDetectionPimpl() :
        Pimpl(CLSID_D2D1EdgeDetection)
    {
    }

    ~EdgeDetectionPimpl() override = default;

    void configureEffect() override
    {
    }
};

Direct2DEdgeDetectionEffect::Direct2DEdgeDetectionEffect() :
    pimpl{ std::make_unique<EdgeDetectionPimpl>() }
{
}

Direct2DEdgeDetectionEffect::~Direct2DEdgeDetectionEffect() {}

Direct2DEffect::Pimpl* Direct2DEdgeDetectionEffect::getPimpl() const noexcept
{
    return pimpl.get();
}
