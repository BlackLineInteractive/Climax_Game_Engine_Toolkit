#include "ClimaxEngine/Game/SceneQueue.h"

namespace ClimaxEngine {
namespace Game {

const char *SceneCmdName(SceneCmd c) {
    switch (c) {
    case SceneCmd::Begin:          return "Begin";
    case SceneCmd::LoadTextures:   return "LoadTextures";
    case SceneCmd::ReadHeader:     return "ReadHeader";
    case SceneCmd::Prepare:        return "Prepare";
    case SceneCmd::Step4:          return "Step4";
    case SceneCmd::ClearWorld:     return "ClearWorld";
    case SceneCmd::ReleaseRes:     return "ReleaseResources";
    case SceneCmd::Instantiate:    return "Instantiate";
    case SceneCmd::UnloadScene:    return "UnloadScene";
    case SceneCmd::HandOver:       return "HandOver";
    case SceneCmd::ResetSystems:   return "ResetSystems";
    case SceneCmd::Transition:     return "Transition";
    case SceneCmd::TransitionWait: return "TransitionWait";
    case SceneCmd::StepE:          return "StepE";
    case SceneCmd::StepF:          return "StepF";
    case SceneCmd::FreeTemp:       return "FreeTemporaries";
    case SceneCmd::LevelAudio:     return "LevelAudio";
    }
    return "?";
}

bool SceneCmdGated(SceneCmd c) {
    switch (c) {
    case SceneCmd::Begin:
    case SceneCmd::LoadTextures:
    case SceneCmd::Prepare:
    case SceneCmd::ClearWorld:
    case SceneCmd::ReleaseRes:
    case SceneCmd::Transition:
    case SceneCmd::TransitionWait:
    case SceneCmd::StepE:
    case SceneCmd::StepF:
        return true;
    default:
        return false;
    }
}

void SceneQueue::Push(SceneCmd c, std::string name, std::string name2, int a,
                      int b) {
    // FUN_00179AE0 walks the list first and does not queue a second node with
    // the same id and the same name.
    for (const SceneNode &n : m_nodes)
        if (n.cmd == c && n.name == name)
            return;
    m_nodes.push_back(SceneNode{c, std::move(name), std::move(name2), a, b});
}

// FUN_0017B128. The duration word is converted to an integer on the way into
// the node (`cvt.w.s`), and the handler divides 1 by it again.
void SceneQueue::PushTransition(const TransitionDesc &d) {
    m_nodes.push_back(SceneNode{d.kind == 0 ? SceneCmd::Transition
                                            : SceneCmd::TransitionWait,
                                {}, {}, d.mode, (int)d.seconds});
}

// FUN_00179FB8, in the order it pushes. The bracketed steps are conditional
// in the original too.
//
//   0x0C/0x0D  opening descriptor (FUN_0017B128)
//   0x0C       mode 2, pushed directly -- stop the fader once it is done
//   0x05, 0x06
//   [0x0B]     when `purge`
//   [0x09]     the scene being left, when it differs (no purge)
//   [0x0B]     "PrePurge" instead, when `purge`
//   0x01 0x02  scene + scene being left
//   0x03 0x08  scene + arg3
//   0x10
//   0x0B       "PostPurge"
//   0x11
//   0x0C/0x0D  closing descriptor
//   0x0A
void SceneQueue::LoadScene(const std::string &scene, const std::string &current,
                           const TransitionDesc &opening,
                           const TransitionDesc &closing, bool purge, int arg3) {
    m_scene = scene;
    m_nodes.clear();

    PushTransition(opening);
    m_nodes.push_back(SceneNode{SceneCmd::Transition, {}, {}, 2, 0});

    Push(SceneCmd::ClearWorld);
    Push(SceneCmd::ReleaseRes);
    if (purge)
        Push(SceneCmd::ResetSystems);

    std::string leaving;
    if (!scene.empty()) {
        if (!purge) {
            // uVar9 is the current scene whenever one is loaded; only the
            // unload depends on it being a different one.
            leaving = current;
            if (!current.empty() && current != scene)
                Push(SceneCmd::UnloadScene, leaving);
        } else {
            Push(SceneCmd::ResetSystems, "PrePurge");
        }
        Push(SceneCmd::LoadTextures, scene, leaving);
        Push(SceneCmd::ReadHeader, scene, leaving);
        Push(SceneCmd::Prepare, scene, {}, arg3);
        Push(SceneCmd::Instantiate, scene, {}, arg3);
    }

    Push(SceneCmd::FreeTemp);
    Push(SceneCmd::ResetSystems, "PostPurge");
    m_nodes.push_back(SceneNode{SceneCmd::LevelAudio});
    PushTransition(closing);
    Push(SceneCmd::HandOver);
}

SceneCmd SceneQueue::Current() const {
    return m_nodes.empty() ? SceneCmd::HandOver : m_nodes.front().cmd;
}

// FUN_00179D60. Run nodes until one is not finished; that one stays at the
// head and is tried again next frame.
void SceneQueue::Update(SceneQueueHost &host) {
    while (!m_nodes.empty()) {
        const SceneNode &n = m_nodes.front();
        if (SceneCmdGated(n.cmd) && host.FaderBusy())
            return;
        bool done = true;
        switch (n.cmd) {
        case SceneCmd::Transition:     done = host.Transition(n.a, (float)n.b); break;
        case SceneCmd::TransitionWait: done = host.TransitionWait(n.a, (float)n.b); break;
        case SceneCmd::ClearWorld:     done = host.ClearWorld(); break;
        case SceneCmd::ReleaseRes:     done = host.ReleaseResources(); break;
        case SceneCmd::UnloadScene:    done = host.UnloadScene(n.name); break;
        case SceneCmd::LoadTextures:   done = host.LoadTextures(n.name, n.name2); break;
        case SceneCmd::ReadHeader:     done = host.ReadHeader(); break;
        case SceneCmd::Prepare:        done = host.Prepare(); break;
        case SceneCmd::Instantiate:    done = host.Instantiate(n.name); break;
        case SceneCmd::FreeTemp:       done = host.FreeTemporaries(); break;
        case SceneCmd::ResetSystems:   done = host.ResetSystems(n.name); break;
        case SceneCmd::LevelAudio:     done = host.StartLevelAudio(); break;
        case SceneCmd::HandOver:       done = host.HandOver(); break;
        case SceneCmd::Begin:          done = host.MatchResources(); break;
        case SceneCmd::Step4:          done = host.ResolveResource(); break;
        case SceneCmd::StepE:          done = host.ResetSubsystems(); break;
        case SceneCmd::StepF:          done = host.PlaySceneMovie(n.name); break;
        }
        if (!done)
            return;
        m_nodes.pop_front();
    }
}

} // namespace Game
} // namespace ClimaxEngine
