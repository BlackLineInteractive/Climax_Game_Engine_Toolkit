#include "SHO/Actor/PlayerBehaviour.h"
#include "SHO/Camera/CameraManager.h"
#include <algorithm>
#include <cmath>

namespace SHO {
namespace Actor {

PlayerBehaviour::PlayerBehaviour() {
    m_className = "CPlayerBehaviour";
    m_name = "Travis";
    m_health = 100.0f;
    m_maxHealth = 100.0f;

    OnInit();
}

PlayerBehaviour::~PlayerBehaviour() {
    auto& em = Core::EventManager::GetInstance();
    for (uint32_t handle : m_eventHandles) {
        em.UnlinkMsg(handle);
    }
    m_eventHandles.clear();
}

void PlayerBehaviour::OnInit() {
    auto& em = Core::EventManager::GetInstance();

    // Link real SLES_551.47 messages recovered from FUN_0012c3b0 / MESSAGES.md
    auto link = [&](const char* eventName) {
        uint32_t h = em.LinkMsg(eventName, [this](const Core::Msg& msg) {
            OnEvent(msg);
        });
        m_eventHandles.push_back(h);
    };

    link(Core::Event::PlayerTorchOn);
    link(Core::Event::PlayerTorchOff);
    link(Core::Event::RadioStaticEnable);
    link(Core::Event::RadioStaticDisable);
    link(Core::Event::ScreenStaticEnable);
    link(Core::Event::ScreenStaticDisable);
    link(Core::Event::EnablePlayerControl);
    link(Core::Event::DisablePlayerCombat);
    link(Core::Event::EnablePlayerCombat);
    link(Core::Event::EnablePlayerRender);
    link(Core::Event::MsgGrabStarted);
    link(Core::Event::MsgGrabFinished);
    link(Core::Event::MsgGrabCancel);
    link(Core::Event::MsgSetGrabSyncState);
}

void PlayerBehaviour::OnEvent(const Core::Msg& msg) {
    if (msg.id.name == Core::Event::PlayerTorchOn) {
        m_torchOn = true;
    } else if (msg.id.name == Core::Event::PlayerTorchOff) {
        m_torchOn = false;
    } else if (msg.id.name == Core::Event::RadioStaticEnable) {
        m_radioStaticEnabled = true;
    } else if (msg.id.name == Core::Event::RadioStaticDisable) {
        m_radioStaticEnabled = false;
    } else if (msg.id.name == Core::Event::ScreenStaticEnable) {
        m_screenStaticEnabled = true;
    } else if (msg.id.name == Core::Event::ScreenStaticDisable) {
        m_screenStaticEnabled = false;
    } else if (msg.id.name == Core::Event::EnablePlayerControl) {
        m_controlEnabled = true;
    } else if (msg.id.name == Core::Event::EnablePlayerCombat) {
        m_combatEnabled = true;
    } else if (msg.id.name == Core::Event::DisablePlayerCombat) {
        m_combatEnabled = false;
    } else if (msg.id.name == Core::Event::MsgGrabStarted) {
        m_isGrappled = true;
    } else if (msg.id.name == Core::Event::MsgGrabFinished || msg.id.name == Core::Event::MsgGrabCancel) {
        m_isGrappled = false;
        m_grabSyncAnim.clear();
    } else if (msg.id.name == Core::Event::MsgSetGrabSyncState) {
        if (msg.data) {
            m_grabSyncAnim = static_cast<const char*>(msg.data);
        }
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
    // Direction latch from TODO.md §4c & FUN_0012e2a0:
    // When camera angle transitions during movement, latch current world move vector
    // so Travis continues along the same physical trajectory until analog stick is released.
    if (std::abs(m_stickX) > 0.15f || std::abs(m_stickY) > 0.15f) {
        m_isDirectionLatched = true;
    }
}

void PlayerBehaviour::UpdateStateVariables() {
    float stickMag = std::sqrt(m_stickX * m_stickX + m_stickY * m_stickY);
    m_hasRun = Core::HasFlag(m_buttons, Core::PadButton::Square) && stickMag > 0.3f && !m_isFatigued;
    m_hasStanced = Core::HasFlag(m_buttons, Core::PadButton::R1);
    m_hasAttacked = Core::HasFlag(m_buttons, Core::PadButton::Cross) && m_hasStanced;
    m_hasInteracted = Core::HasFlag(m_buttons, Core::PadButton::Cross) && !m_hasStanced;
    m_hasReloaded = Core::HasFlag(m_buttons, Core::PadButton::Square) && m_hasStanced;
    m_hasToggled = Core::HasFlag(m_buttons, Core::PadButton::Triangle);
}

void PlayerBehaviour::HandleMovement(float dt) {
    if (!IsAlive() || !m_controlEnabled || m_isGrappled) {
        return;
    }

    UpdateStateVariables();

    float stickMag = std::sqrt(m_stickX * m_stickX + m_stickY * m_stickY);
    if (stickMag < 0.15f) {
        m_isDirectionLatched = false;
        m_latchedMoveDir = Core::Vec3(0.0f);

        // Ground truth stamina recovery from FUN_0012c3b0 / FUN_0012e2a0:
        // Standing recovers faster (m_staminaRecoveryRate = 3.0f * dt scaling)
        m_stamina = std::min(m_maxStamina, m_stamina + m_staminaRecoveryRate * 8.0f * dt);
        if (m_stamina >= m_fatigueThreshold) {
            m_isFatigued = false;
        }
        return;
    }

    Core::Vec3 moveDir(0.0f);

    if (m_isDirectionLatched && glm::length(m_latchedMoveDir) > 0.1f) {
        moveDir = m_latchedMoveDir;
    } else {
        // Camera-relative input calculation (CamMsgReceiveTrackingDetails basis)
        auto& camMgr = Camera::CameraManager::GetInstance();
        auto* activeCam = camMgr.GetActiveCamera();

        Core::Vec3 camFwd(0, 0, 1);
        Core::Vec3 camRight(1, 0, 0);

        if (activeCam) {
            camFwd = activeCam->GetForward();
            camFwd.y = 0.0f;
            if (glm::length(camFwd) > 1e-4f) camFwd = glm::normalize(camFwd);

            camRight = activeCam->GetRight();
            camRight.y = 0.0f;
            if (glm::length(camRight) > 1e-4f) camRight = glm::normalize(camRight);
        }

        moveDir = camRight * m_stickX + camFwd * m_stickY;
        if (glm::length(moveDir) > 1e-4f) {
            moveDir = glm::normalize(moveDir);
        }
        m_latchedMoveDir = moveDir;
    }

    // Stamina calculation from FUN_0012e2a0
    if (m_hasRun) {
        m_stamina = std::max(0.0f, m_stamina - 15.0f * dt);
        if (m_stamina <= 0.0f) {
            m_isFatigued = true;
            m_hasRun = false;
        }
    } else {
        m_stamina = std::min(m_maxStamina, m_stamina + m_staminaRecoveryRate * 4.0f * dt);
        if (m_stamina >= m_fatigueThreshold) {
            m_isFatigued = false;
        }
    }

    float speed = m_walkSpeed;
    if (m_hasRun) {
        speed = m_runSpeed;
    } else if (m_isFatigued) {
        speed = m_walkSpeed * 0.65f;
    }

    Core::Vec3 displacement = moveDir * (speed * dt);

    // Orientation yaw update
    float targetYaw = glm::degrees(std::atan2(moveDir.x, moveDir.z));
    float yawDiff = targetYaw - m_yaw;
    while (yawDiff > 180.0f)  yawDiff -= 360.0f;
    while (yawDiff < -180.0f) yawDiff += 360.0f;

    float maxTurn = m_turnSpeed * dt;
    yawDiff = std::clamp(yawDiff, -maxTurn, maxTurn);
    SetYaw(m_yaw + yawDiff);

    // Apply movement with character controller collision
    Core::Vec3 currentPos = GetPosition();
    Core::Vec3 newPos = m_controller.Move(currentPos, displacement, dt);
    SetPosition(newPos);
}

void PlayerBehaviour::OnUpdate(float dt) {
    HandleMovement(dt);
}

} // namespace Actor
} // namespace SHO
