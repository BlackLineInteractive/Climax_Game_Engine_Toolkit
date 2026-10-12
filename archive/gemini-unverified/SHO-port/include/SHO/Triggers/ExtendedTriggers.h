#pragma once

#include "SHO/Triggers/TriggerBase.h"
#include "SHO/Core/EventManager.h"
#include "SHO/Core/EventIds.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

namespace SHO {
namespace Triggers {

// ─────────────────────────────────────────────────────────────────────────────
// ButtonBoxTrigger
// Decompiled from SLES_551.47: Factory @ 0x001691D8, Ctor @ 0x00169228,
// Attributes @ 0x00168E60 / 0x001695D8, Event Dispatch @ 0x00169448 (size 272)
// ─────────────────────────────────────────────────────────────────────────────
class ButtonBoxTrigger : public TriggerBase {
public:
    ButtonBoxTrigger();
    virtual ~ButtonBoxTrigger();

    void SetBounds(const Core::Vec3& min, const Core::Vec3& max);
    void SetTargetEvent(const std::string& evt) { m_targetEvent = evt; }
    void SetPromptText(const std::string& prompt) { m_prompt = prompt; }
    void SetButtonType(int buttonType) { m_buttonType = buttonType; } // 0=Cross, 1=Square, 2=Triangle

    void SetEnableMessage(const std::string& msg);
    void SetDisableMessage(const std::string& msg);

    // FUN_00169448: Transforms actor pos into local space via inverse matrix & checks threshold
    bool CheckTrigger(const Core::Vec3& currentPos, const Core::Vec3& previousPos) override;
    void Interact();

    bool IsActive() const { return m_active; }
    void SetActiveState(bool active) { m_active = active; }

private:
    glm::mat4 m_transform = glm::mat4(1.0f);        // 0x60 / 0x70
    glm::mat4 m_inverseTransform = glm::mat4(1.0f); // 0xa0 / 0xb0 (via FUN_0024bdb8)
    std::string m_targetEvent;                      // 0x50
    std::string m_prompt;                           // 0x48
    std::string m_enableMsg;                        // 0xf8
    std::string m_disableMsg;                       // 0x100
    int m_buttonType = 0;                           // 0x44 (0=Cross, 1=Square, 2=Triangle)
    bool m_active = true;                           // 0xe0 / 0xf0
    bool m_playerInside = false;                    // 0xe4
    uint32_t m_enableHandle = 0;
    uint32_t m_disableHandle = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// AreaTriggerBox
// Decompiled from SLES_551.47: Factory @ 0x00171938, Ctor @ 0x00171988 / 0x001703D0,
// Attributes @ 0x001712C8 / 0x00170650, Point Check @ 0x00171B28 (size 288)
// ─────────────────────────────────────────────────────────────────────────────
class AreaTriggerBox : public TriggerBase {
public:
    AreaTriggerBox();
    virtual ~AreaTriggerBox();

    void SetBoxExtents(const Core::Vec3& center, const Core::Vec3& halfExtents);
    void SetOnEnterEvent(const std::string& enterEvt) { m_onEnterEvent = enterEvt; }
    void SetOnExitEvent(const std::string& exitEvt) { m_onExitEvent = exitEvt; }
    void SetTriggerOnce(bool once) { m_triggerOnce = once; }
    void SetPlayerOnly(bool playerOnly) { m_playerOnly = playerOnly; }

    // FUN_00171B28: Transforms point by inverse matrix (0x90) and tests [-0.5, 0.5]^3
    bool IsPointInside(const Core::Vec3& worldPos) const;
    bool CheckTrigger(const Core::Vec3& currentPos, const Core::Vec3& previousPos) override;

private:
    glm::mat4 m_transform = glm::mat4(1.0f);        // 0x50
    glm::mat4 m_inverseTransform = glm::mat4(1.0f); // 0x90
    std::string m_onEnterEvent;                     // 0xf0
    std::string m_onExitEvent;                      // 0xf8
    bool m_active = true;                           // 0x104
    bool m_triggerOnce = false;                     // 0x10c
    bool m_playerOnly = true;                       // 0x110
    bool m_wasInside = false;                       // 0xd0
    bool m_hasTriggered = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// AreaTriggerBattleManager
// Decompiled from SLES_551.47: Factory @ 0x00171068, Ctor @ 0x001710B8 (size 368)
// ─────────────────────────────────────────────────────────────────────────────
class BattleManagerTrigger : public TriggerBase {
public:
    BattleManagerTrigger();
    virtual ~BattleManagerTrigger() = default;

    void SetWaveCount(int waves) { m_waves = waves; }
    void SetClearEvent(const std::string& clearEvt) { m_clearEvent = clearEvt; }
    void OnEnemyKilled();

    bool CheckTrigger(const Core::Vec3& currentPos, const Core::Vec3& previousPos) override;

    bool IsBattleActive() const { return m_active; }
    bool IsBattleCleared() const { return m_cleared; }

private:
    int m_waves = 1;
    int m_remainingEnemies = 0;
    std::string m_clearEvent;
    bool m_active = false;
    bool m_cleared = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// CFadeTrigger & CScreenShakeTrigger
// Decompiled from SLES_551.47 @ 0x0016B2C0 & 0x0016EC10
// ─────────────────────────────────────────────────────────────────────────────
class ScreenEffectTrigger : public TriggerBase {
public:
    ScreenEffectTrigger();
    virtual ~ScreenEffectTrigger() = default;

    void SetFadeColor(const Core::Vec3& col) { m_fadeColor = col; }
    void SetFadeDuration(float dur) { m_duration = dur; }
    void SetShakeIntensity(float intensity) { m_shakeIntensity = intensity; }

    bool CheckTrigger(const Core::Vec3& currentPos, const Core::Vec3& previousPos) override;

    void TriggerFade(bool fadeOut);
    void TriggerShake(float intensity, float duration);

private:
    Core::Vec3 m_fadeColor = Core::Vec3(0.0f);
    float m_duration = 1.0f;
    float m_shakeIntensity = 0.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// CCounter
// Decompiled from SLES_551.47 @ 0x00169CD8
// ─────────────────────────────────────────────────────────────────────────────
class EventCounter : public Core::Behaviour {
public:
    EventCounter();
    virtual ~EventCounter() = default;

    void SetTargetCount(int target) { m_targetCount = target; }
    void SetTargetEvent(const std::string& evt) { m_targetEvent = evt; }
    void Increment();
    void Reset() { m_currentCount = 0; }

    int GetCurrentCount() const { return m_currentCount; }
    bool IsSatisfied() const { return m_currentCount >= m_targetCount; }

private:
    int m_currentCount = 0;
    int m_targetCount = 1;
    std::string m_targetEvent;
};

// ─────────────────────────────────────────────────────────────────────────────
// CPadRumbleTrigger
// Decompiled from SLES_551.47 @ 0x0016D7F8 (size 108)
// ─────────────────────────────────────────────────────────────────────────────
class PadRumbleTrigger : public Core::Behaviour {
public:
    PadRumbleTrigger();
    virtual ~PadRumbleTrigger() = default;

    const std::string& GetTriggerEvent() const { return m_triggerEvent; }      // 0x44
    void SetTriggerEvent(const std::string& evt) { m_triggerEvent = evt; }

    int32_t GetLargeMotor() const { return m_largeMotor; }                     // 0x4c (0..255)
    void SetLargeMotor(int32_t val) { m_largeMotor = val; }

    int32_t GetSmallMotor() const { return m_smallMotor; }                     // 0x54 (0..255)
    void SetSmallMotor(int32_t val) { m_smallMotor = val; }

    float GetDuration() const { return m_duration; }                           // 0x58
    void SetDuration(float d) { m_duration = d; }

    float GetFadeIn() const { return m_fadeIn; }                               // 0x5c
    void SetFadeIn(float f) { m_fadeIn = f; }

    float GetFadeOut() const { return m_fadeOut; }                             // 0x64 (100)
    void SetFadeOut(float f) { m_fadeOut = f; }

    void ExecuteRumble();

private:
    std::string m_triggerEvent;        // 0x44
    int32_t     m_largeMotor = 128;    // 0x4c
    int32_t     m_smallMotor = 128;    // 0x54
    float       m_duration = 0.5f;     // 0x58
    float       m_fadeIn = 0.05f;      // 0x5c
    float       m_fadeOut = 0.1f;      // 0x64
};

// ─────────────────────────────────────────────────────────────────────────────
// CScreenShakeTrigger
// Decompiled from SLES_551.47 @ 0x0016EA78 (size 108)
// ─────────────────────────────────────────────────────────────────────────────
class ScreenShakeTrigger : public Core::Behaviour {
public:
    ScreenShakeTrigger();
    virtual ~ScreenShakeTrigger() = default;

    const std::string& GetTriggerEvent() const { return m_triggerEvent; }      // 0x44
    void SetTriggerEvent(const std::string& evt) { m_triggerEvent = evt; }

    float GetAmplitude() const { return m_amplitude; }                         // 0x4c
    void SetAmplitude(float amp) { m_amplitude = amp; }

    float GetFrequency() const { return m_frequency; }                         // 0x50
    void SetFrequency(float freq) { m_frequency = freq; }

    float GetDuration() const { return m_duration; }                           // 0x54
    void SetDuration(float dur) { m_duration = dur; }

    float GetDecayRate() const { return m_decayRate; }                         // 0x58
    void SetDecayRate(float decay) { m_decayRate = decay; }

    void ExecuteShake();

private:
    std::string m_triggerEvent;        // 0x44
    float       m_amplitude = 0.1f;    // 0x4c
    float       m_frequency = 25.0f;   // 0x50
    float       m_duration = 0.6f;     // 0x54
    float       m_decayRate = 2.0f;    // 0x58
};

} // namespace Triggers
} // namespace SHO
