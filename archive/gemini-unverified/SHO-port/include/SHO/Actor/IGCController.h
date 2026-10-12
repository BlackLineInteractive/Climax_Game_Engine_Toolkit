#pragma once

#include "SHO/Core/Behaviour.h"
#include "SHO/Core/EventManager.h"
#include <string>

namespace SHO {
namespace Actor {

// Translated from CFMAController (0x0017D498, size 80) & CFMAPlayerAnim (0x0017D8E8, size 92)
class FMAController : public Core::Behaviour {
public:
    FMAController();
    virtual ~FMAController();

    void OnInit();
    void SetTriggerEvent(const std::string& evt) { m_triggerEvent = evt; }
    void SetForcedAnimState(const std::string& animState) { m_forcedAnimState = animState; }

    const std::string& GetTriggerEvent() const { return m_triggerEvent; }
    const std::string& GetForcedAnimState() const { return m_forcedAnimState; }

    void ExecuteTrigger();

private:
    std::string m_triggerEvent;     // Property 0
    std::string m_forcedAnimState;  // Property 1 (e.g. EnterMirror, ExitMirror, GetUpFromFall)
    uint32_t m_linkHandle = 0;
};

// Translated from CIGCTrigger (0x00187D50, size 152) & IGCBorder (0x00183490, size 88)
class CutsceneTrigger : public Core::Behaviour {
public:
    CutsceneTrigger();
    virtual ~CutsceneTrigger() = default;

    void SetCutsceneName(const std::string& name) { m_cutsceneName = name; }
    const std::string& GetCutsceneName() const { return m_cutsceneName; }

    void StartCutscene();
    void EndCutscene();
    bool IsPlaying() const { return m_isPlaying; }

private:
    std::string m_cutsceneName;
    bool m_isPlaying = false;
};

} // namespace Actor
} // namespace SHO
