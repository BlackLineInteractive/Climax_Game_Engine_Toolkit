#include "ClimaxEngine/Harness/CameraView.h"

#include "ClimaxEngine/Harness/Body.h"

#include <cmath>

namespace ClimaxEngine {
namespace Harness {

glm::vec3 CameraAim(const LevelCamera &cam, const glm::vec3 &eye,
                    const glm::vec3 &subject) {
  glm::vec3 v = subject - eye;
  if (glm::length(v) < 1e-3f)
    return cam.forward;
  v = glm::normalize(v);

  // A static camera keeps the direction it was given -- but only while that
  // direction still holds the subject. On 350 of 471 instances it does, and
  // there the authored shot is the game's own framing, which is the whole
  // point of this camera system. The other 121 sit in levels where every
  // camera carries the same default rotation -- 29 of the 201 levels with more
  // than one camera are like that, HO_1_Hallway1 among them -- and honouring a
  // default aims the shot at a wall. Falling through to tracking there is the
  // difference between the game's framing and no framing at all.
  if (cam.className == "CStaticCamera") {
    const float halfFov = glm::radians(glm::min(cam.fovDeg * 0.5f, 85.0f));
    if (glm::dot(v, cam.forward) >= cosf(halfFov))
      return cam.forward;
  }

  // Walking directly beneath a camera would otherwise point it at the floor;
  // the game never tilts that far. A stand-in for the real constraint, which
  // lives in properties this does not read yet.
  const float kMaxTilt = glm::radians(55.0f);
  const float minY = -sinf(kMaxTilt);
  if (v.y < minY) {
    glm::vec2 h(v.x, v.z);
    h = (glm::length(h) > 1e-4f)
            ? glm::normalize(h) * cosf(kMaxTilt)
            : glm::normalize(glm::vec2(cam.forward.x, cam.forward.z)) *
                  cosf(kMaxTilt);
    v = glm::vec3(h.x, minY, h.y);
  }
  return v;
}

void ResolveCameraView(const CollisionMesh &world, const LevelCamera &cam,
                       const glm::vec3 &subject, glm::vec3 &eyeOut,
                       glm::vec3 &lookOut) {
  eyeOut = cam.position;

  const glm::vec3 v = subject - eyeOut;
  const float dist = glm::length(v);
  if (dist > 1e-3f && !world.indices.empty()) {
    const float t = LastBlockerAlong(world, eyeOut, subject);
    // Clear the surface just crossed, and never step so far that the camera
    // lands on top of the player -- a shot needs room to be a shot.
    const float kClear = 0.25f;
    const float kMinShot = 1.5f;
    if (t > 0.0f && t + kClear < dist - kMinShot)
      eyeOut += (v / dist) * (t + kClear);
  }

  lookOut = CameraAim(cam, eyeOut, subject);
}

} // namespace Harness
} // namespace ClimaxEngine
