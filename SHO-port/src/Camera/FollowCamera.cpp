#include "SHO/Camera/FollowCamera.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace SHO {
namespace Camera {

FollowCamera::FollowCamera() {
    m_className = "CFollowCamera";
}

void FollowCamera::Update(float dt, const Core::Vec3& targetPos, float targetYaw) {
    if (!m_active) return;

    // Calculate ideal behind-the-player camera position
    float rad = glm::radians(targetYaw);
    Core::Vec3 offset(
        -std::sin(rad) * m_targetDistance,
        m_heightOffset,
        -std::cos(rad) * m_targetDistance
    );

    Core::Vec3 desiredPos = targetPos + offset;
    // Exponential lag smoothing (FUN_0011bef0 @ 0x94)
    float t = 1.0f - std::exp(-m_lagRate * dt);
    SetPosition(glm::mix(GetPosition(), desiredPos, t));

    // Look at player subject height
    Core::Vec3 lookTarget = targetPos + Core::Vec3(0.0f, m_heightOffset, 0.0f);
    m_targetPos = lookTarget;
}

PeepholeCamera::PeepholeCamera() {
    m_className = "CPeepholeCamera";
}

void PeepholeCamera::StartPeeping() {
    m_isPeeping = true;
    m_active = true;
    if (!m_enterMsg.empty()) {
        Core::EventManager::GetInstance().SendMsg(m_enterMsg, this);
    }
}

void PeepholeCamera::StopPeeping() {
    m_isPeeping = false;
    m_active = false;
    if (!m_exitMsg.empty()) {
        Core::EventManager::GetInstance().SendMsg(m_exitMsg, this);
    }
}

} // namespace Camera
} // namespace SHO
