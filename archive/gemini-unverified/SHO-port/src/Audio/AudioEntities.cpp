#include "SHO/Audio/AudioEntities.h"
#include "SHO/Core/EventManager.h"

namespace SHO {
namespace Audio {

AudioSound3D::AudioSound3D() {
    m_className = "AudioSound3D";
}

void AudioSound3D::Play() {
    m_isPlaying = true;
    Core::EventManager::GetInstance().SendMsg("PlaySound3D", this, (void*)m_waveName.c_str());
}

void AudioSound3D::Stop() {
    m_isPlaying = false;
    Core::EventManager::GetInstance().SendMsg("StopSound3D", this);
}

AudioStream::AudioStream() {
    m_className = "AudioStream";
}

void AudioStream::Play() {
    m_isPlaying = true;
    Core::EventManager::GetInstance().SendMsg("PlayAudioStream", this, (void*)m_streamPath.c_str());
}

void AudioStream::Stop() {
    m_isPlaying = false;
    Core::EventManager::GetInstance().SendMsg("StopAudioStream", this);
}

void AudioStream::FadeOut(float duration) {
    m_isPlaying = false;
    Core::EventManager::GetInstance().SendMsg("FadeOutAudioStream", this, &duration, sizeof(duration));
}

AudioReverb::AudioReverb() {
    m_className = "AudioReverb";
}

MaterialMap::MaterialMap() {
    m_className = "MaterialMap";
}

void MaterialMap::SetSoundForMaterial(int materialId, int soundId) {
    if (materialId >= 0 && materialId < 17) {
        m_materialSounds[materialId] = soundId;
    }
}

int MaterialMap::GetSoundForMaterial(int materialId) const {
    if (materialId >= 0 && materialId < 17) {
        return m_materialSounds[materialId];
    }
    return 0;
}

} // namespace Audio
} // namespace SHO
