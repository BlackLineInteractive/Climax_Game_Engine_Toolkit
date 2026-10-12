#pragma once

#include "SHO/Core/Types.h"
#include <string>
#include <vector>
#include <map>

namespace SHO {
namespace Actor {
    class PlayerBehaviour;
}

namespace Animation {

// Standard player animation states from Travis_STD.rst
enum class PlayerAnimState {
    Idle,
    Walking,
    Running,
    RunningFast,
    Combat_Idle,
    MeleeCombo1,
    MeleeCombo2,
    KnifeAttack,
    FirearmAim,
    Reload,
    Block,
    Grabbed,
    Grab_Struggle,
    Grab_Win,
    Grab_Lose,
    BeenHitGround,
    Fatigued_Idle,
    Injured_Idle
};

// Transition condition matching FUN_0019f710 / FUN_001a0cc0
struct Condition {
    std::string varName;
    uint32_t    varHash = 0;
    float       valA = 0.0f;
    float       valB = 0.0f;
    std::string opCode = "=="; // "==", "!=", "<", "<=", ">", ">=", "<>"

    bool Evaluate(float varVal) const;
};

// Transition edge in graph (FUN_0019f448 @ 0x0019F448)
struct Transition {
    PlayerAnimState srcState;
    PlayerAnimState dstState;
    float           blendDuration = 0.2f;
    std::vector<Condition> conditions;
};

// State Transition Graph driving Travis_STD.rst (rwID_STATETRANSITION)
class StateTransitionGraph {
public:
    StateTransitionGraph();

    // Reset graph to initial Idle state
    void Reset();

    // Advance state machine each frame based on PlayerBehaviour attributes
    // Returns the exact .anm clip filename that should be played
    std::string Update(const Actor::PlayerBehaviour& player, float dt);

    PlayerAnimState GetCurrentState() const { return m_currentState; }
    const std::string& GetCurrentStateName() const { return m_currentStateName; }
    float GetStateTime() const { return m_stateTime; }
    float GetBlendDuration() const { return m_blendDuration; }

    // Direct transition trigger (e.g. for damage or grapple events)
    void ForceState(PlayerAnimState state, float blendDuration = 0.2f);

private:
    void BuildDefaultGraph();
    std::string ResolveClipForState(PlayerAnimState state, int32_t weaponClass, bool isInjured, bool isFatigued) const;

    PlayerAnimState m_currentState = PlayerAnimState::Idle;
    std::string     m_currentStateName = "Idle";
    float           m_stateTime = 0.0f;
    float           m_blendDuration = 0.2f;

    std::vector<Transition> m_transitions;
    float m_footstepTimer = 0.0f;
};

} // namespace Animation
} // namespace SHO
