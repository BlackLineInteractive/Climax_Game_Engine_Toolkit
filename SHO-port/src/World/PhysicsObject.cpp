#include "SHO/World/PhysicsObject.h"
#include <algorithm>
#include <cmath>

namespace SHO {
namespace World {

PhysicsObject::PhysicsObject() {
    m_className = "CPhysicsObject";
}

void PhysicsObject::ApplyImpulse(const Core::Vec3& impulse) {
    if (m_mass > 0.0f) {
        m_velocity += impulse / m_mass;
    }
}

void PhysicsObject::UpdatePhysics(float dt) {
    if (glm::length(m_velocity) > 1e-4f) {
        Core::Vec3 currentPos = GetPosition();
        SetPosition(currentPos + m_velocity * dt);

        // Apply ground friction damping
        m_velocity *= std::pow(m_friction, dt * 60.0f);
        if (glm::length(m_velocity) < 0.01f) {
            m_velocity = Core::Vec3(0.0f);
        }
    }
}

SmashableObject::SmashableObject() {
    m_className = "CSmashableObject";
}

void SmashableObject::TakeImpact(float force) {
    if (m_broken) return;

    if (force >= m_breakThreshold) {
        m_broken = true;
        SetSolid(false);
        if (!m_breakEvent.empty()) {
            Core::EventManager::GetInstance().SendMsg(m_breakEvent, this);
        }
    }
}

PathFollower::PathFollower() {
    m_className = "CPathFollower";
}

void PathFollower::OnUpdate(float dt) {
    if (!m_playing || m_nodes.empty()) return;

    if (m_waitTimer > 0.0f) {
        m_waitTimer -= dt;
        return;
    }

    const auto& targetNode = m_nodes[m_currentNode];
    Core::Vec3 currentPos = GetPosition();
    Core::Vec3 toTarget = targetNode.position - currentPos;
    float dist = glm::length(toTarget);

    float step = targetNode.moveSpeed * dt;
    if (dist <= step) {
        // Arrived at node
        SetPosition(targetNode.position);
        if (!targetNode.arrivalEvent.empty()) {
            Core::EventManager::GetInstance().SendMsg(targetNode.arrivalEvent, this);
        }

        m_waitTimer = targetNode.waitTimeSec;
        m_currentNode++;

        if (m_currentNode >= static_cast<int>(m_nodes.size())) {
            if (m_loop) {
                m_currentNode = 0;
            } else {
                m_playing = false;
            }
        }
    } else {
        SetPosition(currentPos + glm::normalize(toTarget) * step);
    }
}

} // namespace World
} // namespace SHO
