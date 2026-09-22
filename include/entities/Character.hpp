#pragma once
#include "Actor.hpp"

namespace CharacterID {
    enum index{
        player,
        cow,
        pig,
        sheep
    };
};

/**
 * Character
 * 一个可移动的，同时拥有血量，伤害等属性的Actor
 * 拥有交互行为
*/
class Character : public Actor {
private:
    int health_;
    int damage_;
    sf::Vector2f damagerange_;
    float attackCoolDown_;
    float attackCoolDownMax_ = 0.3f;
    bool isAttacking_ = false;
public:
    Character(sf::Vector2f center, sf::Vector2f halfsize, std::filesystem::path texture_path);

    Character(sf::Vector2f center, float radius, std::filesystem::path texture_path);

    void attack(Character &victim, sf::Vector2f direction);

    void fixedUpDate(float fixed_dt) override ;

    AnimationState getAnimationState() const override;

    void killed();

    inline void heal(int health) {
        health_ += health;
    }

    inline void hurt(int damage) {
        health_ -= damage;
    }

    inline int getHealth() {
        return health_;
    }

    inline void setHealth(int health) {
        health_ = health;
    }

    inline int getDamage() {
        return damage_;
    }

    inline void setDamage(int damage) {
        damage_ = damage;
    }

    inline sf::Vector2f getDamageRange() {
        return damagerange_;
    }

    inline void setDamageRange(sf::Vector2f damagerange) {
        damagerange_ = damagerange;
    }
};