#pragma once

#include "SHO/Core/Behaviour.h"
#include <cstdint>
#include <string>

namespace SHO {
namespace Audio {

// ─────────────────────────────────────────────────────────────────────────────
// AudioSound3D
// Decompiled from SLES_551.47: Factory @ 0x0019BBA0, HandleAttributes @ 0x0019B5A4 (size 160)
// ─────────────────────────────────────────────────────────────────────────────
class AudioSound3D : public Core::Behaviour {
public:
    AudioSound3D();
    virtual ~AudioSound3D() = default;

    const std::string& GetWaveName() const { return m_waveName; }              // 0x0019B5E4 (SelectWave)
    void SetWaveName(const std::string& wave) { m_waveName = wave; }

    int32_t GetFlags() const { return m_flags; }                               // prop 4 @ 0x70
    void SetFlags(int32_t flags) { m_flags = flags; }

    float GetMinDistance() const { return m_minDistance; }                     // prop 7 @ 0x78
    void SetMinDistance(float d) { m_minDistance = d; }

    float GetMaxDistance() const { return m_maxDistance; }                     // prop 8 @ 0x7c
    void SetMaxDistance(float d) { m_maxDistance = d; }

    float GetVolume() const { return m_volume; }                               // prop 9 @ 0x80
    void SetVolume(float v) { m_volume = v; }

    float GetPitch() const { return m_pitch; }                                 // prop 10 @ 0x84
    void SetPitch(float p) { m_pitch = p; }

    float GetRollOff() const { return m_rollOff; }                             // prop 11 @ 0x88
    void SetRollOff(float r) { m_rollOff = r; }

    float GetInnerAngle() const { return m_innerAngle; }                       // prop 12 @ 0x8c
    void SetInnerAngle(float a) { m_innerAngle = a; }

    float GetOuterAngle() const { return m_outerAngle; }                       // prop 13 @ 0x90
    void SetOuterAngle(float a) { m_outerAngle = a; }

    float GetOuterVolume() const { return m_outerVolume; }                     // prop 14 @ 0x94
    void SetOuterVolume(float v) { m_outerVolume = v; }

    void Play();
    void Stop();

private:
    std::string m_waveName;
    int32_t     m_flags = 0;           // 0x70
    float       m_minDistance = 1.0f;  // 0x78
    float       m_maxDistance = 15.0f; // 0x7c
    float       m_volume = 1.0f;       // 0x80
    float       m_pitch = 1.0f;        // 0x84
    float       m_rollOff = 1.0f;      // 0x88
    float       m_innerAngle = 360.0f; // 0x8c
    float       m_outerAngle = 360.0f; // 0x90
    float       m_outerVolume = 1.0f;  // 0x94
    bool        m_isPlaying = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// AudioStream
// Decompiled from SLES_551.47: Factory @ 0x0019C348, HandleAttributes @ 0x0019BF50 (size 108)
// ─────────────────────────────────────────────────────────────────────────────
class AudioStream : public Core::Behaviour {
public:
    AudioStream();
    virtual ~AudioStream() = default;

    const std::string& GetStreamPath() const { return m_streamPath; }
    void SetStreamPath(const std::string& path) { m_streamPath = path; }

    int32_t GetTrackId() const { return m_trackId; }                           // prop 0 @ 0x4c
    void SetTrackId(int32_t id) { m_trackId = id; }

    float GetVolume() const { return m_volume; }                               // prop 1 @ 0x50
    void SetVolume(float v) { m_volume = v; }

    int32_t GetLoopFlag() const { return m_loop; }                             // prop 2 @ 0x54
    void SetLoopFlag(int32_t loop) { m_loop = loop; }

    void Play();
    void Stop();
    void FadeOut(float duration);

private:
    std::string m_streamPath;
    int32_t     m_trackId = 0;         // 0x4c
    float       m_volume = 1.0f;       // 0x50
    int32_t     m_loop = 1;            // 0x54
    bool        m_isPlaying = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// AudioReverb
// Decompiled from SLES_551.47: Factory @ 0x0019B4B8, HandleAttributes @ 0x0019AE00 (size 176)
// ─────────────────────────────────────────────────────────────────────────────
class AudioReverb : public Core::Behaviour {
public:
    AudioReverb();
    virtual ~AudioReverb() = default;

    int32_t GetPreset() const { return m_preset; }                             // prop 0 @ 0x4c
    void SetPreset(int32_t p) { m_preset = p; }

    float GetDecayTime() const { return m_decayTime; }                         // prop 1 @ 0x50
    void SetDecayTime(float d) { m_decayTime = d; }

    float GetMix() const { return m_mix; }                                     // prop 2 @ 0x54
    void SetMix(float m) { m_mix = m; }

private:
    int32_t m_preset = 0;              // 0x4c
    float   m_decayTime = 1.5f;        // 0x50
    float   m_mix = 0.35f;             // 0x54
};

// ─────────────────────────────────────────────────────────────────────────────
// MaterialMap
// Decompiled from SLES_551.47: Factory @ 0x0010EF20, FUN_0010EF78 (size 148, 17 materials @ 0x4c)
// ─────────────────────────────────────────────────────────────────────────────
class MaterialMap : public Core::Behaviour {
public:
    MaterialMap();
    virtual ~MaterialMap() = default;

    void SetSoundForMaterial(int materialId, int soundId);
    int GetSoundForMaterial(int materialId) const;

private:
    int32_t m_materialSounds[17] = {0}; // 0x4c..0x8f (68 bytes, 17 material sound IDs)
};

} // namespace Audio
} // namespace SHO
