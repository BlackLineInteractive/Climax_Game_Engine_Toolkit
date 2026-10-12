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
    CalibanBoss,
    FlaurosBoss
};

// Reconstructed CEnemyBehaviour from SLES_551.47
// Registry @ 0x006B4B78, Factory @ 0x0012B630, Ctor @ 0x00127718 (size: 11,664 bytes / 0x2D90)
// Vtables: 0x0068CFE8, 0x0068D108, 0x0068CF90, 0x0068D128, 0x0068D3E8, 0x0068CFA8
class EnemyBehaviour : public Actor {
public:
    static constexpr size_t SLES_CLASS_SIZE   = 11664;
    static constexpr uint32_t SLES_FACTORY    = 0x0012B630;
    static constexpr uint32_t SLES_CTOR       = 0x00127718;

    EnemyBehaviour(EnemyArchetype archetype = EnemyArchetype::PsychoNurse);
    virtual ~EnemyBehaviour();

    void OnInit() override;
    void OnUpdate(float dt) override;
    void OnEvent(const Core::Msg& msg) override;
    void TakeDamage(float amount, const Core::Vec3& hitDir) override;

    EnemyArchetype GetArchetype() const { return m_archetype; }

    // Sensory evaluation based on sho_enemies.json & SLES constants
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
    Actor* GetTarget() const { return m_target; }

    // 24 Authentic animation state parameters recovered from FUN_00127718 @ 0x127718
    bool IsHeavyAttacked() const { return m_heavyAttacked; }
    bool IsLightAttacked() const { return m_lightAttacked; }
    bool IsSpecialAttacked() const { return m_specialAttacked; }
    bool IsBlocking() const { return m_blocking; }
    bool IsWalking() const { return m_walking; }
    bool IsSideSteppingLeft() const { return m_sideSteppingLeft; }
    bool IsSideSteppingRight() const { return m_sideSteppingRight; }
    bool IsSteppingBack() const { return m_steppingBack; }
    bool IsBeingHit() const { return m_beingHit; }
    bool IsTurnSmall() const { return m_turnSmall; }
    bool IsTurnBig() const { return m_turnBig; }
    bool IsKnockDown() const { return m_isKnockedDown; }
    bool IsKnockDownFlinch() const { return m_knockDownFlinch; }
    bool IsSpiritShielded() const { return m_spiritShielded; }
    bool IsAggressive() const { return m_aggressive; }

    // Global active monster counter tracking (DAT_00338848)
    static int32_t GetGlobalActiveMonsterCount() { return s_activeMonsterCount; }

private:
    void ConfigureArchetype(EnemyArchetype archetype);
    void UpdateAI(float dt);

    static int32_t s_activeMonsterCount;

    EnemyArchetype m_archetype = EnemyArchetype::PsychoNurse;
    Actor* m_target = nullptr;
    CharacterController m_controller;

    // Sensory attributes from sho_enemies.json
    float m_coneAngleDeg = 45.0f;       // index 14
    float m_sightRangeUnlit = 3.0f;     // index 15
    float m_sightRangeLit = 7.5f;       // index 17
    float m_hearingRange = 15.0f;       // index 19
    float m_attackRadius = 1.2f;

    // 24 Authentic Climax Engine parameters from FUN_00127718
    bool m_heavyAttacked = false;       // 0xa3f
    bool m_lightAttacked = false;       // 0xa41
    bool m_specialAttacked = false;     // 0xa43
    bool m_blocking = false;            // 0xa45
    bool m_walking = false;             // 0xa47
    bool m_sideSteppingLeft = false;    // 0xa49
    bool m_sideSteppingRight = false;   // 0xa4b
    bool m_steppingBack = false;        // 0xa4d
    bool m_beingHit = false;            // 0xa4f
    bool m_walkingRelease = false;      // 0xa51
    bool m_heavyAttackedRelease = false;// 0xa53
    bool m_lightAttackedRelease = false;// 0xa55
    bool m_specialAttackedRelease = false; // 0xa57
    bool m_blockingRelease = false;     // 0xa59
    bool m_sideSteppingLeftRelease = false;  // 0xa5b
    bool m_sideSteppingRightRelease = false; // 0xa5d
    bool m_steppingBackRelease = false;      // 0xa5f
    bool m_hasLanded = false;           // 0xa61
    bool m_spiritShielded = false;      // 0xa63
    bool m_turnBig = false;             // 0xa65
    bool m_turnSmall = false;           // 0xa67
    bool m_isKnockedDown = false;       // 0xa69
    bool m_knockDownFlinch = false;     // 0xa6b
    bool m_aggressive = false;          // 0xa6d

    // State machine timers
    bool m_isAlerted = false;
    bool m_isGrabbing = false;
    float m_knockdownTimer = 0.0f;
    float m_attackCooldown = 0.0f;
    float m_hitStunTimer = 0.0f;

    std::string m_grabAnimName = "Grabbed_Nurse";
    uint32_t m_awarenessMsgHandle = 0;
};

} // namespace Actor
} // namespace SHO
