#include "SHO/Environment/PostProcessingFSE.h"

namespace SHO {
namespace Environment {

FullScreenEffect::FullScreenEffect() {
    m_className = "cBlackOutFSE";
}

ThreatController::ThreatController() {
    m_className = "cThreatController";
    // 0x00168678 default parameter floats
    m_param0 = 1.0f; // 0x54
    m_param1 = 1.0f; // 0x58
    m_param2 = 1.0f; // 0x60
    m_param3 = 1.0f; // 0x64
    m_enterMsg = "effectEnterMirror"; // 0x74
    m_exitMsg = "animExitMirror";     // 0x7c

    OnInit();
}

ThreatController::~ThreatController() {
    auto& em = Core::EventManager::GetInstance();
    if (m_enterHandle) em.UnlinkMsg(m_enterHandle);
    if (m_exitHandle) em.UnlinkMsg(m_exitHandle);
}

void ThreatController::OnInit() {
    auto& em = Core::EventManager::GetInstance();
    if (!m_enterMsg.empty()) {
        m_enterHandle = em.LinkMsg(m_enterMsg, [this](const Core::Msg&) {
            OnEnter();
        });
    }
    if (!m_exitMsg.empty()) {
        m_exitHandle = em.LinkMsg(m_exitMsg, [this](const Core::Msg&) {
            OnExit();
        });
    }
}

void ThreatController::SetEnterEffectMessage(const std::string& msg) {
    m_enterMsg = msg;
    auto& em = Core::EventManager::GetInstance();
    if (m_enterHandle) em.UnlinkMsg(m_enterHandle);
    if (!msg.empty()) {
        m_enterHandle = em.LinkMsg(msg, [this](const Core::Msg&) {
            OnEnter();
        });
    }
}

void ThreatController::SetExitEffectMessage(const std::string& msg) {
    m_exitMsg = msg;
    auto& em = Core::EventManager::GetInstance();
    if (m_exitHandle) em.UnlinkMsg(m_exitHandle);
    if (!msg.empty()) {
        m_exitHandle = em.LinkMsg(msg, [this](const Core::Msg&) {
            OnExit();
        });
    }
}

void ThreatController::OnEnter() {
    m_triggered = true;
    Core::EventManager::GetInstance().SendMsg("ThreatFilterOn", this);
}

void ThreatController::OnExit() {
    m_triggered = false;
    Core::EventManager::GetInstance().SendMsg("ThreatFilterOff", this);
}

RainManager::RainManager() {
    m_className = "CFullScreenRain";
}

void RainManager::OnUpdate(float) {
    // Dynamic rain simulation
}

} // namespace Environment
} // namespace SHO
