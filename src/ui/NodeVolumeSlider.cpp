#include "NodeVolumeSlider.hpp"
#include "UI.hpp"
#include "../pw/PwState.hpp"
#include <cmath>

constexpr float NODE_BOTTOM_HEIGHT = 8;
constexpr float INNER_MARGIN       = 4;
constexpr float BUTTON_HEIGHT      = 26;

CNodeVolumeSlider::CNodeVolumeSlider(uint32_t id, const std::string& name) : m_id(id) {
    m_background = Hyprtoolkit::CRectangleBuilder::begin()
                       ->color([] { return g_ui->m_backend->getPalette()->m_colors.background.brighten(0.05F); })
                       ->rounding(6)
                       ->borderThickness(1)
                       ->borderColor([] { return g_ui->m_backend->getPalette()->m_colors.alternateBase; })
                       ->size({
                           Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT,
                           Hyprtoolkit::CDynamicSize::HT_SIZE_AUTO,
                           {1.F, 1.F},
                       })
                       ->commence();

    m_container = Hyprtoolkit::CNullBuilder::begin()
                      ->size({
                          Hyprtoolkit::CDynamicSize::HT_SIZE_AUTO,
                          Hyprtoolkit::CDynamicSize::HT_SIZE_AUTO,
                          {1.F, 1.F},
                      })
                      ->commence();
    m_container->setMargin(INNER_MARGIN);

    m_mainLayout = Hyprtoolkit::CColumnLayoutBuilder::begin()
                       ->gap(5)
                       ->size({
                           Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT,
                           Hyprtoolkit::CDynamicSize::HT_SIZE_AUTO,
                           {1.F, 1.F},
                       })
                       ->commence();

    m_topLayout =
        Hyprtoolkit::CRowLayoutBuilder::begin()->gap(10)->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_AUTO, {1.F, 1.F}})->commence();
    m_topLayout->setGrow(true);

    m_slider = Hyprtoolkit::CSliderBuilder::begin()
                   ->min(0.F)
                   ->max(1.F)
                   ->val(logdVolume())
                   ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {1.F, NODE_BOTTOM_HEIGHT}})
                   ->onChanged([this](SP<Hyprtoolkit::CSliderElement> s, float val) {
                       if (m_settingVolume)
                           return;

                       setVolume(unlogVolume(val), true);
                       g_pipewire->setVolume(m_id, unlogVolume(val));
                   })
                   ->commence();

    m_topName   = Hyprtoolkit::CTextBuilder::begin()->text(std::string{name})->commence();
    m_topVol    = Hyprtoolkit::CTextBuilder::begin()->text(std::format("{}%", sc<int>(logdVolume() * 100.F)))->commence();
    m_topSpacer = Hyprtoolkit::CNullBuilder::begin()->commence();
    m_topSpacer->setGrow(true);

    m_muteButton = Hyprtoolkit::CButtonBuilder::begin()
                       ->label("volume_up")
                       ->fontFamily("Material Symbols Outlined")
                       ->fontSize({Hyprtoolkit::CFontSize::HT_FONT_H3})
                       ->noBorder(true)
                       ->onMainClick([this](SP<Hyprtoolkit::CButtonElement>) {
                           setMuted(!m_muted);
                           g_pipewire->setMuted(m_id, m_muted);
                       })
                       ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {BUTTON_HEIGHT, BUTTON_HEIGHT}})
                       ->commence();

    m_muteButton->setPositionMode(Hyprtoolkit::IElement::HT_POSITION_VCENTER);

    m_topLayout->addChild(m_topName);
    m_topLayout->addChild(m_topSpacer);
    m_topLayout->addChild(m_muteButton);
    m_topLayout->addChild(m_topVol);

    m_mainLayout->addChild(m_topLayout);
    m_mainLayout->addChild(m_slider);

    m_container->addChild(m_mainLayout);

    m_background->addChild(m_container);
}

CNodeVolumeSlider::~CNodeVolumeSlider() = default;

void CNodeVolumeSlider::setVolume(float v, bool force) {
    if (!force && m_slider->sliding())
        return;

    m_vol = v;

    m_settingVolume = true;

    m_topVol->rebuild()->text(std::format("{}%", sc<int>(logdVolume() * 100.F)))->commence();
    m_slider->rebuild()->val(logdVolume())->commence();

    m_settingVolume = false;
}

void CNodeVolumeSlider::setMuted(bool x) {
    m_muted = x;
    m_muteButton->rebuild()->label(m_muted ? "volume_off" : "volume_up")->commence();
}

// K constant for logarithmic volume. Can be a matter of taste, but 8 seems alright.
constexpr float K = 8;

//
float CNodeVolumeSlider::logdVolume() {
    return std::log(1 + (K * m_vol)) / std::log(1 + K);
}

float CNodeVolumeSlider::unlogVolume(float x) {
    return (std::pow(1 + K, x) - 1) / K;
}
