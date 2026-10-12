#include "SHO/Actor/PlayerSpawner.h"
#include "SHO/Core/EventManager.h"
#include <iostream>

namespace SHO {
namespace Actor {

PlayerSpawner::PlayerSpawner() {
    m_className = "CPlayerSpawner";
}

void PlayerSpawner::ExecuteSpawn() {
    if (!m_active) return;

    struct SpawnPayload {
        Core::Vec3 position;
        float angle;
        int cameraIdx;
    };

    SpawnPayload payload{ GetPosition(), m_spawnAngle, m_initialCamera };
    Core::EventManager::GetInstance().SendMsg("PlayerSpawnAt", this, &payload, sizeof(payload));

    if (!m_spawnEvent.empty()) {
        Core::EventManager::GetInstance().SendMsg(m_spawnEvent, this);
    }
}

PlayerCheckpoint::PlayerCheckpoint() {
    m_className = "CPlayerCheckpoint";
}

void PlayerCheckpoint::TriggerCheckpoint() {
    if (!m_active) return;

    // Dispatch checkpoint reached notification
    Core::EventManager::GetInstance().SendMsg("CheckpointReached", this);

    if (!m_triggeredEvent.empty()) {
        Core::EventManager::GetInstance().SendMsg(m_triggeredEvent, this);
    }
}

} // namespace Actor
} // namespace SHO
