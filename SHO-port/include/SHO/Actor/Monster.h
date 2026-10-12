#pragma once

#include "SHO/Actor/EnemyBehaviour.h"

namespace SHO {
namespace Actor {

enum class MonsterType {
    Nurse,
    StraightJacket,
    Butcher,
    Caliban,
    Ariel,
    TwoBack
};

// Thin wrapper over CEnemyBehaviour mapping MonsterType to archetypes in sho_enemies.json
class Monster : public EnemyBehaviour {
public:
    Monster(MonsterType type = MonsterType::Nurse);
    virtual ~Monster() = default;

    MonsterType GetMonsterType() const { return m_type; }

private:
    static EnemyArchetype MapType(MonsterType type);
    MonsterType m_type = MonsterType::Nurse;
};

} // namespace Actor
} // namespace SHO
