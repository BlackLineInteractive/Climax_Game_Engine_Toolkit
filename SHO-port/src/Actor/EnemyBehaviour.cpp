#include "SHO/Actor/EnemyBehaviour.h"
#include <algorithm>
#include <cmath>

namespace SHO {
namespace Actor {

EnemyBehaviour::EnemyBehaviour(EnemyArchetype archetype) {
    m_className = "CEnemyBehaviour";
    ConfigureArchetype(archetype);
    OnInit();
}

EnemyBehaviour::~EnemyBehaviour() {
    auto& em = Core::EventManager::GetInstance();
    if (m_awarenessMsgHandle) {
        em.UnlinkMsg(m_awarenessMsgHandle);
    }
}

void EnemyBehaviour::ConfigureArchetype(EnemyArchetype archetype) {
    m_archetype = archetype;

    // Ground truth health & sensory constants from sho_enemies.json / ENEMIES.md
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
        m_name = "StraightJacket";
        m_health = 125.0f;
        m_maxHealth = 125.0f;
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
        m_sightRangeLit = 8.0f;
        m_hearingRange = 18.0f;
        m_attackRadius = 1.2f;
        m_grabAnimName = "Grabbed_Ariel";
        break;

    case EnemyArchetype::TwoBack:
    case EnemyArchetype::TwoBackNoTNF:
    case EnemyArchetype::StraightJacketBoss:
        m_name = "2Back";
        m_health = 200.0f;
        m_maxHealth = 200.0f;
        m_coneAngleDeg = 45.0f;
        m_sightRangeUnlit = 3.0f;
        m_sightRangeLit = 7.5f;
        m_hearingRange = 15.0f;
        m_attackRadius = 1.5f;
        m_grabAnimName = "Grabbed_2Back";
        break;

    case EnemyArchetype::Butcher:
    case EnemyArchetype::Momma:
        m_name = "Butcher";
        m_health = 375.0f;
        m_maxHealth = 375.0f;
        m_coneAngleDeg = 45.0f;
        m_sightRangeUnlit = 80.0f;
        m_sightRangeLit = 100.0f;
        m_hearingRange = 80.0f;
        m_attackRadius = 2.2f;
        m_grabAnimName = "Grabbed_Butcher";
        break;

    case EnemyArchetype::CalibanBoss:
        m_name = "CalibanBoss";
        m_health = 450.0f;
        m_maxHealth = 450.0f;
        m_coneAngleDeg = 180.0f;
        m_sightRangeUnlit = 1000.0f;
        m_sightRangeLit = 1000.0f;
        m_hearingRange = 1000.0f;
        m_attackRadius = 3.5f;
        m_grabAnimName = "Grabbed_Caliban";
        break;
    }
}

void EnemyBehaviour::OnInit() {
    auto& em = Core::EventManager::GetInstance();
    m_awarenessMsgHandle = em.LinkMsg(Core::Event::MsgSetAllEnemiesAware, [this](const Core::Msg& msg) {
        OnEvent(msg);
    });
}

void EnemyBehaviour::OnEvent(const Core::Msg& msg) {
    if (msg.id.name == Core::Event::MsgSetAllEnemiesAware) {
        m_isAlerted = true;
    }
}

bool EnemyBehaviour::CanSeePlayer(const Core::Vec3& playerPos, bool isFlashlightOn) const {
    Core::Vec3 eyePos = GetPosition() + Core::Vec3(0, 1.4f, 0);
    Core::Vec3 toPlayer = playerPos - eyePos;
    float dist = glm::length(toPlayer);

    float maxRange = isFlashlightOn ? m_sightRangeLit : m_sightRangeUnlit;
    if (dist > maxRange || dist <= 1e-4f) return false;

    Core::Vec3 lookDir = GetForward();
    float cosAngle = glm::dot(lookDir, toPlayer / dist);
    float cosThreshold = std::cos(glm::radians(m_coneAngleDeg));

    return cosAngle >= cosThreshold;
}

bool EnemyBehaviour::CanHearPlayer(const Core::Vec3& playerPos, bool isRunning) const {
    float dist = glm::distance(GetPosition(), playerPos);
    // Running makes Travis audible across the monster's full hearing range
    float hearLimit = isRunning ? m_hearingRange : (m_hearingRange * 0.25f);
    return dist <= hearLimit;
}

void EnemyBehaviour::TakeDamage(float amount, const Core::Vec3& hitDir) {
    Actor::TakeDamage(amount, hitDir);

    if (!IsAlive()) {
        CancelGrab();
        m_isKnockedDown = false;
        return;
    }

    // Heavy hit knocks down enemy
    if (amount >= 30.0f && !m_isKnockedDown) {
        m_isKnockedDown = true;
        m_knockdownTimer = 4.5f;
        CancelGrab();
    }
}

void EnemyBehaviour::InitiateGrab(Actor* target) {
    if (!target || !target->IsAlive() || m_isKnockedDown) return;

    m_isGrabbing = true;
    auto& em = Core::EventManager::GetInstance();
    em.SendMsg(Core::Event::MsgGrabStarted, this);
    em.SendMsg(Core::Event::MsgSetGrabSyncState, this, (void*)m_grabAnimName.c_str());
}

void EnemyBehaviour::CancelGrab() {
    if (m_isGrabbing) {
        m_isGrabbing = false;
        Core::EventManager::GetInstance().SendMsg(Core::Event::MsgGrabCancel, this);
    }
}

void EnemyBehaviour::OnUpdate(float dt) {
    if (!IsAlive()) return;

    if (m_attackCooldown > 0.0f) {
        m_attackCooldown -= dt;
    }

    if (m_isKnockedDown) {
        m_knockdownTimer -= dt;
        if (m_knockdownTimer <= 0.0f) {
            m_isKnockedDown = false;
        }
        return;
    }

    if (m_isGrabbing) {
        return;
    }

    if (!m_target || !m_target->IsAlive()) return;

    Core::Vec3 myPos = GetPosition();
    Core::Vec3 targetPos = m_target->GetPosition();
    Core::Vec3 toTarget = targetPos - myPos;
    float dist = glm::length(toTarget);

    if (dist <= m_attackRadius) {
        if (m_attackCooldown <= 0.0f) {
            m_target->TakeDamage(15.0f, glm::normalize(toTarget));
            m_attackCooldown = 1.5f;
        }
    } else if (dist <= m_hearingRange) {
        Core::Vec3 dir = glm::normalize(toTarget);
        float targetYaw = glm::degrees(std::atan2(dir.x, dir.z));
        SetYaw(targetYaw);

        Core::Vec3 displacement = dir * (1.2f * dt);
        Core::Vec3 newPos = m_controller.Move(myPos, displacement, dt);
        SetPosition(newPos);
    }
}

} // namespace Actor
} // namespace SHO
