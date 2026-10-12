#include "SHO/Animation/StateTransitionGraph.h"
#include "SHO/Actor/PlayerBehaviour.h"
#include "SHO/Audio/AudioEngine.h"
#include "SHO/Core/EventManager.h"
#include <algorithm>
#include <cmath>

namespace SHO {
namespace Animation {

// Condition evaluator implementing SLES_551.47 @ 0x001A0CC0 (FUN_001a0cc0)
bool Condition::Evaluate(float varVal) const {
    if (opCode == "==") {
        return std::abs(varVal - valA) < 0.001f;
    } else if (opCode == "!=") {
        return std::abs(varVal - valA) >= 0.001f;
    } else if (opCode == "<") {
        return varVal < valA;
    } else if (opCode == "<=") {
        return varVal <= valA;
    } else if (opCode == ">") {
        return varVal > valA;
    } else if (opCode == ">=") {
        return varVal >= valA;
    } else if (opCode == "<>") {
        return valA <= varVal && varVal <= valB;
    }
    return false;
}

StateTransitionGraph::StateTransitionGraph() {
    BuildDefaultGraph();
    Reset();
}

void StateTransitionGraph::Reset() {
    m_currentState = PlayerAnimState::Idle;
    m_currentStateName = "Idle";
    m_stateTime = 0.0f;
    m_blendDuration = 0.2f;
    m_footstepTimer = 0.0f;
}

void StateTransitionGraph::ForceState(PlayerAnimState state, float blendDuration) {
    m_currentState = state;
    m_blendDuration = blendDuration;
    m_stateTime = 0.0f;
}

void StateTransitionGraph::BuildDefaultGraph() {
    m_transitions.clear();

    auto addTrans = [this](PlayerAnimState src, PlayerAnimState dst, float blend,
                           const std::vector<Condition>& conds) {
        Transition t;
        t.srcState = src;
        t.dstState = dst;
        t.blendDuration = blend;
        t.conditions = conds;
        m_transitions.push_back(t);
    };

    // ── From Idle ────────────────────────────────────────────────────────────
    addTrans(PlayerAnimState::Idle, PlayerAnimState::RunningFast, 0.35f,
             { {"HasRun", 0, 1.0f, 0.0f, "=="}, {"HasRunReleased", 0, 0.0f, 0.0f, "=="} });

    addTrans(PlayerAnimState::Idle, PlayerAnimState::Walking, 0.25f,
             { {"ForwardMovementValue", 0, 0.1f, 0.0f, ">"}, {"HasRun", 0, 0.0f, 0.0f, "=="} });

    addTrans(PlayerAnimState::Idle, PlayerAnimState::Combat_Idle, 0.2f,
             { {"HasStanced", 0, 1.0f, 0.0f, "=="} });

    addTrans(PlayerAnimState::Idle, PlayerAnimState::Block, 0.15f,
             { {"HasBlocked", 0, 1.0f, 0.0f, "=="} });

    addTrans(PlayerAnimState::Idle, PlayerAnimState::Fatigued_Idle, 0.4f,
             { {"Fatigued", 0, 1.0f, 0.0f, "=="} });

    // ── From Walking ─────────────────────────────────────────────────────────
    addTrans(PlayerAnimState::Walking, PlayerAnimState::RunningFast, 0.3f,
             { {"HasRun", 0, 1.0f, 0.0f, "=="} });

    addTrans(PlayerAnimState::Walking, PlayerAnimState::Idle, 0.25f,
             { {"ForwardMovementValue", 0, 0.1f, 0.0f, "<="} });

    addTrans(PlayerAnimState::Walking, PlayerAnimState::Combat_Idle, 0.2f,
             { {"HasStanced", 0, 1.0f, 0.0f, "=="} });

    // ── From RunningFast ─────────────────────────────────────────────────────
    addTrans(PlayerAnimState::RunningFast, PlayerAnimState::Walking, 0.3f,
             { {"HasRun", 0, 0.0f, 0.0f, "=="}, {"ForwardMovementValue", 0, 0.1f, 0.0f, ">"} });

    addTrans(PlayerAnimState::RunningFast, PlayerAnimState::Idle, 0.35f,
             { {"ForwardMovementValue", 0, 0.1f, 0.0f, "<="} });

    addTrans(PlayerAnimState::RunningFast, PlayerAnimState::Fatigued_Idle, 0.4f,
             { {"Fatigued", 0, 1.0f, 0.0f, "=="} });

    // ── From Combat_Idle ─────────────────────────────────────────────────────
    addTrans(PlayerAnimState::Combat_Idle, PlayerAnimState::Idle, 0.2f,
             { {"HasStanced", 0, 0.0f, 0.0f, "=="} });

    addTrans(PlayerAnimState::Combat_Idle, PlayerAnimState::MeleeCombo1, 0.15f,
             { {"HasAttacked", 0, 1.0f, 0.0f, "=="} });

    addTrans(PlayerAnimState::Combat_Idle, PlayerAnimState::Reload, 0.2f,
             { {"HasReloaded", 0, 1.0f, 0.0f, "=="} });

    addTrans(PlayerAnimState::Combat_Idle, PlayerAnimState::Block, 0.15f,
             { {"HasBlocked", 0, 1.0f, 0.0f, "=="} });

    // ── From Combat Actions ──────────────────────────────────────────────────
    addTrans(PlayerAnimState::MeleeCombo1, PlayerAnimState::MeleeCombo2, 0.15f,
             { {"HasAttacked", 0, 1.0f, 0.0f, "=="} });

    addTrans(PlayerAnimState::MeleeCombo1, PlayerAnimState::Combat_Idle, 0.25f,
             { {"HasAttackedReleased", 0, 1.0f, 0.0f, "=="} });

    addTrans(PlayerAnimState::MeleeCombo2, PlayerAnimState::Combat_Idle, 0.3f,
             { {"HasAttackedReleased", 0, 1.0f, 0.0f, "=="} });

    addTrans(PlayerAnimState::Block, PlayerAnimState::Idle, 0.2f,
             { {"HasBlocked", 0, 0.0f, 0.0f, "=="} });

    addTrans(PlayerAnimState::Reload, PlayerAnimState::Combat_Idle, 0.25f,
             { {"HasReloadedReleased", 0, 1.0f, 0.0f, "=="} });

    // ── From Fatigued_Idle ───────────────────────────────────────────────────
    addTrans(PlayerAnimState::Fatigued_Idle, PlayerAnimState::Idle, 0.4f,
             { {"Fatigued", 0, 0.0f, 0.0f, "=="} });
}

std::string StateTransitionGraph::ResolveClipForState(PlayerAnimState state, int32_t weaponClass,
                                                      bool isInjured, bool isFatigued) const {
    switch (state) {
    case PlayerAnimState::Idle:
        if (isInjured) return "PC_TG_Pained_Idle.anm";
        if (isFatigued) return "PC_TG_Fatigued_Idle.anm";
        return "PC_TG_Idle.anm";

    case PlayerAnimState::Walking:
        if (weaponClass == 2) return "PC_TG_ShotGun_Walk.anm"; // Shotgun
        if (weaponClass >= 4) return "PC_TG_Bulk_Walk.anm";    // Bulk/Heavy
        return "PC_TG_Walk.anm";

    case PlayerAnimState::Running:
    case PlayerAnimState::RunningFast:
        if (weaponClass == 2) return "PC_TG_ShotGun_Run.anm";
        return "PC_TG_Run.anm";

    case PlayerAnimState::Combat_Idle:
        if (weaponClass == 1) return "PC_TG_Pistol_CombatStance_L_SUB.anm";
        if (weaponClass == 2) return "PC_TG_ShotGun_StanceLevel_SUB.anm";
        if (weaponClass == 3) return "PC_TG_Knife_CombatStance_SUB.anm";
        if (weaponClass == 4) return "PC_TG_Sword_Idle_SUB.anm";
        return "PC_TG_Lockon_Idle.anm";

    case PlayerAnimState::MeleeCombo1:
        return "PC_TG_Punch_Combo_A.anm";

    case PlayerAnimState::MeleeCombo2:
        return "PC_TG_Punch_Combo_B.anm";

    case PlayerAnimState::Block:
        return "PC_TG_Block.anm";

    case PlayerAnimState::Reload:
        return "PC_TG_Pistol_Reload.anm";

    case PlayerAnimState::Grabbed:
    case PlayerAnimState::Grab_Struggle:
        return "PC_TG_Grapple_Loop.anm";

    case PlayerAnimState::BeenHitGround:
        return "PC_TG_Hit_Ground.anm";

    case PlayerAnimState::Fatigued_Idle:
        return "PC_TG_Fatigued_Idle.anm";

    case PlayerAnimState::Injured_Idle:
        return "PC_TG_Pained_Idle.anm";

    default:
        return "PC_TG_Idle.anm";
    }
}

std::string StateTransitionGraph::Update(const Actor::PlayerBehaviour& player, float dt) {
    m_stateTime += dt;

    // Check grapple override
    if (player.IsGrappled()) {
        m_currentState = PlayerAnimState::Grab_Struggle;
        m_currentStateName = "Grab_Struggle";
        return "PC_TG_Grapple_Loop.anm";
    }

    // Helper to extract player variables matching 29 Climax animation attributes
    auto getVar = [&player](const std::string& name) -> float {
        if (name == "HasRun") return player.HasRun() ? 1.0f : 0.0f;
        if (name == "HasRunReleased") return player.HasRunReleased() ? 1.0f : 0.0f;
        if (name == "HasAttacked") return player.HasAttacked() ? 1.0f : 0.0f;
        if (name == "HasAttackedReleased") return player.HasAttackedReleased() ? 1.0f : 0.0f;
        if (name == "HasStanced") return player.HasStanced() ? 1.0f : 0.0f;
        if (name == "HasBlocked") return player.HasBlocked() ? 1.0f : 0.0f;
        if (name == "HasReloaded") return player.HasReloaded() ? 1.0f : 0.0f;
        if (name == "HasReloadedReleased") return player.HasReloadedReleased() ? 1.0f : 0.0f;
        if (name == "Fatigued") return player.IsFatigued() ? 1.0f : 0.0f;
        if (name == "Injured") return player.IsInjured() ? 1.0f : 0.0f;
        if (name == "ForwardMovementValue") return player.GetForwardMovementValue();
        if (name == "HeldWeaponClass") return static_cast<float>(player.GetHeldWeaponClass());
        return 0.0f;
    };

    // Evaluate outgoing transitions matching FUN_0019f710
    for (const auto& trans : m_transitions) {
        if (trans.srcState != m_currentState) continue;

        bool allPassed = true;
        for (const auto& cond : trans.conditions) {
            float val = getVar(cond.varName);
            if (!cond.Evaluate(val)) {
                allPassed = false;
                break;
            }
        }

        if (allPassed) {
            m_currentState = trans.dstState;
            m_blendDuration = trans.blendDuration;
            m_stateTime = 0.0f;
            break;
        }
    }

    // Update state name string
    switch (m_currentState) {
    case PlayerAnimState::Idle:          m_currentStateName = "Idle"; break;
    case PlayerAnimState::Walking:       m_currentStateName = "Walking"; break;
    case PlayerAnimState::Running:       m_currentStateName = "Running"; break;
    case PlayerAnimState::RunningFast:   m_currentStateName = "RunningFast"; break;
    case PlayerAnimState::Combat_Idle:   m_currentStateName = "Combat_Idle"; break;
    case PlayerAnimState::MeleeCombo1:   m_currentStateName = "MeleeCombo1"; break;
    case PlayerAnimState::MeleeCombo2:   m_currentStateName = "MeleeCombo2"; break;
    case PlayerAnimState::Block:         m_currentStateName = "Block"; break;
    case PlayerAnimState::Reload:        m_currentStateName = "Reload"; break;
    case PlayerAnimState::Fatigued_Idle: m_currentStateName = "Fatigued_Idle"; break;
    default:                             m_currentStateName = "CustomState"; break;
    }

    // Footstep audio cues synchronized with animation state
    if (m_currentState == PlayerAnimState::Walking || m_currentState == PlayerAnimState::RunningFast) {
        m_footstepTimer += dt;
        float interval = (m_currentState == PlayerAnimState::RunningFast) ? 0.32f : 0.48f;
        if (m_footstepTimer >= interval) {
            m_footstepTimer = 0.0f;
            Core::EventManager::GetInstance().SendMsg("FootStep", (void*)&player);
        }
    } else {
        m_footstepTimer = 0.0f;
    }

    return ResolveClipForState(m_currentState, player.GetHeldWeaponClass(),
                               player.IsInjured(), player.IsFatigued());
}

} // namespace Animation
} // namespace SHO
