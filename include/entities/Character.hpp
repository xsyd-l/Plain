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
    int health_ = 100;                  // 血量，构造时初始化，避免堆上残留垃圾值
    int damage_ = 10;                   // 伤害
    sf::Vector2f damagerange_ = {50.f, 0.8f};  // {攻击距离, 角度上限(弧度≈46°)}
    float attackCoolDown_ = 0.f;        // 攻击冷却，初版从未初始化
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

    void hurt(int damage);

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