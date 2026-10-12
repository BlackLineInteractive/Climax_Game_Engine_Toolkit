#include "SHO/Actor/Monster.h"

namespace SHO {
namespace Actor {

EnemyArchetype Monster::MapType(MonsterType type) {
    switch (type) {
    case MonsterType::Nurse:          return EnemyArchetype::PsychoNurse;
    case MonsterType::StraightJacket: return EnemyArchetype::StraightJacket;
    case MonsterType::Butcher:        return EnemyArchetype::Butcher;
    case MonsterType::Caliban:        return EnemyArchetype::CalibanBoss;
    case MonsterType::Ariel:          return EnemyArchetype::Ariel;
    case MonsterType::TwoBack:        return EnemyArchetype::TwoBack;
    default:                          return EnemyArchetype::PsychoNurse;
    }
}

Monster::Monster(MonsterType type)
    : EnemyBehaviour(MapType(type)), m_type(type) {
}

} // namespace Actor
} // namespace SHO
