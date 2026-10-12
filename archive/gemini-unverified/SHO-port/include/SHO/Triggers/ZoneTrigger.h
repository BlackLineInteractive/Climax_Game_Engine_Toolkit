#pragma once

#include "SHO/Triggers/TriggerBase.h"
#include "SHO/Core/EventManager.h"
#include "SHO/Core/EventIds.h"
#include <string>

namespace SHO {
namespace Triggers {

// ─────────────────────────────────────────────────────────────────────────────
// ZoneTrigger
// Decompiled from SLES_551.47: Factory @ 0x0017C5F8, Ctor @ 0x0017BBA8,
// Attributes @ 0x0017BF80, Trigger Event Dispatch @ 0x0017C828 / 0x0017C1D0 (size 368)
// ─────────────────────────────────────────────────────────────────────────────
class ZoneTrigger : public TriggerBase {
public:
    ZoneTrigger();
    virtual ~ZoneTrigger() = default;

    // ── Fields strictly matching SLES_551.47 FUN_0017bf80 ──
    const std::string& GetTargetZone() const { return m_targetZone; }          // 0xf0
    void SetTargetZone(const std::string& zone) { m_targetZone = zone; }

    const std::string& GetSourceZone() const { return m_sourceZone; }
    void SetSourceZone(const std::string& zone) { m_sourceZone = zone; }

    const std::string& GetTargetSpawn() const { return m_targetSpawn; }        // 0xf4
    void SetTargetSpawn(const std::string& spawn) { m_targetSpawn = spawn; }
    void SetTargetSpawnName(const std::string& spawn) { m_targetSpawn = spawn; }

    int GetTransitionType() const { return m_transitionType; }                 // 0x11c
    void SetTransitionType(int type) { m_transitionType = type; }

    bool IsActive() const { return m_active; }                                 // 0x110
    void SetActiveState(bool active) { m_active = active; }

    int GetSoundOpen() const { return m_soundOpen; }                           // 0x150
    void SetSoundOpen(int snd) { m_soundOpen = snd; }

    int GetSoundClose() const { return m_soundClose; }                         // 0x154
    void SetSoundClose(int snd) { m_soundClose = snd; }

    int GetLockedPrompt() const { return m_lockedPrompt; }                     // 0x13c
    void SetLockedPrompt(int prompt) { m_lockedPrompt = prompt; }

    const std::string& GetRequiredKey() const { return m_requiredKey; }        // 0x160
    void SetRequiredKey(const std::string& key) { m_requiredKey = key; }

    const std::string& GetExaminedText() const { return m_examinedText; }      // 0x164
    void SetExaminedText(const std::string& txt) { m_examinedText = txt; }

    const std::string& GetOtherworldZone() const { return m_otherworldZone; }  // 0x168
    void SetOtherworldZone(const std::string& zone) { m_otherworldZone = zone; }

    bool IsLocked() const { return m_isLocked; }                               // 0x114
    void SetLocked(bool locked) { m_isLocked = locked; }

    const std::string& GetContainerId() const { return m_containerId; }        // 0x16c
    void SetContainerId(const std::string& cid) { m_containerId = cid; }

    bool IsOneWay() const { return m_isOneWay; }                               // 0x148
    void SetOneWay(bool oneWay) { m_isOneWay = oneWay; }

    void SetBounds(const Core::AABB& bounds) { m_bounds = bounds; }
    const Core::AABB& GetBounds() const { return m_bounds; }

    // FUN_0017c1d0 & FUN_0017c828: Zone transition trigger evaluation & message dispatch
    bool CheckTrigger(const Core::Vec3& actorPos, const Core::Vec3& prevActorPos) override;
    bool TryUnlockWithKey();
    void TriggerTransition();

private:
    std::string m_sourceZone;
    std::string m_targetZone;       // 0xf0
    std::string m_targetSpawn;      // 0xf4
    int         m_transitionType = 0; // 0x11c
    bool        m_active = true;    // 0x110
    int         m_soundOpen = 0;    // 0x150
    int         m_soundClose = 0;   // 0x154
    int         m_lockedPrompt = 0; // 0x13c
    std::string m_requiredKey;      // 0x160
    std::string m_examinedText;     // 0x164
    std::string m_otherworldZone;   // 0x168
    bool        m_isLocked = false; // 0x114
    std::string m_containerId;      // 0x16c
    bool        m_isOneWay = false; // 0x148
    Core::AABB  m_bounds;
};

} // namespace Triggers
} // namespace SHO
