#include "SHO/Actor/PlayerLight.h"

namespace SHO {
namespace Actor {

PlayerLight::PlayerLight() {
    m_className = "CPlayerLight";
    m_mountBone = "TORCH_MOUNT";
    m_isOn = false;

    // Ground-truth values from SLES_551.47 (CColorLight base @ 0x00140110 / CPlayerLight @ 0x00141D00)
    // White light: 0xFFFFFFFF, cone: 45 deg, range: 3.0m
    SetColor(Core::Vec3(1.0f, 1.0f, 1.0f));
    SetIntensity(1.0f);
    SetRange(3.0f);
    SetLightType(Environment::LightType::Spot);
    SetConeAngleDeg(45.0f);
    SetActive(false);
    SetVisible(false);

    OnInit();
}

PlayerLight::~PlayerLight() {
    auto& em = Core::EventManager::GetInstance();
    if (m_torchOnHandle) em.UnlinkMsg(m_torchOnHandle);
    if (m_torchOffHandle) em.UnlinkMsg(m_torchOffHandle);
}

void PlayerLight::OnInit() {
    auto& em = Core::EventManager::GetInstance();
    m_torchOnHandle = em.LinkMsg(Core::Event::PlayerTorchOn, [this](const Core::Msg& msg) {
        OnEvent(msg);
    });
    m_torchOffHandle = em.LinkMsg(Core::Event::PlayerTorchOff, [this](const Core::Msg& msg) {
        OnEvent(msg);
    });
}

void PlayerLight::OnEvent(const Core::Msg& msg) {
    if (msg.id.name == Core::Event::PlayerTorchOn) {
        SetTorchOn(true);
    } else if (msg.id.name == Core::Event::PlayerTorchOff) {
        SetTorchOn(false);
    }
}

void PlayerLight::SetTorchOn(bool on) {
    m_isOn = on;
    SetActive(on);
    SetVisible(on);
}

} // namespace Actor
} // namespace SHO
