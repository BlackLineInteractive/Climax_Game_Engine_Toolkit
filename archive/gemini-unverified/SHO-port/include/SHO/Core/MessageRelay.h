#pragma once

#include "SHO/Core/Behaviour.h"
#include "SHO/Core/EventManager.h"
#include "SHO/Core/EventIds.h"
#include <string>
#include <vector>

namespace SHO {
namespace Core {

// ─────────────────────────────────────────────────────────────────────────────
// CMessageRelay
// Decompiled from SLES_551.47: Factory @ 0x0016CE80, Ctor @ 0x0016CEC8 (size 220)
// Manages 1 input trigger event, up to 10 relayed output events with delays,
// and enable/disable control messages.
// ─────────────────────────────────────────────────────────────────────────────
struct RelayedMessageTarget {
    std::string targetEvent; // 0x4c + i * 8
    float delaySec = 0.0f;   // 0x9c + i * 4
};

class MessageRelay : public Behaviour {
public:
    MessageRelay();
    virtual ~MessageRelay();

    void OnInit() override;
    void OnUpdate(float dt) override;

    void SetInputEvent(const std::string& inputEvt) { m_inputEvent = inputEvt; }
    void AddRelayedTarget(const std::string& targetEvt, float delaySec = 0.0f);

    void SetEnableMessage(const std::string& msg);
    void SetDisableMessage(const std::string& msg);

    void TriggerRelay();
    bool IsActive() const { return m_active; }

private:
    std::string m_inputEvent;                           // 0x44
    std::vector<RelayedMessageTarget> m_targets;       // 0x4c (up to 10 targets)
    std::string m_enableMsg;                            // 0xb8
    std::string m_disableMsg;                           // 0xc0
    bool m_active = true;                               // 0xd8 = 1

    struct PendingDispatch {
        std::string eventName;
        float remainingTime;
    };
    std::vector<PendingDispatch> m_pendingQueue;

    uint32_t m_inputHandle = 0;
    uint32_t m_enableHandle = 0;
    uint32_t m_disableHandle = 0;
};

} // namespace Core
} // namespace SHO
