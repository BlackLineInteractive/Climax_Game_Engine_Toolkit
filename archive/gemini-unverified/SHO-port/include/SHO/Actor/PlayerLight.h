#pragma once

#include "SHO/Environment/Light.h"
#include "SHO/Core/EventManager.h"
#include "SHO/Core/EventIds.h"

namespace SHO {
namespace Actor {

// Translated from CPlayerLight (factory @ 0x00141CB8, ctor @ 0x00141D00, size 160)
// Subscribes to PlayerTorchOn / PlayerTorchOff and attaches to bone TORCH_MOUNT.
class PlayerLight : public Environment::Light {
public:
    PlayerLight();
    virtual ~PlayerLight();

    void OnInit();
    void OnEvent(const Core::Msg& msg);

    bool IsTorchOn() const { return m_isOn; }
    void SetTorchOn(bool on);

    const std::string& GetMountBone() const { return m_mountBone; }

private:
    std::string m_mountBone = "TORCH_MOUNT";
    bool  m_isOn = false;
    uint32_t m_torchOnHandle = 0;
    uint32_t m_torchOffHandle = 0;
};

} // namespace Actor
} // namespace SHO
