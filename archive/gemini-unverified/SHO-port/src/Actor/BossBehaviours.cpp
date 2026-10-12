#include "SHO/Actor/BossBehaviours.h"
#include "SHO/Core/EventManager.h"

namespace SHO {
namespace Actor {

// ─────────────────────────────────────────────────────────────────────────────
// CCalibanBehaviour
// ─────────────────────────────────────────────────────────────────────────────
CalibanBehaviour::CalibanBehaviour()
    : EnemyBehaviour(EnemyArchetype::CalibanBoss) {
    m_className = "CCalibanBehaviour";
    // 0x0011F2E0: Initial state values
    m_health = 450.0f;
    m_maxHealth = 450.0f;
    m_chargePhase = 0;
    m_stompState = 0;
    m_stompCooldown = 0.0f;
    m_enrageFlag = false;
    m_groundShakeTimer = 0.0f;
}

void CalibanBehaviour::OnUpdate(float dt) {
    if (!IsAlive()) return;

    if (m_stompCooldown > 0.0f) {
        m_stompCooldown -= dt;
    }

    if (m_groundShakeTimer > 0.0f) {
        m_groundShakeTimer -= dt;
    }

    EnemyBehaviour::OnUpdate(dt);
}

void CalibanBehaviour::PerformStompAttack() {
    if (m_stompCooldown <= 0.0f) {
        m_stompCooldown = 4.0f;
        m_stompState = 1;
        m_groundShakeTimer = 1.2f;
        Core::EventManager::GetInstance().SendMsg("CalibanStomp", this);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CFlaurosBehaviour
// ─────────────────────────────────────────────────────────────────────────────
FlaurosBossBehaviour::FlaurosBossBehaviour()
    : EnemyBehaviour(EnemyArchetype::FlaurosBoss) {
    m_className = "CFlaurosBehaviour";
    m_health = 1000.0f;
    m_maxHealth = 1000.0f;
}

void FlaurosBossBehaviour::OnUpdate(float dt) {
    if (!IsAlive()) return;

    m_teleportTimer -= dt;
    if (m_teleportTimer <= 0.0f) {
        m_teleportTimer = 6.0f;
        Core::EventManager::GetInstance().SendMsg("FlaurosTeleport", this);
    }

    EnemyBehaviour::OnUpdate(dt);
}

void FlaurosBossBehaviour::TeleportTo(const Core::Vec3& newPos) {
    SetPosition(newPos);
    Core::EventManager::GetInstance().SendMsg("FlaurosArrival", this);
}

// ─────────────────────────────────────────────────────────────────────────────
// CSadDaddyBehaviour
// ─────────────────────────────────────────────────────────────────────────────
SadDaddyBehaviour::SadDaddyBehaviour()
    : EnemyBehaviour(EnemyArchetype::SadDaddyTongue) {
    m_className = "CSadDaddyBehaviour";
    // 0x0013B090: Initial state values
    m_health = 800.0f;
    m_maxHealth = 800.0f;
    m_isCatatonic = true; // 0x2d5c = 1
    m_leaveCatatonicMsg = "LeaveCatatonic"; // 0x2d90
    m_swipeCooldown = 0.0f;
    m_vulnerabilityTimer = 0.0f;

    OnInit();
}

void SadDaddyBehaviour::OnInit() {
    EnemyBehaviour::OnInit();

    auto& em = Core::EventManager::GetInstance();
    m_leaveCatatonicHandle = em.LinkMsg(m_leaveCatatonicMsg, [this](const Core::Msg&) {
        LeaveCatatonicState();
    });
}

void SadDaddyBehaviour::LeaveCatatonicState() {
    m_isCatatonic = false;
    Core::EventManager::GetInstance().SendMsg("SadDaddyAwake", this);
}

void SadDaddyBehaviour::OnUpdate(float dt) {
    if (!IsAlive()) return;

    if (m_isCatatonic) {
        // Boss sits inert in catatonic state until LeaveCatatonic message is received
        return;
    }

    if (m_swipeCooldown > 0.0f) {
        m_swipeCooldown -= dt;
    }

    EnemyBehaviour::OnUpdate(dt);
}

void SadDaddyBehaviour::TongueSwipeAttack() {
    if (m_isCatatonic) return;

    if (m_swipeCooldown <= 0.0f) {
        m_swipeCooldown = 3.5f;
        Core::EventManager::GetInstance().SendMsg("SadDaddySwipe", this);
    }
}

} // namespace Actor
} // namespace SHO
