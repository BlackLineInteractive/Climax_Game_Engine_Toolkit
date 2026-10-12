#pragma once

#include "SHO/Camera/BaseCamera.h"
#include "SHO/Core/EventManager.h"
#include <string>

namespace SHO {
namespace Camera {

// ─────────────────────────────────────────────────────────────────────────────
// CFollowCamera
// Decompiled from SLES_551.47: Factory @ 0x0011C058, Attributes @ 0x0011BEF0 (size 160)
// ─────────────────────────────────────────────────────────────────────────────
class FollowCamera : public BaseCamera {
public:
    FollowCamera();
    virtual ~FollowCamera() = default;

    float GetTargetDistance() const { return m_targetDistance; }               // prop 0 @ 0x8c
    void SetTargetDistance(float dist) { m_targetDistance = dist; }

    float GetHeightOffset() const { return m_heightOffset; }                   // prop 1 @ 0x90
    void SetHeightOffset(float h) { m_heightOffset = h; }

    float GetLagRate() const { return m_lagRate; }                             // prop 2 @ 0x94
    void SetLagRate(float rate) { m_lagRate = rate; }

    float GetCollisionRadius() const { return m_collisionRadius; }             // prop 3 @ 0x98
    void SetCollisionRadius(float r) { m_collisionRadius = r; }

    void Update(float dt, const Core::Vec3& targetPos, float targetYaw);

private:
    float m_targetDistance = 3.5f;     // 0x8c
    float m_heightOffset = 1.4f;       // 0x90
    float m_lagRate = 8.0f;            // 0x94
    float m_collisionRadius = 0.3f;    // 0x98
};

// ─────────────────────────────────────────────────────────────────────────────
// CPeepholeCamera
// Decompiled from SLES_551.47: Factory @ 0x0011C850, Attributes @ 0x0011C3C8 (size 280)
// ─────────────────────────────────────────────────────────────────────────────
class PeepholeCamera : public BaseCamera {
public:
    PeepholeCamera();
    virtual ~PeepholeCamera() = default;

    const std::string& GetEnterMsg() const { return m_enterMsg; }              // prop 0 @ 0xd0
    void SetEnterMsg(const std::string& msg) { m_enterMsg = msg; }

    const std::string& GetExitMsg() const { return m_exitMsg; }                // prop 1 @ 0xd8
    void SetExitMsg(const std::string& msg) { m_exitMsg = msg; }

    const std::string& GetLookMsg() const { return m_lookMsg; }                // prop 2 @ 0xe0
    void SetLookMsg(const std::string& msg) { m_lookMsg = msg; }

    int32_t GetYawLimit() const { return m_yawLimit; }                         // prop 3 @ 0xf0
    void SetYawLimit(int32_t lim) { m_yawLimit = lim; }

    int32_t GetPitchLimit() const { return m_pitchLimit; }                     // prop 4 @ 0xf4
    void SetPitchLimit(int32_t lim) { m_pitchLimit = lim; }

    float GetFocalLength() const { return m_focalLength; }                     // prop 5 @ 0x100
    void SetFocalLength(float f) { m_focalLength = f; }

    bool IsPeeping() const { return m_isPeeping; }
    void StartPeeping();
    void StopPeeping();

private:
    std::string m_enterMsg;            // 0xd0
    std::string m_exitMsg;             // 0xd8
    std::string m_lookMsg;             // 0xe0
    int32_t     m_yawLimit = 45;       // 0xf0
    int32_t     m_pitchLimit = 30;     // 0xf4
    float       m_focalLength = 1.0f;  // 0x100
    bool        m_isPeeping = false;
};

} // namespace Camera
} // namespace SHO
