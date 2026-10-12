#pragma once

#include "SHO/Core/Behaviour.h"
#include "SHO/Core/EventManager.h"
#include <vector>
#include <string>

namespace SHO {
namespace World {

// Translated from CPhysicsObject (0x0013D490, size 416) & CSolidObject (0x0013F1E8, size 944)
class PhysicsObject : public Core::Behaviour {
public:
    PhysicsObject();
    virtual ~PhysicsObject() = default;

    void SetMass(float mass) { m_mass = mass; }
    float GetMass() const { return m_mass; }

    void SetVelocity(const Core::Vec3& vel) { m_velocity = vel; }
    const Core::Vec3& GetVelocity() const { return m_velocity; }

    void ApplyImpulse(const Core::Vec3& impulse);
    void UpdatePhysics(float dt);

    void SetSolid(bool solid) { m_isSolid = solid; }
    bool IsSolid() const { return m_isSolid; }

private:
    float m_mass = 10.0f;
    float m_friction = 0.85f;
    Core::Vec3 m_velocity = Core::Vec3(0.0f);
    bool m_isSolid = true;
};

// Translated from CSmashableObject (0x0013E7D0, size 480)
class SmashableObject : public PhysicsObject {
public:
    SmashableObject();
    virtual ~SmashableObject() = default;

    void SetBreakThreshold(float force) { m_breakThreshold = force; }
    void TakeImpact(float force);

    bool IsBroken() const { return m_broken; }
    const std::string& GetBreakEvent() const { return m_breakEvent; }
    void SetBreakEvent(const std::string& evt) { m_breakEvent = evt; }

private:
    float m_breakThreshold = 25.0f;
    bool m_broken = false;
    std::string m_breakEvent;
};

// Translated from Waypoint (0x001A5298, size 92) & CMoverNode (0x00147698, size 96)
struct MoverNode {
    Core::Vec3 position = Core::Vec3(0.0f);
    float waitTimeSec = 0.0f;
    float moveSpeed = 2.0f;
    std::string arrivalEvent;
};

// Translated from CPathFollower (0x001481A0, size 340) & CRotator (0x00148EA0, size 400)
class PathFollower : public Core::Behaviour {
public:
    PathFollower();
    virtual ~PathFollower() = default;

    void AddNode(const MoverNode& node) { m_nodes.push_back(node); }
    void SetLooping(bool loop) { m_loop = loop; }
    void SetPlaying(bool play) { m_playing = play; }

    void OnUpdate(float dt) override;
    int GetCurrentNodeIndex() const { return m_currentNode; }

private:
    std::vector<MoverNode> m_nodes;
    int m_currentNode = 0;
    float m_waitTimer = 0.0f;
    bool m_loop = true;
    bool m_playing = true;
};

} // namespace World
} // namespace SHO
