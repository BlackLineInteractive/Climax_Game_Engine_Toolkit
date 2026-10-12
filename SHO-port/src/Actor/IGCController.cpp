#include "SHO/Actor/IGCController.h"

namespace SHO {
namespace Actor {

FMAController::FMAController() {
    m_className = "CFMAController";
}

FMAController::~FMAController() {
    if (m_linkHandle) {
        Core::EventManager::GetInstance().UnlinkMsg(m_linkHandle);
    }
}

void FMAController::OnInit() {
    if (!m_triggerEvent.empty()) {
        m_linkHandle = Core::EventManager::GetInstance().LinkMsg(m_triggerEvent, [this](const Core::Msg&) {
            ExecuteTrigger();
        });
    }
}

void FMAController::ExecuteTrigger() {
    if (!m_forcedAnimState.empty()) {
        Core::EventManager::GetInstance().SendMsg("PlayerForceAnimState", this, (void*)m_forcedAnimState.c_str());
    }
}

CutsceneTrigger::CutsceneTrigger() {
    m_className = "CIGCTrigger";
}

void CutsceneTrigger::StartCutscene() {
    m_isPlaying = true;
    Core::EventManager::GetInstance().SendMsg("DisablePlayerControl");
    Core::EventManager::GetInstance().SendMsg("CutsceneStarted", this, (void*)m_cutsceneName.c_str());
}

void CutsceneTrigger::EndCutscene() {
    m_isPlaying = false;
    Core::EventManager::GetInstance().SendMsg("EnablePlayerControl");
    Core::EventManager::GetInstance().SendMsg("CutsceneEnded", this, (void*)m_cutsceneName.c_str());
}

} // namespace Actor
} // namespace SHO
