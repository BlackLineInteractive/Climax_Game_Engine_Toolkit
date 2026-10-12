#pragma once

#include "SHO/Actor/Actor.h"
#include "SHO/Actor/CharacterController.h"
#include "SHO/Core/EventIds.h"
#include "SHO/Core/EventManager.h"
#include <string>

namespace SHO {
namespace Actor {

enum class EnemyArchetype {
    PsychoNurse,
    PsychoNurseSan,
    Carrion,
    StraightJacket,
    StraightJacketAlt,
    StraightJacketBoss,
    Ariel,
    TwoBack,
    TwoBackNoTNF,
    Butcher,
    Momma,
    InvisibleMan,
    SadDaddyTongue,
    CalibanBoss
};

// Reconstructed CEnemyBehaviour from SLES_551.47
// Registry @ 0x006B4B78, Factory @ 0x0012B630, Ctor @ 0x00127718 (size 11664)
// All 14 standard monsters in SH.ARC instance this class with their respective property rows.
class EnemyBehaviour : public Actor {
public:
    EnemyBehaviour(EnemyArchetype archetype = EnemyArchetype::PsychoNurse);
    virtual ~EnemyBehaviour();

    void OnInit() override;
    void OnUpdate(float dt) override;
    void OnEvent(const Core::Msg& msg) override;
    void TakeDamage(float amount, const Core::Vec3& hitDir) override;

    EnemyArchetype GetArchetype() const { return m_archetype; }

    // Sensory evaluation based on sho_enemies.json
    bool CanSeePlayer(const Core::Vec3& playerPos, bool isFlashlightOn) const;
    bool CanHearPlayer(const Core::Vec3& playerPos, bool isRunning) const;

    // Grab & synchronization (MsgGrabStarted / MsgSetGrabSyncState)
    bool IsGrabbing() const { return m_isGrabbing; }
    void InitiateGrab(Actor* target);
    void CancelGrab();
    const std::string& GetGrabAnimName() const { return m_grabAnimName; }

    // Knockdown state
    bool IsKnockedDown() const { return m_isKnockedDown; }

    // Target assignment
    void SetTarget(Actor* target) { m_target = target; }

private:
    void ConfigureArchetype(EnemyArchetype archetype);

    EnemyArchetype m_archetype = EnemyArchetype::PsychoNurse;
    Actor* m_target = nullptr;
    CharacterController m_controller;

    // Sensory attributes from sho_enemies.json (indices 14, 15, 17, 18, 19, 22)
    float m_coneAngleDeg = 45.0f;       // index 14
    float m_sightRangeUnlit = 3.0f;     // index 15
    float m_sightRangeLit = 7.5f;       // index 17
    float m_hearingRange = 15.0f;       // index 19
    float m_attackRadius = 1.2f;

    // State flags
    bool m_isAlerted = false;
    bool m_isGrabbing = false;
    bool m_isKnockedDown = false;
    float m_knockdownTimer = 0.0f;
    float m_attackCooldown = 0.0f;

    std::string m_grabAnimName = "Grabbed_2Back"; // Property 68 from archive
    uint32_t m_awarenessMsgHandle = 0;
};

} // namespace Actor
} // namespace SHO
