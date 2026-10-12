#pragma once

#include "SHO/Core/Behaviour.h"
#include "SHO/Core/EventManager.h"
#include "SHO/Core/Types.h"
#include <string>

namespace SHO {
namespace Actor {

// ─────────────────────────────────────────────────────────────────────────────
// CPlayerSpawner
// Decompiled from SLES_551.47: Factory @ 0x0014AC40, Attributes @ 0x0014A888 (size 108)
// ─────────────────────────────────────────────────────────────────────────────
class PlayerSpawner : public Core::Behaviour {
public:
    PlayerSpawner();
    virtual ~PlayerSpawner() = default;

    int32_t GetInitialCamera() const { return m_initialCamera; }               // prop 0 @ 0x4c
    void SetInitialCamera(int32_t cam) { m_initialCamera = cam; }

    float GetSpawnAngle() const { return m_spawnAngle; }                       // prop 1 @ 0x50
    void SetSpawnAngle(float angle) { m_spawnAngle = angle; }

    int32_t GetPriority() const { return m_priority; }                         // prop 2 @ 0x54
    void SetPriority(int32_t prio) { m_priority = prio; }

    bool IsActive() const { return m_active; }                                 // prop 3 @ 0x58
    void SetActive(bool act) { m_active = act; }

    const std::string& GetSpawnEvent() const { return m_spawnEvent; }          // prop 4 @ 0x5c
    void SetSpawnEvent(const std::string& evt) { m_spawnEvent = evt; }

    bool IsDefaultSpawn() const { return m_defaultSpawn; }                     // prop 5 @ 0x64 (100)
    void SetDefaultSpawn(bool def) { m_defaultSpawn = def; }

    void ExecuteSpawn();

private:
    int32_t     m_initialCamera = -1;  // 0x4c
    float       m_spawnAngle = 0.0f;   // 0x50
    int32_t     m_priority = 0;        // 0x54
    bool        m_active = true;       // 0x58
    std::string m_spawnEvent;          // 0x5c
    bool        m_defaultSpawn = false;// 0x64
};

// ─────────────────────────────────────────────────────────────────────────────
// CPlayerCheckpoint
// Decompiled from SLES_551.47: Factory @ 0x0014A2E0, Attributes @ 0x00149FD8 (size 120)
// ─────────────────────────────────────────────────────────────────────────────
class PlayerCheckpoint : public Core::Behaviour {
public:
    PlayerCheckpoint();
    virtual ~PlayerCheckpoint() = default;

    const std::string& GetActivateMsg() const { return m_activateMsg; }        // prop 0 @ 0x4c
    void SetActivateMsg(const std::string& msg) { m_activateMsg = msg; }

    const std::string& GetDeactivateMsg() const { return m_deactivateMsg; }    // prop 1 @ 0x54
    void SetDeactivateMsg(const std::string& msg) { m_deactivateMsg = msg; }

    const std::string& GetQueryMsg() const { return m_queryMsg; }              // prop 2 @ 0x44
    void SetQueryMsg(const std::string& msg) { m_queryMsg = msg; }

    const std::string& GetTriggeredEvent() const { return m_triggeredEvent; }  // prop 3 @ 0x5c
    void SetTriggeredEvent(const std::string& evt) { m_triggeredEvent = evt; }

    bool IsActive() const { return m_active; }                                 // prop 4 @ 0x64 (100)
    void SetActive(bool act) { m_active = act; }

    bool ShouldSaveInventory() const { return m_saveInventory; }               // prop 5 @ 0x6c
    void SetSaveInventory(bool save) { m_saveInventory = save; }

    bool ShouldHealPlayer() const { return m_healPlayer; }                     // prop 6 @ 0x70
    void SetHealPlayer(bool heal) { m_healPlayer = heal; }

    void TriggerCheckpoint();

private:
    std::string m_activateMsg;         // 0x4c
    std::string m_deactivateMsg;       // 0x54
    std::string m_queryMsg;            // 0x44
    std::string m_triggeredEvent;      // 0x5c
    bool        m_active = true;       // 0x64
    bool        m_saveInventory = true;// 0x6c
    bool        m_healPlayer = false;  // 0x70
};

} // namespace Actor
} // namespace SHO
