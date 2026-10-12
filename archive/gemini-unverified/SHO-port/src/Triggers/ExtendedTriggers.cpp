#include "SHO/Triggers/ExtendedTriggers.h"
#include <cmath>

namespace SHO {
namespace Triggers {

// ─────────────────────────────────────────────────────────────────────────────
// ButtonBoxTrigger
// ─────────────────────────────────────────────────────────────────────────────
ButtonBoxTrigger::ButtonBoxTrigger() {
    m_className = "ButtonBoxTrigger";
}

ButtonBoxTrigger::~ButtonBoxTrigger() {
    auto& em = Core::EventManager::GetInstance();
    if (m_enableHandle) em.UnlinkMsg(m_enableHandle);
    if (m_disableHandle) em.UnlinkMsg(m_disableHandle);
}

void ButtonBoxTrigger::SetBounds(const Core::Vec3& min, const Core::Vec3& max) {
    Core::Vec3 center = (min + max) * 0.5f;
    Core::Vec3 size = max - min;
    SetPosition(center);

    // FUN_001695d8 / FUN_00168e60: construct OBB matrix (0x60) and invert to (0xa0) via FUN_0024bdb8
    m_transform = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), size);
    m_inverseTransform = glm::inverse(m_transform);
}

void ButtonBoxTrigger::SetEnableMessage(const std::string& msg) {
    m_enableMsg = msg;
    if (!msg.empty()) {
        m_enableHandle = Core::EventManager::GetInstance().LinkMsg(msg, [this](const Core::Msg&) {
            m_active = true; // 0xe0 = 1 in FUN_00169448 line 18
        });
    }
}

void ButtonBoxTrigger::SetDisableMessage(const std::string& msg) {
    m_disableMsg = msg;
    if (!msg.empty()) {
        m_disableHandle = Core::EventManager::GetInstance().LinkMsg(msg, [this](const Core::Msg&) {
            m_active = false; // 0xe0 = 0 in FUN_00169448 line 21
        });
    }
}

bool ButtonBoxTrigger::CheckTrigger(const Core::Vec3& currentPos, const Core::Vec3&) {
    if (!m_active) {
        m_playerInside = false;
        return false;
    }

    // FUN_00169448: transform world position to local space via inverse matrix
    glm::vec4 localPos = m_inverseTransform * glm::vec4(currentPos.x, currentPos.y, currentPos.z, 1.0f);

    // Check unit cube [-0.5, 0.5]^3
    bool inside = (localPos.x >= -0.5f && localPos.x <= 0.5f &&
                   localPos.y >= -0.5f && localPos.y <= 0.5f &&
                   localPos.z >= -0.5f && localPos.z <= 0.5f);

    m_playerInside = inside;
    return inside;
}

void ButtonBoxTrigger::Interact() {
    if (m_playerInside && m_active && !m_targetEvent.empty()) {
        // FUN_00169448 line 45: invoke vtable slot 0x44 to send target message
        Core::EventManager::GetInstance().SendMsg(m_targetEvent, this);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AreaTriggerBox
// ─────────────────────────────────────────────────────────────────────────────
AreaTriggerBox::AreaTriggerBox() {
    m_className = "AreaTriggerBox";
}

AreaTriggerBox::~AreaTriggerBox() = default;

void AreaTriggerBox::SetBoxExtents(const Core::Vec3& center, const Core::Vec3& halfExtents) {
    SetPosition(center);
    Core::Vec3 fullSize = halfExtents * 2.0f;
    // FUN_00170650 / FUN_001712c8: build direct transform (0x50) and inverse (0x90) via FUN_0024bdb8
    m_transform = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), fullSize);
    m_inverseTransform = glm::inverse(m_transform);
}

bool AreaTriggerBox::IsPointInside(const Core::Vec3& worldPos) const {
    // FUN_00171b28: transform world position into local box unit space via inverse matrix at 0x90
    glm::vec4 local = m_inverseTransform * glm::vec4(worldPos.x, worldPos.y, worldPos.z, 1.0f);
    return (local.x > -0.5f && local.x < 0.5f &&
            local.y > -0.5f && local.y < 0.5f &&
            local.z > -0.5f && local.z < 0.5f);
}

bool AreaTriggerBox::CheckTrigger(const Core::Vec3& currentPos, const Core::Vec3&) {
    if (!m_active || (m_triggerOnce && m_hasTriggered)) {
        return false;
    }

    bool inside = IsPointInside(currentPos);

    if (inside && !m_wasInside) {
        m_wasInside = true;
        m_hasTriggered = true;
        if (!m_onEnterEvent.empty()) {
            Core::EventManager::GetInstance().SendMsg(m_onEnterEvent, this);
        }
        return true;
    } else if (!inside && m_wasInside) {
        m_wasInside = false;
        if (!m_onExitEvent.empty()) {
            Core::EventManager::GetInstance().SendMsg(m_onExitEvent, this);
        }
    }

    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// AreaTriggerBattleManager
// ─────────────────────────────────────────────────────────────────────────────
BattleManagerTrigger::BattleManagerTrigger() {
    m_className = "AreaTriggerBattleManager";
}

bool BattleManagerTrigger::CheckTrigger(const Core::Vec3&, const Core::Vec3&) {
    return m_active;
}

void BattleManagerTrigger::OnEnemyKilled() {
    if (!m_active || m_cleared) return;

    m_remainingEnemies--;
    if (m_remainingEnemies <= 0) {
        m_cleared = true;
        m_active = false;
        if (!m_clearEvent.empty()) {
            Core::EventManager::GetInstance().SendMsg(m_clearEvent, this);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CFadeTrigger & CScreenShakeTrigger
// ─────────────────────────────────────────────────────────────────────────────
ScreenEffectTrigger::ScreenEffectTrigger() {
    m_className = "CFadeTrigger";
}

bool ScreenEffectTrigger::CheckTrigger(const Core::Vec3&, const Core::Vec3&) {
    return false;
}

void ScreenEffectTrigger::TriggerFade(bool fadeOut) {
    auto& em = Core::EventManager::GetInstance();
    if (fadeOut) {
        em.SendMsg("ScreenFadeOut", this, &m_duration, sizeof(m_duration));
    } else {
        em.SendMsg("ScreenFadeIn", this, &m_duration, sizeof(m_duration));
    }
}

void ScreenEffectTrigger::TriggerShake(float intensity, float duration) {
    m_shakeIntensity = intensity;
    m_duration = duration;
    Core::EventManager::GetInstance().SendMsg("ScreenShake", this, &m_shakeIntensity, sizeof(m_shakeIntensity));
}

EventCounter::EventCounter() {
    m_className = "CCounter";
}

void EventCounter::Increment() {
    m_currentCount++;
    if (m_currentCount >= m_targetCount && !m_targetEvent.empty()) {
        Core::EventManager::GetInstance().SendMsg(m_targetEvent, this);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CPadRumbleTrigger (FUN_0016d7f8)
// ─────────────────────────────────────────────────────────────────────────────
PadRumbleTrigger::PadRumbleTrigger() {
    m_className = "CPadRumbleTrigger";
}

void PadRumbleTrigger::ExecuteRumble() {
    struct RumblePayload {
        int largeMotor;
        int smallMotor;
        float duration;
    };
    RumblePayload payload{ m_largeMotor, m_smallMotor, m_duration };
    Core::EventManager::GetInstance().SendMsg("PadRumble", this, &payload, sizeof(payload));

    if (!m_triggerEvent.empty()) {
        Core::EventManager::GetInstance().SendMsg(m_triggerEvent, this);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CScreenShakeTrigger (FUN_0016ea78)
// ─────────────────────────────────────────────────────────────────────────────
ScreenShakeTrigger::ScreenShakeTrigger() {
    m_className = "CScreenShakeTrigger";
}

void ScreenShakeTrigger::ExecuteShake() {
    struct ShakePayload {
        float amplitude;
        float frequency;
        float duration;
        float decayRate;
    };
    ShakePayload payload{ m_amplitude, m_frequency, m_duration, m_decayRate };
    Core::EventManager::GetInstance().SendMsg("ScreenShake", this, &payload, sizeof(payload));

    if (!m_triggerEvent.empty()) {
        Core::EventManager::GetInstance().SendMsg(m_triggerEvent, this);
    }
}

} // namespace Triggers
} // namespace SHO
