#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// Loading a scene, as the game does it.
//
// `FUN_00179FB8` does not load anything. It pushes commands: 0x1C-byte nodes
// {next, prev, id, name, name2, a, b} onto a linked list at `world + 0x18`,
// through `FUN_00179AE0` (which skips a push when an identical id+name is
// already queued). `FUN_00179D60` drains that list once per frame and switches
// on the node's id. A handler returning 0 stops the drain for this frame -- the
// node stays at the head and runs again next frame -- and returning non-zero
// removes it and lets the next one run.
//
// The ids are the cases of `FUN_00179D60`'s switch, verbatim (there is no 7):
//
//     0x00  FUN_0017AA58  g    0x09  FUN_0017B5B0
//     0x01  FUN_0017B398  g    0x0A  FUN_0017B780
//     0x02  FUN_0017B448       0x0B  FUN_0017B680
//     0x03  FUN_0017B488  g    0x0C  FUN_0017A6B8  g
//     0x04  FUN_0017B510       0x0D  FUN_0017A7A8  g
//     0x05  FUN_0017B2E0  g    0x0E  FUN_0017B6C0  g
//     0x06  FUN_0017A900  g    0x0F  FUN_0017B7A0  g
//     0x08  FUN_0017B540       0x10  FUN_0017B848
//                              0x11  FUN_0017B890
//
// `g` marks the handlers that open with the same gate: unless world+0x24 is
// set, they return 0 while `FUN_001EF8F8` reports the screen fader moving
// (`fader+0x10`, its rate, non-zero). So a transition holds the queue until it
// has finished -- that is how the load waits for black without a timer. The
// other eight do not wait.
//
// Game logic: no GL, no SDL. Everything that touches a file, the screen or the
// audio device goes through SceneQueueHost.
// ─────────────────────────────────────────────────────────────────────────────
#include <deque>
#include <string>

namespace ClimaxEngine {
namespace Game {

enum class SceneCmd {
    Begin          = 0x00,   // FUN_0017AA58
    LoadTextures   = 0x01,   // FUN_0017B398 -- "%s.txd" and "%s-%s.txd"
    ReadHeader     = 0x02,   // FUN_0017B448
    Prepare        = 0x03,   // FUN_0017B488
    Step4          = 0x04,   // FUN_0017B510
    ClearWorld     = 0x05,   // FUN_0017B2E0
    ReleaseRes     = 0x06,   // FUN_0017A900
    Instantiate    = 0x08,   // FUN_0017B540 -> FUN_001777E8
    UnloadScene    = 0x09,   // FUN_0017B5B0 -- frees the named scene's resources
    HandOver       = 0x0A,   // FUN_0017B780 -- control returns to the player
    ResetSystems   = 0x0B,   // FUN_0017B680
    Transition     = 0x0C,   // FUN_0017A6B8 -- starts the fader, returns at once
    TransitionWait = 0x0D,   // FUN_0017A7A8 -- waits on the fader, then reverses it
    StepE          = 0x0E,   // FUN_0017B6C0
    StepF          = 0x0F,   // FUN_0017B7A0
    FreeTemp       = 0x10,   // FUN_0017B848
    LevelAudio     = 0x11,   // FUN_0017B890 -> FUN_001D9A38
};

const char *SceneCmdName(SceneCmd c);

// Whether the handler opens with the fader gate (the `g` column above).
bool SceneCmdGated(SceneCmd c);

// The three-word descriptor `FUN_0017B128` turns into a transition node:
// word 0 picks the node (0 -> 0x0C, else 0x0D), word 1 is its mode, word 2 a
// float duration in seconds (the handlers run the fader at 1/duration).
//
// New game (`FUN_001CF718`) passes 0x339F68 = {0, 0, 1.0} as the opening one
// and 0x339F50 = {1, 0, 1.0} as the closing one.
struct TransitionDesc {
    int   kind = 0;
    int   mode = 0;
    float seconds = 1.0f;
};

// One queued node: the id, the two name slots and the two parameter words.
struct SceneNode {
    SceneCmd    cmd = SceneCmd::Begin;
    std::string name;    // node[3]
    std::string name2;   // node[4]
    int         a = 0;   // node[5]
    int         b = 0;   // node[6]
};

// What the queue needs from outside. Each call is one handler; returning false
// is the handler returning 0 -- "not finished, run me again next frame".
class SceneQueueHost {
public:
    virtual ~SceneQueueHost() = default;

    // FUN_001EF8F8: is the screen fader moving? The gate on the `g` handlers.
    virtual bool FaderBusy() { return false; }

    // 0x0C FUN_0017A6B8, by node[5]:
    //   0 -> FUN_001EFC60(1/node[6], fader, 1, ...)   start, direction 1
    //   1 -> FUN_001EFC60(1/node[6], fader, 0, ...)   start, direction 0
    //   2, 4 -> FUN_001EFC18(fader)                    stop it where it is
    //   3 -> FUN_001EFC60(.., 2, ..)
    // It always returns 1; the gate on the next handler does the waiting.
    virtual bool Transition(int mode, float seconds) = 0;
    // 0x0D FUN_0017A7A8: returns 0 until the fader reports done
    // (FUN_001EFC30: fader+0x1C == 1.0), then, for modes 0 and 1, starts it
    // the other way with FUN_001EFD88(1/node[6], fader, 0, ...).
    virtual bool TransitionWait(int mode, float seconds) = 0;

    virtual bool ClearWorld() = 0;                         // 0x05
    virtual bool ReleaseResources() = 0;                   // 0x06
    virtual bool UnloadScene(const std::string &) { return true; }   // 0x09
    // 0x01. FUN_0017B398 formats "%s.txd" and, when it has a second name,
    // "%s-%s.txd": the room's texture dictionary and the one shared with the
    // room it is entered from.
    virtual bool LoadTextures(const std::string &scene,
                              const std::string &from) = 0;
    virtual bool ReadHeader() = 0;                         // 0x02
    virtual bool Prepare() = 0;                            // 0x03
    virtual bool Instantiate(const std::string &scene) = 0;   // 0x08
    virtual bool FreeTemporaries() = 0;                    // 0x10
    // 0x0B FUN_0017B680. The node carries a name ("PrePurge", "PostPurge")
    // that this handler does not read.
    virtual bool ResetSystems(const std::string &tag) = 0;
    virtual bool StartLevelAudio() = 0;                    // 0x11
    virtual bool HandOver() = 0;                           // 0x0A

    virtual bool MatchResources() { return true; }         // 0x00
    virtual bool ResolveResource() { return true; }        // 0x04
    virtual bool ResetSubsystems() { return true; }        // 0x0E
    virtual bool PlaySceneMovie(const std::string &name) { (void)name; return true; }  // 0x0F
};

class SceneQueue {
public:
    // `FUN_00179FB8(world, scene, a3, purge, closing, opening, extra)`, minus
    // the `extra` pair (param_7, a {name, name2} that pushes a 0x0D and a
    // second 0x0C; New Game passes null). `current` is the scene being left
    // (FUN_00178920), empty on a fresh start.
    void LoadScene(const std::string &scene, const std::string &current,
                   const TransitionDesc &opening, const TransitionDesc &closing,
                   bool purge = false, int arg3 = 0);

    // `FUN_00179D60`: drain until a handler says "not yet". Call once a frame.
    void Update(SceneQueueHost &host);

    bool Busy() const { return !m_nodes.empty(); }
    SceneCmd Current() const;
    const std::string &Scene() const { return m_scene; }

private:
    void Push(SceneCmd c, std::string name = {}, std::string name2 = {},
              int a = 0, int b = 0);
    void PushTransition(const TransitionDesc &d);

    std::deque<SceneNode> m_nodes;
    std::string m_scene;
};

} // namespace Game
} // namespace ClimaxEngine
