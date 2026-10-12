# RenderWare State Transition (`rwID_STATETRANSITION` / `.rst`) Specification

## 1. Overview & Role in Silent Hill: Origins

In the Climax Game Engine (built on Criterion's **RenderWare Studio 2.0**), characters and complex animated entities do not simply play raw `.anm` animation clips in isolation. Instead, high-level character behavior is governed by a **RenderWare State Transition Graph** (`.rst`).

The section tag in the engine is **`rwID_STATETRANSITION`**, and the primary player asset is **`Travis_STD.rst`** ("Travis Standard" state machine), embedded inside the container asset **`CPlayerBehaviour.Travis`** in `SH.ARC`.

This graph drives the animation blend tree and transitions based on the 29 game variables exposed by `CPlayerBehaviour` (`HasRun`, `HasAttacked`, `HasStanced`, `HeldWeaponClass`, `Fatigued`, `Injured`, etc.).

---

## 2. Binary Container & Identification

* **RenderWare Tag:** `rwID_STATETRANSITION`
* **Internal Magic / FourCC:** `0x4D494E41` (`"ANIM"` in Little-Endian ASCII)
* **Binary Loader in SLES_551.47:** `FUN_00125c50` (@ `0x00125C50`)
* **State Machine Parser:** `FUN_0019f448` (@ `0x0019F448`)
* **Runtime Transition Evaluator:** `FUN_0019f710` (@ `0x0019F710`)
* **Condition Comparator:** `FUN_001a0cc0` (@ `0x001A0CC0`)
* **State Setter & Blend Trigger:** `FUN_001a02e8` (@ `0x001A02E8`)

---

## 3. Data Structures

### 3.1 Header (`ANIM` Chunk)

Located at offset `0x327F74` inside decompressed `CPlayerBehaviour.Travis`:

| Offset | Type | Field | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `char[4]` | `magic` | `"ANIM"` (`0x4D494E41`) |
| `0x04` | `uint32_t` | `numStates` | Total number of animation states (**20** in `Travis_STD`) |
| `0x08` | `uint32_t` | `numTransitions` | Total transitions in graph (**214** in `Travis_STD`) |
| `0x0C` | `uint32_t` | `layerCount` | Sub-layer count (**5** layers) |

---

### 3.2 State Record (In-Memory Stride: `0x98` / 152 bytes)

Each state represents an animation mode (e.g. `Idle`, `Walking`, `RunningFast`, `RunningFaster`, `LockOn_Idle`, `Crawl`).

| Offset | Type | Field | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `uint32_t` | `stateId` | Unique sequential integer ID (0 .. `numStates-1`) |
| `0x04` | `char[32]` | `name` | State name string (null-terminated, e.g. `"Idle"`, `"Walking"`, `"RunningFast"`) |
| `0x24` | `char[32]` | `animClip` | Primary `.anm` clip (e.g. `PC_TG_Idle.anm`, `PC_TG_Walk.anm`) |
| `0x28` | `int32_t` | `loopCount` | `-1` for infinite looping, `1` for single-shot playback |
| `0x2C` | `float` | `speed` | Base playback speed multiplier (`1.0f`) |

> [!NOTE]
> Within the serialized container, each state includes weapon-specific variations based on `HeldWeaponClass` (e.g. `PC_TG_ShotGun_Idle.anm`, `PC_TG_Bulk_Idle.anm`, `PC_TG_Knife_CombatStance_SUB.anm`) as well as **Animation Cues** (e.g. `FootStep` @ 0.43s, `CameraShake`).

---

### 3.3 Transition Record (Stride: `0x3C` / 60 bytes)

Transitions define edges between states in the directed graph:

| Offset | Type | Field | Description |
| :--- | :--- | :--- | :--- |
| `0x20` | `uint32_t` | `srcStateId` | Origin state ID |
| `0x24` | `uint32_t` | `dstStateId` | Destination state ID |
| `0x2C` | `float` | `blendDuration` | Cross-fade blend time in seconds (e.g. `0.35f` = 350 ms) |
| `0x30` | `uint32_t` | `flags` | Transition flags (interruptibility, phase synchronization) |
| `0x34` | `uint32_t` | `numConditions` | Number of conditions that must ALL evaluate to true |
| `0x38` | `uint32_t` | `conditionsOffset` | Offset / pointer to condition records |

---

### 3.4 Condition Record (Stride: `0x24` / 36 bytes)

Conditions test variables exposed by `CPlayerBehaviour`:

| Offset | Type | Field | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `char[20]` | `varName` | Parameter name string (e.g. `"HasRun"`, `"HasAttacked"`, `"HeldWeaponClass"`) |
| `0x14` | `uint32_t` | `varHash` | 32-bit CRC/SLES hash of the parameter name |
| `0x18` | `float/int` | `valA` | Comparison operand A / minimum value |
| `0x1C` | `float/int` | `valB` | Comparison operand B / maximum value |
| `0x20` | `uint16_t` | `opCode` | 2-char ASCII comparison operator (see table below) |

---

## 4. Comparison Operators (`FUN_001a0cc0`)

The comparator function in SLES (`0x001A0CC0`) unpacks a 16-bit ASCII code to determine how the condition is checked:

| Operator Code | Little-Endian Hex | Meaning | Evaluated Logic |
| :---: | :---: | :--- | :--- |
| `==` | `0x3D3D` | Equal | `param == valA` |
| `!=` | `0x3D21` | Not Equal | `param != valA` |
| `<` | `0x003C` | Less Than | `param < valA` |
| `<=` | `0x3D3C` | Less Than or Equal | `param <= valA` |
| `>` | `0x003E` | Greater Than | `param > valA` |
| `>=` | `0x3D3E` | Greater Than or Equal | `param >= valA` |
| `<>` | `0x3E3C` | Range Check | `valA <= param && param <= valB` |

---

## 5. Runtime Evaluation Cycle (`FUN_0019f710`)

Every tick:
1. `FUN_0019f710` looks up the active state `param_1[0x203]` in the state table (`param_1[0x211]`).
2. It walks the linked list of outgoing transitions from that state.
3. For each transition:
   * It inspects each condition.
   * Finds the matching variable in the entity's 64-element parameter table (`param_1 + 2`) by name and hash.
   * Calls `FUN_001a0cc0` to evaluate the operator against the entity's current value.
4. If **all conditions** of a transition return `true`:
   * It calls `FUN_001a02e8` with `dstStateId`.
   * Sets cross-fade blend duration `param_1[0x206] = blendDuration`.
   * Triggers the animation subsystem to start cross-fading the skeleton bones into the new clip.
   * Execution breaks; the state transition is complete.
