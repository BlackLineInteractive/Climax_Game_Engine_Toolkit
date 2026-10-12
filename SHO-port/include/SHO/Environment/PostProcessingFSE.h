#pragma once

#include "SHO/Core/Behaviour.h"
#include "SHO/Core/EventManager.h"
#include <string>

namespace SHO {
namespace Environment {

// ─────────────────────────────────────────────────────────────────────────────
// cBlackOutFSE & cPenanceFSE & cFlexFilter
// Decompiled from SLES_551.47 @ 0x00156A50, 0x001622F0, 0x0015EDD8
// ─────────────────────────────────────────────────────────────────────────────
class FullScreenEffect : public Core::Behaviour {
public:
    FullScreenEffect();
    virtual ~FullScreenEffect() = default;

    void SetEffectActive(bool active) { m_active = active; }
    bool IsEffectActive() const { return m_active; }

    void SetIntensity(float intensity) { m_intensity = intensity; }
    float GetIntensity() const { return m_intensity; }

    void SetColorTint(const Core::Vec3& tint) { m_tint = tint; }
    const Core::Vec3& GetColorTint() const { return m_tint; }

private:
    bool m_active = false;
    float m_intensity = 1.0f;
    Core::Vec3 m_tint = Core::Vec3(1.0f);
};

// ─────────────────────────────────────────────────────────────────────────────
// cThreatController & CThreatConfig
// Decompiled from SLES_551.47: Factory @ 0x00168630, Ctor @ 0x00168678 (size 132)
// ─────────────────────────────────────────────────────────────────────────────
class ThreatController : public Core::Behaviour {
public:
    ThreatController();
    virtual ~ThreatController();

    void OnInit() override;

    void SetEnterEffectMessage(const std::string& msg);
    void SetExitEffectMessage(const std::string& msg);

    void SetFilterParams(float p0, float p1, float p2, float p3) {
        m_param0 = p0; m_param1 = p1; m_param2 = p2; m_param3 = p3;
    }

    void OnEnter();
    void OnExit();

    bool IsActive() const { return m_triggered; }

private:
    float m_param0 = 1.0f; // 0x54
    float m_param1 = 1.0f; // 0x58
    float m_param2 = 1.0f; // 0x60
    float m_param3 = 1.0f; // 0x64
    std::string m_enterMsg = "effectEnterMirror"; // 0x74
    std::string m_exitMsg = "animExitMirror";     // 0x7c
    bool m_triggered = false;

    uint32_t m_enterHandle = 0;
    uint32_t m_exitHandle = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// CFullScreenRain & CFRainDropManager
// Decompiled from SLES_551.47: Factory @ 0x00162B18, Ctor @ 0x00162B68 (size 7596)
// ─────────────────────────────────────────────────────────────────────────────
class RainManager : public Core::Behaviour {
public:
    RainManager();
    virtual ~RainManager() = default;

    void SetRainIntensity(float intensity) { m_intensity = intensity; }
    float GetRainIntensity() const { return m_intensity; }

    void SetDropletCount(int count) { m_dropletCount = count; }
    int GetDropletCount() const { return m_dropletCount; }

    void OnUpdate(float dt) override;

private:
    float m_intensity = 0.5f;
    int m_dropletCount = 500;
};

} // namespace Environment
} // namespace SHO
