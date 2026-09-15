#include "../include/entities/Character.hpp"
#include "../include/core/VectorMath.hpp"

Character::Character(sf::Vector2f center, sf::Vector2f halfsize, std::filesystem::path texture_path):
Actor(center, halfsize, texture_path){}

Character::Character(sf::Vector2f center, float radius, std::filesystem::path texture_path):
Actor(center, radius, texture_path){}

void Character::attack(Character &victim, sf::Vector2f direction) {
    if (attackCoolDown_ > 0) {
        return;
    } 
    sf::Vector2f dis = victim.getCenter() - getCenter();
    if (
        angleUnsign(dis, direction) <= damagerange_.y &&
        vMod_p2(dis) <= damagerange_.x * damagerange_.x
    ) {
        victim.hurt(damage_);
    }
    attackCoolDown_ = attackCoolDownMax_;\
    isAttacking_ = true;
}

void Character::fixedUpDate(float fixed_dt) {
    Actor::fixedUpDate(fixed_dt);
    
    if (attackCoolDown_ > 0.f) {
        attackCoolDown_ -= fixed_dt;
        if (attackCoolDown_ < 0.f) {
            attackCoolDown_ = 0.f;
            isAttacking_ = false;
        }
    }
}

void Character::killed() {

}

AnimationState Character::getAnimationState() const {
    if(health_ <= 0) {
        return AnimationState::DEAD;
    }
    if(isAttacking_) {
        if (getVelocity() == sf::Vector2f(0.f, 0.f)) {
            return AnimationState::IDLE_ATTACK;
        } else {
            return AnimationState::MOVING_ATTACK;
        }
    } else {
        if (getVelocity() == sf::Vector2f(0.f, 0.f)) {
            return AnimationState::IDLE;
        } else {
            return AnimationState::MOVING;
        }
    }
}