#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// HARNESS -- not the game's code.
//
// How the port aims and places a fixed camera. Position, FOV and the authored
// direction are data (the placement matrix); everything this file adds on top
// is ours: falling back to tracking when the authored shot loses the player,
// the 55-degree tilt limit, CConstraintCamera simply following, and stepping
// the eye through the last wall between camera and player. The real rules live
// in CStaticCamera / CConstraintCamera (21 properties, none decoded) and are
// not read yet.
// ─────────────────────────────────────────────────────────────────────────────
#include "ClimaxEngine/Core/Types.h"

namespace ClimaxEngine {
namespace Harness {

// Where a camera points with the player standing at `subject`.
//
// The two fixed-camera classes behave differently and the archive says so.
// CStaticCamera really is fixed, and the direction in its placement matrix is
// usually the right one: measured across all 473 instances against each level's
// own walkable markers, the median error is 4.5 degrees of yaw and 3.6 of
// pitch, and on 350 of 471 the authored aim holds the play space in frame. The
// remaining quarter is not a decoding failure but real data -- 29 levels give
// every camera the same default rotation -- so the authored direction is used
// only while it still holds the subject.
//
// CConstraintCamera is not fixed at all. 400 of its 417 instances carry no
// pitch, while the aim needed to see the floor from where they hang averages 11
// degrees down -- so its vertical aim is computed while the game runs, which is
// what "constraint camera" means. Its 21-property table holds the real tracking
// limits and none of them are decoded yet, so here it simply follows the
// player, stopped only from tipping further than a camera plausibly would.
glm::vec3 CameraAim(const LevelCamera &cam, const glm::vec3 &eye,
                    const glm::vec3 &subject);

// The eye position and look direction to render a camera with, given where the
// player is and what the level is built of.
//
// A fixed camera is routinely placed behind the room it films -- camEntrance in
// HO_1_Hallway1 stands at z = 3.20 while the floor stops at z = 2.00. From
// there a part of the frustum falls outside the level's geometry entirely and
// shows the void past the floor and the wall. So the eye is stepped through the
// last surface between it and the player, putting it in the same room he is in.
// How far it moves comes from the collision mesh, so a thick wall moves it
// further than a thin one without anything being tuned.
void ResolveCameraView(const CollisionMesh &world, const LevelCamera &cam,
                       const glm::vec3 &subject, glm::vec3 &eyeOut,
                       glm::vec3 &lookOut);

} // namespace Harness
} // namespace ClimaxEngine
