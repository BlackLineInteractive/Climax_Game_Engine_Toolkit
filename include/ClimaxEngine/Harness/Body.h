#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "ClimaxEngine/Core/Types.h"

// ─────────────────────────────────────────────────────────────────────────────
// HARNESS -- not the game's code.
//
// A sphere pushed out of the level's collision triangles. The original walks
// Travis through `CollisionBSP::TestSphere` with a surface-class mask (the
// top nibble of each face's fourth word, docs/TODO.md §4c), and none of that
// is read yet: this solver treats every face as solid to everything, and its
// radius, gravity and slope limit were chosen, not recovered. It exists so the
// recovered parts of a level can be walked through; replace it, do not tune it.
// ─────────────────────────────────────────────────────────────────────────────
namespace ClimaxEngine {
namespace Harness {

// Moves a body through the level's collision mesh.
//
// Deliberately free of the viewer: no ImGui, no GL, no AppState. It takes a
// triangle soup and a desired motion and returns where the body ended up, so
// the same code can drive a port with no tool around it.
//
// The body is a sphere rather than a capsule. Silent Hill's collision is a
// coarse triangle mesh built for a character that walks -- 99 triangles for a
// whole corridor -- and against geometry that sparse a sphere behaves the same
// as a capsule everywhere it matters, while being far easier to get right.
struct CharacterController {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);

    float radius = 0.35f;      // body half-width, in level units
    float stepHeight = 0.45f;  // ledges this tall are walked over, not blocked
    float gravity = -18.0f;
    float maxSlope = 0.6f;     // cos of the steepest floor that still counts

    bool grounded = false;
    glm::vec3 groundNormal = glm::vec3(0, 1, 0);

    // `move` is the wanted horizontal motion for this step, already scaled by
    // speed and dt. Gravity is applied here, not by the caller.
    void Step(const CollisionMesh &world, const glm::vec3 &move, float dt);

    // Drops the body onto whatever is beneath it. Used at spawn, where the
    // placement matrix puts the object at roughly floor height but not exactly.
    bool SnapToGround(const CollisionMesh &world, float maxDrop = 4.0f);
};

// Closest point to `p` on triangle (a, b, c). Shared because both the sweep and
// the ground probe need it.
glm::vec3 ClosestPointOnTriangle(const glm::vec3 &p, const glm::vec3 &a,
                                 const glm::vec3 &b, const glm::vec3 &c);

// Distance along a downward ray from `origin` to the mesh, or a negative value
// when nothing is hit within `maxDist`.
float RayDown(const CollisionMesh &world, const glm::vec3 &origin,
              float maxDist, glm::vec3 *normalOut = nullptr);

// True when nothing in the collision mesh sits between the two points.
//
// Used by the toolkit to second-guess which of a PlaneTrigger's two cameras
// belongs to which side -- itself a guess (see CameraSwitcher).
bool HasLineOfSight(const CollisionMesh &world, const glm::vec3 &from,
                    const glm::vec3 &to);

// Distance from `from` to the *last* surface crossed on the way to `to`, or a
// negative value when the segment crosses nothing.
//
// The last one rather than the first: what matters for a camera buried in a
// wall is where it finally enters the room the player stands in, and a thick
// wall or a double-skinned one is crossed more than once.
float LastBlockerAlong(const CollisionMesh &world, const glm::vec3 &from,
                       const glm::vec3 &to);

} // namespace Harness
} // namespace ClimaxEngine
