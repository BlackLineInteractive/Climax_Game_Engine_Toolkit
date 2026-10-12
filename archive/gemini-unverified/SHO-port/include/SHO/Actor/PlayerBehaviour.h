#pragma once

#include "SHO/Actor/Actor.h"
#include "SHO/Actor/CharacterController.h"
#include "SHO/Core/EventIds.h"
#include "SHO/Core/EventManager.h"
#include <array>
#include <string>
#include <vector>

namespace SHO {
namespace Actor {

// Damage & death reaction types recovered from SLES_551.47 @ 0x0068F210-0x0068F2A8
enum class PlayerDamageReaction : uint32_t {
    None = 0,
    BeenHitGroundLarge      = 1, // 0x0068F210
    BeenHitGroundBackLarge  = 2, // 0x0068F228
    BeenHitGroundLeftLarge  = 3, // 0x0068F240
    BeenHitGroundRightLarge = 4, // 0x0068F258
    BeenKilledSmall         = 5, // 0x0068F270
    BeenKilledExplosion     = 6, // 0x0068F280
    BeenKilledLarge         = 7, // 0x0068F298
    Dead                    = 8  // 0x0068F2A8
};

// Reconstructed Travis Grady entity from SLES_551.47
// Size: 7,152 bytes (0x1BF0)
// Factory @ 0x00136CC8, Ctor @ 0x0012C3B0, Dtor @ 0x0012D250
// Tick @ 0x00133BB0, Msg Dispatcher @ 0x001335B8
// Vtables: 0x0068E588, 0x0068E6C8, 0x0068E548, 0x0068E530, 0x0068E6A8
class PlayerBehaviour : public Actor {
public:
    static constexpr size_t SLES_CLASS_SIZE   = 7152;
    static constexpr uint32_t SLES_FACTORY    = 0x00136CC8;
    static constexpr uint32_t SLES_CTOR       = 0x0012C3B0;
    static constexpr uint32_t SLES_DTOR       = 0x0012D250;
    static constexpr uint32_t SLES_DISPATCH   = 0x001335B8;
    static constexpr uint32_t SLES_TICK       = 0x00133BB0;

    PlayerBehaviour();
    virtual ~PlayerBehaviour();

    void OnInit() override;
    void OnUpdate(float dt) override;
    void OnEvent(const Core::Msg& msg) override;

    // Pad inputs (bitmask of Core::PadButton)
    void SetInputMask(Core::PadButton buttons) { m_buttons = buttons; }
    void SetAnalogStick(float stickX, float stickY) { m_stickX = stickX; m_stickY = stickY; }

    // Authentic Climax animation state parameters recovered from FUN_0012c3b0
    bool HasBlocked() const { return m_hasBlocked; }
    bool HasRun() const { return m_hasRun; }
    bool HasInteracted() const { return m_hasInteracted; }
    bool HasAttacked() const { return m_hasAttacked; }
    bool HasStanced() const { return m_hasStanced; }
    bool HasReloaded() const { return m_hasReloaded; }
    bool HasToggled() const { return m_hasToggled; }

    bool HasBlockedReleased() const { return m_hasBlockedReleased; }
    bool HasRunReleased() const { return m_hasRunReleased; }
    bool HasInteractedReleased() const { return m_hasInteractedReleased; }
    bool HasAttackedReleased() const { return m_hasAttackedReleased; }
    bool HasStancedReleased() const { return m_hasStancedReleased; }
    bool HasReloadedReleased() const { return m_hasReloadedReleased; }
    bool HasToggledReleased() const { return m_hasToggledReleased; }

    int32_t GetHeldWeaponClass() const { return m_heldWeaponClass; }
    bool IsLockedToEnemy() const { return m_lockedToEnemy; }
    bool IsFinishingMoveEnabled() const { return m_enableFinishingMove; }
    bool HasBullets() const { return m_hasBullets; }
    bool HasAmmo() const { return m_hasAmmo; }

    bool IsFatigued() const { return m_isFatigued; }
    bool IsInjured() const { return m_health <= 25.0f; }

    float GetStrafeForwardValue() const { return m_strafeForwardValue; }
    float GetStrafeSidewaysValue() const { return m_strafeSidewaysValue; }
    float GetForwardMovementValue() const { return m_forwardMovementValue; }

    // Pickups (FUN_0012c3b0 @ 0x15d0-0x15ec)
    bool IsPickupFloor() const { return m_pickupFloor; }
    bool IsPickupTable() const { return m_pickupTable; }
    bool IsPickupDone() const { return m_pickupDone; }
    bool IsPickupNoAnim() const { return m_pickupNoAnim; }

    // Radio & Screen static
    bool IsRadioStaticEnabled() const { return m_radioStaticEnabled; }
    bool IsScreenStaticEnabled() const { return m_screenStaticEnabled; }

    // Torch (PlayerTorchOn / PlayerTorchOff)
    bool IsTorchOn() const { return m_torchOn; }
    void SendTorchToggle();

    // Combat & Control switches
    bool IsControlEnabled() const { return m_controlEnabled; }
    bool IsCombatEnabled() const { return m_combatEnabled; }

    // Movement & Stamina constants (IEEE 754 from FUN_0012c3b0)
    float GetWalkSpeed() const { return m_walkSpeed; }
    float GetRunSpeed() const { return m_runSpeed; }
    float GetStamina() const { return m_stamina; }
    float GetMaxStamina() const { return m_maxStamina; }
    float GetTurnSpeed() const { return m_turnSpeed; }

    // Grapple synchronization & TNF struggle (FUN_001335b8)
    bool IsGrappled() const { return m_isGrappled; }
    const std::string& GetGrabSyncAnim() const { return m_grabSyncAnim; }
    bool IsTNFDisplayEnabled() const { return m_tnfDisplayEnabled; }
    uint32_t GetTNFButtonPrompt() const { return m_tnfButtonPrompt; }
    float GetTNFStruggleProgress() const { return m_tnfStruggleProgress; }

    // Mirror transition state
    bool IsInMirror() const { return m_inMirror; }
    bool IsOtherworld() const { return m_isOtherworld; }

    // Damage reactions
    PlayerDamageReaction GetDamageReaction() const { return m_damageReaction; }

    CharacterController& GetController() { return m_controller; }
    void NotifyCameraTransition();

    // Genuine weapon rack: 32 slots recovered from FUN_00139350 @ 0x00139350
    struct WeaponSlot {
        uint32_t itemDefId = 0;
        int32_t  durability = 100;
        int32_t  ammoInClip = 0;
        bool     broken = false;
    };
    const std::array<WeaponSlot, 32>& GetWeaponRack() const { return m_weaponRack; }
    int32_t GetActiveWeaponSlot() const { return m_activeWeaponSlot; }

private:
    void HandleMovement(float dt);
    void UpdateStateVariables();
    void DispatchSLESMessage(const std::string& msgName, void* data = nullptr);

    CharacterController m_controller;

    // Pad inputs
    Core::PadButton m_buttons = Core::PadButton::None;
    float m_stickX = 0.0f;
    float m_stickY = 0.0f;

    // Subsystem switches
    bool m_controlEnabled = true;
    bool m_combatEnabled = true;
    bool m_renderEnabled = true;
    bool m_torchOn = false;
    bool m_radioStaticEnabled = false;
    bool m_screenStaticEnabled = false;

    // 29 Climax animation attributes from FUN_0012c3b0
    bool m_hasBlocked = false;             // 0x1540
    bool m_hasRun = false;                 // 0x1548
    bool m_hasInteracted = false;          // 0x1558
    bool m_hasAttacked = false;            // 0x1560
    bool m_hasStanced = false;             // 0x1568
    bool m_hasReloaded = false;            // 0x1570
    bool m_hasToggled = false;             // 0x1578
    bool m_hasBlockedReleased = true;      // 0x1580
    bool m_hasRunReleased = true;          // 0x1588
    bool m_hasInteractedReleased = true;   // 0x1598
    bool m_hasAttackedReleased = true;     // 0x15a0
    bool m_hasStancedReleased = true;      // 0x15a8
    bool m_hasReloadedReleased = true;     // 0x15b0
    bool m_hasToggledReleased = true;      // 0x15b8
    int32_t m_heldWeaponClass = 0;         // 0x15c0
    bool m_lockedToEnemy = false;          // 0x15c8
    bool m_pickupFloor = false;            // 0x15d0
    bool m_pickupTable = false;            // 0x15d8
    bool m_pickupDone = false;             // 0x15e0
    bool m_pickupNoAnim = false;           // 0x15e8
    bool m_isFatigued = false;             // 0x15f0
    bool m_isInjured = false;              // 0x15f8
    float m_strafeForwardValue = 0.0f;     // 0x1600
    float m_strafeSidewaysValue = 0.0f;    // 0x1608
    bool m_enableFinishingMove = false;    // 0x1610
    float m_forwardMovementValue = 0.0f;   // 0x1618
    bool m_hasBullets = true;              // 0x1620
    bool m_hasAmmo = true;                 // 0x1628

    // Weapon Rack (FUN_00139350)
    std::array<WeaponSlot, 32> m_weaponRack{};
    int32_t m_activeWeaponSlot = 0;

    // Grapple & TNF struggle (FUN_001335b8)
    bool m_isGrappled = false;
    std::string m_grabSyncAnim;
    bool m_tnfDisplayEnabled = false;
    uint32_t m_tnfButtonPrompt = 0;
    float m_tnfStruggleProgress = 0.0f;

    // Mirror transition (EnterMirror / ExitMirror)
    bool m_inMirror = false;
    bool m_isOtherworld = false;

    // Damage reactions (0x0068F210-0x0068F2A8)
    PlayerDamageReaction m_damageReaction = PlayerDamageReaction::None;

    // Authentic IEEE-754 constants from FUN_0012c3b0
    float m_maxStamina = 100.0f;          // 0x42c80000 @ 0x19bc
    float m_stamina = 100.0f;
    float m_fatigueThreshold = 64.0f;     // 0x42800000 @ 0x19c0
    float m_staminaRecoveryRate = 3.0f;   // 0x40400000 @ 0x1bd0
    float m_walkSpeed = 1.6f;
    float m_runSpeed = 4.0f;              // 0x40800000 @ 0x19b0
    float m_turnSpeed = 180.0f;           // 0x43340000 @ 0x1a0c

    // Direction latch for fixed-camera cut transitions
    bool       m_isDirectionLatched = false;
    Core::Vec3 m_latchedMoveDir = Core::Vec3(0.0f);

    // Event link handles
    std::vector<uint32_t> m_eventHandles;
};

} // namespace Actor
} // namespace SHO
