#include "SHO/Core/MessageRelay.h"

namespace SHO {
namespace Core {

MessageRelay::MessageRelay() {
    m_className = "CMessageRelay";
    m_active = true; // 0xd8 = 1 in FUN_0016cec8
}

MessageRelay::~MessageRelay() {
    auto& em = EventManager::GetInstance();
    if (m_inputHandle) em.UnlinkMsg(m_inputHandle);
    if (m_enableHandle) em.UnlinkMsg(m_enableHandle);
    if (m_disableHandle) em.UnlinkMsg(m_disableHandle);
}

void MessageRelay::OnInit() {
    auto& em = EventManager::GetInstance();
    if (!m_inputEvent.empty()) {
        m_inputHandle = em.LinkMsg(m_inputEvent, [this](const Msg&) {
            TriggerRelay();
        });
    }
}

void MessageRelay::SetEnableMessage(const std::string& msg) {
    m_enableMsg = msg;
    if (!msg.empty()) {
        m_enableHandle = EventManager::GetInstance().LinkMsg(msg, [this](const Msg&) {
            m_active = true;
        });
    }
}

void MessageRelay::SetDisableMessage(const std::string& msg) {
    m_disableMsg = msg;
    if (!msg.empty()) {
        m_disableHandle = EventManager::GetInstance().LinkMsg(msg, [this](const Msg&) {
            m_active = false;
        });
    }
}

void MessageRelay::AddRelayedTarget(const std::string& targetEvt, float delaySec) {
    if (m_targets.size() < 10) { // FUN_0016cec8 has 10 target slots
        m_targets.push_back({ targetEvt, delaySec });
    }
}

void MessageRelay::TriggerRelay() {
    if (!m_active) return;

    auto& em = EventManager::GetInstance();
    for (const auto& target : m_targets) {
        if (target.targetEvent.empty()) continue;

        if (target.delaySec <= 0.0f) {
            em.SendMsg(target.targetEvent, this);
        } else {
            m_pendingQueue.push_back({ target.targetEvent, target.delaySec });
        }
    }
}

void MessageRelay::OnUpdate(float dt) {
    if (m_pendingQueue.empty()) return;

    auto& em = EventManager::GetInstance();
    for (auto it = m_pendingQueue.begin(); it != m_pendingQueue.end(); ) {
        it->remainingTime -= dt;
        if (it->remainingTime <= 0.0f) {
            em.SendMsg(it->eventName, this);
            it = m_pendingQueue.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace Core
} // namespace SHO
