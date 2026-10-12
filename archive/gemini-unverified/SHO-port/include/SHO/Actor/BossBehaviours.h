#pragma once

#include "SHO/Actor/EnemyBehaviour.h"
#include <string>

namespace SHO {
namespace Actor {

// ─────────────────────────────────────────────────────────────────────────────
// CCalibanBehaviour
// Decompiled from SLES_551.47: Factory @ 0x0011F298, Ctor @ 0x0011F2E0 (size 11712)
// ─────────────────────────────────────────────────────────────────────────────
class CalibanBehaviour : public EnemyBehaviour {
public:
    CalibanBehaviour();
    virtual ~CalibanBehaviour() = default;

    void OnUpdate(float dt) override;
    void PerformStompAttack();
    void SetEnraged(bool enraged) { m_enrageFlag = enraged; }

    bool IsCharging() const { return m_chargePhase > 0; }

private:
    int m_chargePhase = 0;          // 0x2d9c
    int m_stompState = 0;           // 0x2da0
    float m_stompCooldown = 0.0f;   // 0x2da4
    bool m_enrageFlag = false;      // 0x2d94
    float m_groundShakeTimer = 0.0f;// 0x2dac
};

// ─────────────────────────────────────────────────────────────────────────────
// CFlaurosBehaviour
// Decompiled from SLES_551.47: Factory @ 0x0012BF08, Ctor @ 0x0012BF50 (size 11664)
// ─────────────────────────────────────────────────────────────────────────────
class FlaurosBossBehaviour : public EnemyBehaviour {
public:
    FlaurosBossBehaviour();
    virtual ~FlaurosBossBehaviour() = default;

    void OnUpdate(float dt) override;
    void TeleportTo(const Core::Vec3& newPos);

private:
    float m_teleportTimer = 5.0f;
    float m_shieldHealth = 100.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// CSadDaddyBehaviour
// Decompiled from SLES_551.47: Factory @ 0x0013B040, Ctor @ 0x0013B090 (size 11728)
// ─────────────────────────────────────────────────────────────────────────────
class SadDaddyBehaviour : public EnemyBehaviour {
public:
    SadDaddyBehaviour();
    virtual ~SadDaddyBehaviour() = default;

    void OnInit() override;
    void OnUpdate(float dt) override;
    void TongueSwipeAttack();

    bool IsCatatonic() const { return m_isCatatonic; }
    void LeaveCatatonicState();

private:
    bool m_isCatatonic = true;                      // 0x2d5c = 1 on spawn
    std::string m_leaveCatatonicMsg = "LeaveCatatonic"; // 0x2d90
    float m_swipeCooldown = 0.0f;                   // 0x2da4
    float m_vulnerabilityTimer = 0.0f;              // 0x2da8
    uint32_t m_leaveCatatonicHandle = 0;
};

} // namespace Actor
} // namespace SHO
