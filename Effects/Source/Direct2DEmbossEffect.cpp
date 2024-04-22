
struct Direct2DEmbossEffect::EmbossPimpl : public Direct2DEffect::Pimpl
{
    EmbossPimpl() :
        Pimpl(CLSID_D2D1Emboss)
    {
    }
    ~EmbossPimpl() override = default;

    void configureEffect() override
    {
    }
};

Direct2DEmbossEffect::Direct2DEmbossEffect() :
    pimpl{ std::make_unique<EmbossPimpl>() }
{
}

Direct2DEmbossEffect::~Direct2DEmbossEffect() = default;

Direct2DEffect::Pimpl* Direct2DEmbossEffect::getPimpl() const noexcept
{
    return pimpl.get();
}
