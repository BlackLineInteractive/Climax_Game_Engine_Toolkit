#include "SHO/Triggers/ZoneTrigger.h"
#include "SHO/Core/EventManager.h"
#include "SHO/Core/EventIds.h"
#include "SHO/Inventory/InventoryManager.h"

namespace SHO {
namespace Triggers {

ZoneTrigger::ZoneTrigger() {
    m_className = "ZoneTrigger";
    // 0x0017BBA8: Default initialization
    m_active = true;         // 0x110 = 0 (unlocked by default unless 0x114 is set)
    m_isLocked = false;       // 0x114 = 0
    m_transitionType = 1;     // 0x11c = 1
    m_isOneWay = false;       // 0x148 = 0
}

bool ZoneTrigger::CheckTrigger(const Core::Vec3& actorPos, const Core::Vec3& /*prevActorPos*/) {
    if (!m_active) return false;

    if (m_bounds.Contains(actorPos)) {
        if (!m_isLocked) {
            TriggerTransition();
            return true;
        } else {
            return TryUnlockWithKey();
        }
    }
    return false;
}

bool ZoneTrigger::TryUnlockWithKey() {
    // FUN_0017c828: If required key is in inventory, unlock and trigger transition
    if (m_requiredKey.empty() || Inventory::InventoryManager::GetInstance().HasItem(m_requiredKey)) {
        m_isLocked = false;
        Core::EventManager::GetInstance().SendMsg("DoorUnlocked", this);
        TriggerTransition();
        return true;
    } else {
        // Door locked prompt (0x13c in FUN_0017bf80)
        Core::EventManager::GetInstance().SendMsg("DoorLocked", this);
        return false;
    }
}

void ZoneTrigger::TriggerTransition() {
    if (m_targetZone.empty()) return;

    // Send ZoneTransitionBegin message with target zone and target spawn name (SLES @ 0x006bfdb8)
    struct TransitionPayload {
        const char* targetZone;
        const char* targetSpawn;
        int transitionType;
    };

    TransitionPayload payload{ m_targetZone.c_str(), m_targetSpawn.c_str(), m_transitionType };
    Core::EventManager::GetInstance().SendMsg(Core::Events::ZoneTransitionBegin, this, &payload, sizeof(payload));
}

} // namespace Triggers
} // namespace SHO
