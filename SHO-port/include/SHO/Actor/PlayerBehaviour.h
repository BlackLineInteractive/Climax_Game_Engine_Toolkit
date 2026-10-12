#pragma once

#include "SHO/Actor/Actor.h"
#include "SHO/Actor/CharacterController.h"
#include "SHO/Core/EventIds.h"
#include "SHO/Core/EventManager.h"
#include <vector>

namespace SHO {
namespace Actor {

// Reconstructed Travis Grady entity from SLES_551.47
// Factory @ 0x00136CC8, Ctor @ 0x0012C3B0 (7152 bytes)
// Vtable: slot 3 @ 0x0012D250 (teardown), slot 5 @ 0x00137230 (per-frame), slot 7 @ 0x00137210 (attributes)
class PlayerBehaviour : public Actor {
public:
    PlayerBehaviour();
    virtual ~PlayerBehaviour();

    void OnInit() override;
    void OnUpdate(float dt) override;
    void OnEvent(const Core::Msg& msg) override;

    // Pad inputs (bitmask of Core::PadButton)
    void SetInputMask(Core::PadButton buttons) { m_buttons = buttons; }
    void SetAnalogStick(float stickX, float stickY) { m_stickX = stickX; m_stickY = stickY; }

    // Real state flags from FUN_0012c3b0
    bool HasRun() const { return m_hasRun; }
    bool HasAttacked() const { return m_hasAttacked; }
    bool HasStanced() const { return m_hasStanced; }
    bool HasReloaded() const { return m_hasReloaded; }
    bool HasToggled() const { return m_hasToggled; }
    bool IsFatigued() const { return m_isFatigued; }
    bool IsInjured() const { return m_health <= 25.0f; }
    bool IsLockedToEnemy() const { return m_lockedToEnemy; }
    bool IsFinishingMoveEnabled() const { return m_enableFinishingMove; }

    // Radio static (binary on/off from RadioStaticEnable / RadioStaticDisable)
    bool IsRadioStaticEnabled() const { return m_radioStaticEnabled; }
    bool IsScreenStaticEnabled() const { return m_screenStaticEnabled; }

    // Torch state (linked to PlayerTorchOn / PlayerTorchOff)
    bool IsTorchOn() const { return m_torchOn; }
    void SendTorchToggle();

    // Combat & Control switches (EnablePlayerControl / EnablePlayerCombat)
    bool IsControlEnabled() const { return m_controlEnabled; }
    bool IsCombatEnabled() const { return m_combatEnabled; }

    // Movement & Stamina
    float GetWalkSpeed() const { return m_walkSpeed; }
    float GetRunSpeed() const { return m_runSpeed; }
    float GetStamina() const { return m_stamina; }
    float GetMaxStamina() const { return m_maxStamina; }

    // Grapple synchronization (MsgGrabStarted / MsgSetGrabSyncState)
    bool IsGrappled() const { return m_isGrappled; }
    const std::string& GetGrabSyncAnim() const { return m_grabSyncAnim; }

    CharacterController& GetController() { return m_controller; }

    // Camera cut world direction latch
    void NotifyCameraTransition();

private:
    void HandleMovement(float dt);
    void UpdateStateVariables();

    CharacterController m_controller;

    // Inputs
    Core::PadButton m_buttons = Core::PadButton::None;
    float m_stickX = 0.0f;
    float m_stickY = 0.0f;

    // Subsystem enable switches from SLES messages
    bool m_controlEnabled = true;
    bool m_combatEnabled = true;
    bool m_renderEnabled = true;
    bool m_torchOn = false;
    bool m_radioStaticEnabled = false;
    bool m_screenStaticEnabled = false;

    // Real state flags from FUN_0012c3b0
    bool m_hasBlocked = false;
    bool m_hasRun = false;
    bool m_hasInteracted = false;
    bool m_hasAttacked = false;
    bool m_hasStanced = false;
    bool m_hasReloaded = false;
    bool m_hasToggled = false;
    bool m_isFatigued = false;
    bool m_lockedToEnemy = false;
    bool m_enableFinishingMove = false;

    // Grapple state
    bool m_isGrappled = false;
    std::string m_grabSyncAnim;

    // Ground truth constants from FUN_0012c3b0 (IEEE 754 floats)
    float m_maxStamina = 100.0f;         // 0x42c80000 @ 0x19bc
    float m_stamina = 100.0f;
    float m_fatigueThreshold = 64.0f;    // 0x42800000 @ 0x19c0
    float m_staminaRecoveryRate = 3.0f;  // 0x40400000 @ 0x1bd0
    float m_walkSpeed = 1.6f;
    float m_runSpeed = 4.0f;            // 0x40800000 @ 0x19b0
    float m_turnSpeed = 180.0f;          // 0x43340000 @ 0x1a0c

    // Direction latch: holds world move vector across camera transitions until stick is recentered
    bool       m_isDirectionLatched = false;
    Core::Vec3 m_latchedMoveDir = Core::Vec3(0.0f);

    // Event link handles
    std::vector<uint32_t> m_eventHandles;
};

} // namespace Actor
} // namespace SHO
