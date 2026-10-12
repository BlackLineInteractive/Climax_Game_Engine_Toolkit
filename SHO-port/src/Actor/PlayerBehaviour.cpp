#include "SHO/Actor/PlayerBehaviour.h"
#include "SHO/Camera/CameraManager.h"
#include <algorithm>
#include <cmath>

namespace SHO {
namespace Actor {

// Reconstructed Travis Grady entity from SLES_551.47
// Factory @ 0x00136CC8, Ctor @ 0x0012C3B0 (7,152 bytes)
PlayerBehaviour::PlayerBehaviour() {
    m_className = "CPlayerBehaviour";
    m_name = "Travis";
    m_health = 100.0f;
    m_maxHealth = 100.0f;

    // Initialize weapon rack with default bare fists in slot 0
    m_weaponRack[0].itemDefId = 0; // Bare fists
    m_weaponRack[0].durability = 1000;
    m_weaponRack[0].ammoInClip = 0;
    m_weaponRack[0].broken = false;
    m_activeWeaponSlot = 0;

    OnInit();
}

PlayerBehaviour::~PlayerBehaviour() {
    // Teardown unregistering matching FUN_0012d250 @ 0x0012d250
    auto& em = Core::EventManager::GetInstance();
    for (uint32_t handle : m_eventHandles) {
        em.UnlinkMsg(handle);
    }
    m_eventHandles.clear();
}

void PlayerBehaviour::OnInit() {
    auto& em = Core::EventManager::GetInstance();

    auto link = [&](const char* eventName) {
        uint32_t h = em.LinkMsg(eventName, [this](const Core::Msg& msg) {
            OnEvent(msg);
        });
        m_eventHandles.push_back(h);
    };

    // System switches recovered from FUN_0012c3b0
    link(Core::Event::PlayerTorchOn);
    link(Core::Event::PlayerTorchOff);
    link(Core::Event::RadioStaticEnable);
    link(Core::Event::RadioStaticDisable);
    link(Core::Event::ScreenStaticEnable);
    link(Core::Event::ScreenStaticDisable);
    link(Core::Event::EnablePlayerControl);
    link(Core::Event::EnablePlayerCombat);
    link(Core::Event::DisablePlayerCombat);
    link(Core::Event::EnablePlayerRender);

    // Grapple events
    link(Core::Event::MsgGrabStarted);
    link(Core::Event::MsgGrabFinished);
    link(Core::Event::MsgGrabCancel);
    link(Core::Event::MsgSetGrabSyncState);

    // Authentic SLES_551.47 message dispatcher events recovered from FUN_001335b8
    link("SetTNFButtonPrompt");     // 0x0068E398
    link("EnableTNFDisplay");       // 0x0068E3B0
    link("GiveDamageNoFlinch");     // 0x0068E3C8
    link("FootStep");               // 0x0068E3E0
    link("DoPickup");               // 0x0068E3F0
    link("DamageEnemyTarget");      // 0x0068E400
    link("FireGun");                // 0x0068E418
    link("ThrowWeapon");            // 0x0068E420
    link("PlayerReload");           // 0x0068E430
    link("WpnSwapNow");             // 0x0068E440
    link("Snd.Fire");               // 0x0068E450
    link("Snd.Reload");             // 0x0068E460
    link("Snd.Empty");              // 0x0068E470
    link("ReduceStamina");          // 0x0068E480
    link("BulkCharge");             // 0x0068E4A0
    link("Crawl");                  // 0x0068E4B0
    link("EnterMirror");            // 0x0068E4B8
    link("ExitMirror");             // 0x0068E4C8
    link("Idle_AllReleased");       // 0x0068E4D8
    link("Fatigued_AllReleased");   // 0x0068E4F0
    link("Pained_AllReleased");     // 0x0068E508
    link("WeaponSwitch35");         // 0x0068E520

    // Mirror reflection events (FUN_00139350 @ 0x00139350)
    link(Core::Event::Characters__ReflectionEnable);
    link(Core::Event::Characters__ReflectionDisable);
    link(Core::Event::Characters__MsgReflectReposTorch);
}

void PlayerBehaviour::OnEvent(const Core::Msg& msg) {
    const std::string& name = msg.id.name;

    // --- Subsystem switches ---
    if (name == Core::Event::PlayerTorchOn) {
        m_torchOn = true;
    } else if (name == Core::Event::PlayerTorchOff) {
        m_torchOn = false;
    } else if (name == Core::Event::RadioStaticEnable) {
        m_radioStaticEnabled = true;
    } else if (name == Core::Event::RadioStaticDisable) {
        m_radioStaticEnabled = false;
    } else if (name == Core::Event::ScreenStaticEnable) {
        m_screenStaticEnabled = true;
    } else if (name == Core::Event::ScreenStaticDisable) {
        m_screenStaticEnabled = false;
    } else if (name == Core::Event::EnablePlayerControl) {
        bool enable = true;
        if (msg.data && msg.dataSize >= sizeof(bool)) {
            enable = *static_cast<const bool*>(msg.data);
        }
        m_controlEnabled = enable;
    } else if (name == Core::Event::EnablePlayerCombat) {
        m_combatEnabled = true;
    } else if (name == Core::Event::DisablePlayerCombat) {
        m_combatEnabled = false;
    } else if (name == Core::Event::EnablePlayerRender) {
        bool enable = true;
        if (msg.data && msg.dataSize >= sizeof(bool)) {
            enable = *static_cast<const bool*>(msg.data);
        }
        m_renderEnabled = enable;
    }
    // --- Grapple states ---
    else if (name == Core::Event::MsgGrabStarted) {
        m_isGrappled = true;
        m_controlEnabled = false;
        m_combatEnabled = false;
        m_tnfDisplayEnabled = true;
        m_tnfStruggleProgress = 0.0f;
    } else if (name == Core::Event::MsgGrabFinished || name == Core::Event::MsgGrabCancel) {
        m_isGrappled = false;
        m_controlEnabled = true;
        m_combatEnabled = true;
        m_tnfDisplayEnabled = false;
        m_grabSyncAnim.clear();
    } else if (name == Core::Event::MsgSetGrabSyncState) {
        if (msg.data && msg.dataSize > 0) {
            m_grabSyncAnim = std::string(static_cast<const char*>(msg.data), msg.dataSize);
        }
    }
    // --- Authentic SLES_551.47 message handlers (FUN_001335b8) ---
    else if (name == "SetTNFButtonPrompt") {
        if (msg.data && msg.dataSize >= sizeof(uint32_t)) {
            m_tnfButtonPrompt = *static_cast<const uint32_t*>(msg.data);
        }
    } else if (name == "EnableTNFDisplay") {
        m_tnfDisplayEnabled = true;
    } else if (name == "GiveDamageNoFlinch") {
        float dmg = 10.0f;
        if (msg.data && msg.dataSize >= sizeof(float)) {
            dmg = *static_cast<const float*>(msg.data);
        }
        TakeDamage(dmg, Core::Vec3(0.0f));
    } else if (name == "DoPickup") {
        m_pickupDone = true;
    } else if (name == "DamageEnemyTarget") {
        // Attack execution dispatched to enemy in lock-on cone
    } else if (name == "FireGun") {
        auto& slot = m_weaponRack[m_activeWeaponSlot];
        if (slot.ammoInClip > 0) {
            slot.ammoInClip--;
            m_hasBullets = (slot.ammoInClip > 0);
        }
    } else if (name == "PlayerReload") {
        auto& slot = m_weaponRack[m_activeWeaponSlot];
        slot.ammoInClip = 8; // standard handgun clip
        m_hasBullets = true;
        m_hasReloaded = true;
    } else if (name == "WpnSwapNow" || name == "WeaponSwitch35") {
        int32_t newSlot = (m_activeWeaponSlot + 1) % 32;
        m_activeWeaponSlot = newSlot;
        m_heldWeaponClass = m_weaponRack[newSlot].itemDefId;
    } else if (name == "ReduceStamina") {
        float reduction = 15.0f;
        if (msg.data && msg.dataSize >= sizeof(float)) {
            reduction = *static_cast<const float*>(msg.data);
        }
        m_stamina = std::max(0.0f, m_stamina - reduction);
    } else if (name == "EnterMirror") {
        m_inMirror = true;
        m_isOtherworld = !m_isOtherworld;
    } else if (name == "ExitMirror") {
        m_inMirror = false;
    } else if (name == "Idle_AllReleased") {
        m_hasBlockedReleased = true;
        m_hasRunReleased = true;
        m_hasAttackedReleased = true;
        m_hasStancedReleased = true;
    } else if (name == "Fatigued_AllReleased") {
        m_isFatigued = true;
    } else if (name == "Pained_AllReleased") {
        m_isInjured = true;
    }
}

void PlayerBehaviour::SendTorchToggle() {
    auto& em = Core::EventManager::GetInstance();
    if (m_torchOn) {
        em.SendMsg(Core::Event::PlayerTorchOff, this);
    } else {
        em.SendMsg(Core::Event::PlayerTorchOn, this);
    }
}

void PlayerBehaviour::NotifyCameraTransition() {
    if (std::abs(m_stickX) > 0.15f || std::abs(m_stickY) > 0.15f) {
        m_isDirectionLatched = true;
    }
}

void PlayerBehaviour::OnUpdate(float dt) {
    if (!IsAlive()) {
        m_damageReaction = PlayerDamageReaction::Dead;
        return;
    }

    UpdateStateVariables();

    if (m_controlEnabled && !m_isGrappled) {
        HandleMovement(dt);
    }

    // Stamina recovery when not running (FUN_0012c3b0 recovery rate = 3.0f @ 0x1bd0)
    if (!m_hasRun) {
        m_stamina = std::min(m_maxStamina, m_stamina + m_staminaRecoveryRate * dt);
        if (m_stamina > m_fatigueThreshold) {
            m_isFatigued = false;
        }
    } else {
        // Stamina drain while running (4.0f per second)
        m_stamina = std::max(0.0f, m_stamina - 4.0f * dt);
        if (m_stamina <= 0.0f) {
            m_isFatigued = true;
        }
    }

    // Injury state evaluation
    m_isInjured = (m_health <= 25.0f);
}

void PlayerBehaviour::UpdateStateVariables() {
    using namespace Core;

    bool prevBlocked = m_hasBlocked;
    bool prevRun     = m_hasRun;
    bool prevAttacked= m_hasAttacked;
    bool prevStanced = m_hasStanced;

    m_hasBlocked    = HasFlag(m_buttons, PadButton::Square);
    m_hasRun        = HasFlag(m_buttons, PadButton::Square) && (std::abs(m_stickX) > 0.2f || std::abs(m_stickY) > 0.2f) && !m_isFatigued;
    m_hasInteracted = HasFlag(m_buttons, PadButton::Cross);
    m_hasAttacked   = HasFlag(m_buttons, PadButton::Cross) && m_hasStanced;
    m_hasStanced    = HasFlag(m_buttons, PadButton::R1);
    m_hasReloaded   = HasFlag(m_buttons, PadButton::Circle) && m_hasStanced;
    m_hasToggled    = HasFlag(m_buttons, PadButton::Circle) && !m_hasStanced;

    // Track release edges
    m_hasBlockedReleased  = prevBlocked && !m_hasBlocked;
    m_hasRunReleased      = prevRun && !m_hasRun;
    m_hasAttackedReleased = prevAttacked && !m_hasAttacked;
    m_hasStancedReleased  = prevStanced && !m_hasStanced;

    // Torch button toggle edge (Circle when not in combat stance)
    if (m_hasToggled && !prevStanced) {
        SendTorchToggle();
    }
}

void PlayerBehaviour::HandleMovement(float dt) {
    auto& camMgr = Camera::CameraManager::GetInstance();
    Camera::BaseCamera* activeCam = camMgr.GetActiveCamera();

    float inputMag = std::sqrt(m_stickX * m_stickX + m_stickY * m_stickY);
    if (inputMag < 0.15f) {
        m_isDirectionLatched = false;
        m_latchedMoveDir = Core::Vec3(0.0f);
        m_forwardMovementValue = 0.0f;
        m_strafeForwardValue = 0.0f;
        m_strafeSidewaysValue = 0.0f;
        return;
    }

    Core::Vec3 moveDir(0.0f);

    if (m_isDirectionLatched) {
        moveDir = m_latchedMoveDir;
    } else if (activeCam) {
        Core::Vec3 camFwd = activeCam->GetForward();
        camFwd.y = 0.0f;
        if (glm::length(camFwd) > 0.001f) {
            camFwd = glm::normalize(camFwd);
        }

        Core::Vec3 camRight = activeCam->GetRight();
        camRight.y = 0.0f;
        if (glm::length(camRight) > 0.001f) {
            camRight = glm::normalize(camRight);
        }

        moveDir = camFwd * (-m_stickY) + camRight * m_stickX;
        if (glm::length(moveDir) > 0.001f) {
            moveDir = glm::normalize(moveDir);
            m_latchedMoveDir = moveDir;
        }
    } else {
        moveDir = Core::Vec3(m_stickX, 0.0f, -m_stickY);
        if (glm::length(moveDir) > 0.001f) {
            moveDir = glm::normalize(moveDir);
        }
    }

    // Speed calculation using authentic SLES constants
    float targetSpeed = m_hasRun ? m_runSpeed : m_walkSpeed;
    if (m_isFatigued) {
        targetSpeed = m_walkSpeed * 0.75f;
    }

    Core::Vec3 movement = moveDir * targetSpeed * dt;
    SetPosition(GetPosition() + movement);

    m_forwardMovementValue = targetSpeed;
    m_strafeForwardValue = -m_stickY;
    m_strafeSidewaysValue = m_stickX;

    // Face movement direction
    if (glm::length(moveDir) > 0.001f && !m_hasStanced) {
        float targetYaw = std::atan2(moveDir.x, moveDir.z);
        SetYaw(glm::degrees(targetYaw));
    }
}

} // namespace Actor
} // namespace SHO
