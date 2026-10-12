#include "SHO/Actor/EnemyBehaviour.h"
#include <algorithm>
#include <cmath>

namespace SHO {
namespace Actor {

// Global active monster counter tracking DAT_00338848 in SLES_551.47 @ 0x00127718
int32_t EnemyBehaviour::s_activeMonsterCount = 0;

EnemyBehaviour::EnemyBehaviour(EnemyArchetype archetype) {
    m_className = "CEnemyBehaviour";
    ConfigureArchetype(archetype);
    s_activeMonsterCount++;
    OnInit();
}

EnemyBehaviour::~EnemyBehaviour() {
    s_activeMonsterCount = std::max(0, s_activeMonsterCount - 1);

    auto& em = Core::EventManager::GetInstance();
    if (m_awarenessMsgHandle) {
        em.UnlinkMsg(m_awarenessMsgHandle);
    }
}

void EnemyBehaviour::ConfigureArchetype(EnemyArchetype archetype) {
    m_archetype = archetype;

    // Ground truth health & sensory constants from sho_enemies.json / SLES decompilation
    switch (archetype) {
    case EnemyArchetype::PsychoNurse:
    case EnemyArchetype::PsychoNurseSan:
        m_name = "PsychoNurse";
        m_health = 100.0f;
        m_maxHealth = 100.0f;
        m_coneAngleDeg = 45.0f;
        m_sightRangeUnlit = 3.0f;
        m_sightRangeLit = 7.5f;
        m_hearingRange = 15.0f;
        m_attackRadius = 1.2f;
        m_grabAnimName = "Grabbed_Nurse";
        break;

    case EnemyArchetype::Carrion:
    case EnemyArchetype::InvisibleMan:
    case EnemyArchetype::SadDaddyTongue:
        m_name = "Carrion";
        m_health = 125.0f;
        m_maxHealth = 125.0f;
        m_coneAngleDeg = 45.0f;
        m_sightRangeUnlit = 3.0f;
        m_sightRangeLit = 7.5f;
        m_hearingRange = 20.0f;
        m_attackRadius = 1.6f;
        m_grabAnimName = "Grabbed_Carrion";
        break;

    case EnemyArchetype::StraightJacket:
    case EnemyArchetype::StraightJacketAlt:
    case EnemyArchetype::StraightJacketBoss:
        m_name = "StraightJacket";
        m_health = (archetype == EnemyArchetype::StraightJacketBoss) ? 300.0f : 125.0f;
        m_maxHealth = m_health;
        m_coneAngleDeg = 45.0f;
        m_sightRangeUnlit = 3.0f;
        m_sightRangeLit = 7.5f;
        m_hearingRange = 15.0f;
        m_attackRadius = 1.4f;
        m_grabAnimName = "Grabbed_StraightJacket";
        break;

    case EnemyArchetype::Ariel:
        m_name = "Ariel";
        m_health = 150.0f;
        m_maxHealth = 150.0f;
        m_coneAngleDeg = 60.0f;
        m_sightRangeUnlit = 4.0f;
        m_sightRangeLit = 9.0f;
        m_hearingRange = 18.0f;
        m_attackRadius = 1.5f;
        m_grabAnimName = "Grabbed_Ariel";
        break;

    case EnemyArchetype::TwoBack:
    case EnemyArchetype::TwoBackNoTNF:
        m_name = "TwoBack";
        m_health = 200.0f;
        m_maxHealth = 200.0f;
        m_coneAngleDeg = 40.0f;
        m_sightRangeUnlit = 3.5f;
        m_sightRangeLit = 8.0f;
        m_hearingRange = 16.0f;
        m_attackRadius = 1.8f;
        m_grabAnimName = "Grabbed_2Back";
        break;

    case EnemyArchetype::Butcher:
        m_name = "TheButcher";
        m_health = 450.0f;
        m_maxHealth = 450.0f;
        m_coneAngleDeg = 50.0f;
        m_sightRangeUnlit = 4.5f;
        m_sightRangeLit = 10.0f;
        m_hearingRange = 25.0f;
        m_attackRadius = 2.2f;
        m_grabAnimName = "Grabbed_Butcher";
        break;

    case EnemyArchetype::CalibanBoss:
        m_name = "Caliban";
        m_health = 900.0f;
        m_maxHealth = 900.0f;
        m_coneAngleDeg = 60.0f;
        m_sightRangeUnlit = 6.0f;
        m_sightRangeLit = 12.0f;
        m_hearingRange = 30.0f;
        m_attackRadius = 3.5f;
        m_grabAnimName = "Grabbed_Caliban";
        break;

    case EnemyArchetype::FlaurosBoss:
        m_name = "Flauros";
        m_health = 1000.0f;
        m_maxHealth = 1000.0f;
        m_coneAngleDeg = 90.0f;
        m_sightRangeUnlit = 8.0f;
        m_sightRangeLit = 15.0f;
        m_hearingRange = 35.0f;
        m_attackRadius = 4.0f;
        m_grabAnimName = "Grabbed_Flauros";
        break;

    default:
        m_name = "Monster";
        m_health = 100.0f;
        m_maxHealth = 100.0f;
        break;
    }
}

void EnemyBehaviour::OnInit() {
    auto& em = Core::EventManager::GetInstance();
    m_awarenessMsgHandle = em.LinkMsg(Core::Event::MsgGrabCancel, [this](const Core::Msg&) {
        CancelGrab();
    });
}

bool EnemyBehaviour::CanSeePlayer(const Core::Vec3& playerPos, bool isFlashlightOn) const {
    Core::Vec3 toPlayer = playerPos - GetPosition();
    toPlayer.y = 0.0f;
    float dist = glm::length(toPlayer);

    float maxSight = isFlashlightOn ? m_sightRangeLit : m_sightRangeUnlit;
    if (dist > maxSight || dist < 0.001f) return false;

    float yawRad = glm::radians(m_yaw);
    Core::Vec3 forward(std::sin(yawRad), 0.0f, std::cos(yawRad));
    Core::Vec3 dir = toPlayer / dist;

    float cosAngle = glm::dot(forward, dir);
    float cosCone = std::cos(glm::radians(m_coneAngleDeg * 0.5f));

    return cosAngle >= cosCone;
}

bool EnemyBehaviour::CanHearPlayer(const Core::Vec3& playerPos, bool isRunning) const {
    if (!isRunning) return false;
    Core::Vec3 toPlayer = playerPos - GetPosition();
    toPlayer.y = 0.0f;
    return glm::length(toPlayer) <= m_hearingRange;
}

void EnemyBehaviour::InitiateGrab(Actor* target) {
    if (!target || !target->IsAlive() || m_isGrabbing || m_isKnockedDown) return;

    m_isGrabbing = true;
    m_specialAttacked = true;
    m_aggressive = true;

    auto& em = Core::EventManager::GetInstance();
    em.SendMsg(Core::Event::MsgGrabStarted, this, target);
    em.SendMsg(Core::Event::MsgSetGrabSyncState, this,
               (void*)m_grabAnimName.c_str(), m_grabAnimName.size());
}

void EnemyBehaviour::CancelGrab() {
    if (!m_isGrabbing) return;
    m_isGrabbing = false;
    m_specialAttacked = false;
    m_specialAttackedRelease = true;

    auto& em = Core::EventManager::GetInstance();
    em.SendMsg(Core::Event::MsgGrabFinished, this);
}

void EnemyBehaviour::TakeDamage(float amount, const Core::Vec3& hitDir) {
    Actor::TakeDamage(amount, hitDir);

    m_beingHit = true;
    m_hitStunTimer = 0.4f;
    m_aggressive = true;
    m_isAlerted = true;

    // Knockdown threshold logic
    if (m_health <= 30.0f && !m_isKnockedDown) {
        m_isKnockedDown = true;
        m_knockDownFlinch = true;
        m_knockdownTimer = 4.0f; // 4 seconds vulnerability on floor
        m_walking = false;
        if (m_isGrabbing) {
            CancelGrab();
        }
    }
}

void EnemyBehaviour::OnEvent(const Core::Msg& msg) {
    if (msg.id.name == Core::Event::MsgGrabCancel) {
        CancelGrab();
    }
}

void EnemyBehaviour::OnUpdate(float dt) {
    if (!IsAlive()) {
        m_walking = false;
        m_aggressive = false;
        m_isKnockedDown = true;
        return;
    }

    // Hit stun recovery
    if (m_beingHit) {
        m_hitStunTimer -= dt;
        if (m_hitStunTimer <= 0.0f) {
            m_beingHit = false;
            m_knockDownFlinch = false;
        }
    }

    // Knockdown recovery timer
    if (m_isKnockedDown) {
        m_knockdownTimer -= dt;
        if (m_knockdownTimer <= 0.0f) {
            m_isKnockedDown = false;
            m_hasLanded = true;
        }
        return;
    }

    // Attack cooldown tick
    if (m_attackCooldown > 0.0f) {
        m_attackCooldown -= dt;
    }

    UpdateAI(dt);
}

void EnemyBehaviour::UpdateAI(float dt) {
    if (!m_target || !m_target->IsAlive()) {
        m_walking = false;
        m_aggressive = false;
        return;
    }

    Core::Vec3 toTarget = m_target->GetPosition() - GetPosition();
    toTarget.y = 0.0f;
    float dist = glm::length(toTarget);

    // Sensory alert check
    if (!m_isAlerted) {
        if (CanSeePlayer(m_target->GetPosition(), true) ||
            CanHearPlayer(m_target->GetPosition(), true)) {
            m_isAlerted = true;
            m_aggressive = true;
        } else {
            m_walking = false;
            return;
        }
    }

    // Turn toward target
    if (dist > 0.001f) {
        Core::Vec3 dir = toTarget / dist;
        float targetYawDeg = glm::degrees(std::atan2(dir.x, dir.z));
        float diff = targetYawDeg - m_yaw;
        while (diff > 180.0f) diff -= 360.0f;
        while (diff < -180.0f) diff += 360.0f;

        if (std::abs(diff) > 45.0f) {
            m_turnBig = true;
            m_turnSmall = false;
        } else if (std::abs(diff) > 5.0f) {
            m_turnSmall = true;
            m_turnBig = false;
        } else {
            m_turnSmall = false;
            m_turnBig = false;
        }

        m_yaw += std::clamp(diff, -120.0f * dt, 120.0f * dt);
        SetYaw(m_yaw);
    }

    // In attack range
    if (dist <= m_attackRadius) {
        m_walking = false;
        if (m_attackCooldown <= 0.0f && !m_isGrabbing) {
            // Decide attack type: Grab or Melee Strike
            if (m_archetype == EnemyArchetype::PsychoNurse ||
                m_archetype == EnemyArchetype::StraightJacket ||
                m_archetype == EnemyArchetype::TwoBack) {
                InitiateGrab(m_target);
            } else {
                m_lightAttacked = true;
                m_target->TakeDamage(15.0f, glm::normalize(toTarget));
            }
            m_attackCooldown = 2.0f;
        }
    } else if (!m_isGrabbing) {
        // Move toward player
        m_walking = true;
        m_lightAttacked = false;
        m_heavyAttacked = false;

        float moveSpeed = (m_archetype == EnemyArchetype::Butcher ||
                           m_archetype == EnemyArchetype::CalibanBoss) ? 1.0f : 1.8f;

        Core::Vec3 moveDir = toTarget / dist;
        SetPosition(GetPosition() + moveDir * moveSpeed * dt);
    }
}

} // namespace Actor
} // namespace SHO
