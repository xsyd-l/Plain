#include "../include/entities/Actor.hpp"
#include "../include/core/VectorMath.hpp"

Actor::Actor(sf::Vector2f center, sf::Vector2f halfsize, std::filesystem::path texture_path):
Entity(center, halfsize, MoveTag::movable), sprite_(texture_){
    texture_.loadFromFile(texture_path);
    sprite_.setTexture(texture_, true);
    decefactor_ = 0.95f;
    sf::FloatRect localBounds = sprite_.getLocalBounds();
    sprite_.setOrigin({localBounds.size.x / 2.f, localBounds.size.y});
    previous_position = getCenter();
}

Actor::Actor(sf::Vector2f center, float radius, std::filesystem::path texture_path):
Entity(center, radius, MoveTag::movable), sprite_(texture_){
    texture_.loadFromFile(texture_path);
    sprite_.setTexture(texture_, true);
    decefactor_ = 0.95f;
    sf::FloatRect localBounds = sprite_.getLocalBounds();
    sprite_.setOrigin({localBounds.size.x / 2.f, localBounds.size.y});
    previous_position = getCenter();
}

void Actor::fixedUpDate(float fixed_dt) {
    previous_position = getCenter();
    velocity_ += fixed_dt * acceleration_;
    velocity_ *= decefactor_;
    if ((acceleration_ == sf::Vector2f(0.f, 0.f)) && velocity_ * velocity_ <= 900.f) {
        velocity_ = {0.f, 0.f};
    }
    moveCollider(velocity_ * fixed_dt);
    acceleration_ = {0.f, 0.f};
}

void Actor::toScreen(float alpha, float dt) {
    //帧选择
    AnimationState currentState = getAnimationState();
    sf::Vector2i rawDir = getDirection();

    // 动画匹配时只关心水平方向（左/右），忽略垂直分量
    sf::Vector2i matchDir = rawDir;
    if (matchDir.x > 0)      matchDir = {1, 0};
    else if (matchDir.x < 0) matchDir = {-1, 0};
    // x == 0 时保留原值（如 IDLE 时 {0, 0}）

    if (current_animation_ == nullptr || current_animation_->getState() != currentState || current_animation_->getDirection() != matchDir) {
        Animation* fallback = nullptr;
        for (auto& anim : animations_) {
            if (anim.getState() == currentState) {
                if (anim.getDirection() == matchDir) {
                    anim.start();
                    current_animation_ = &anim;
                    fallback = nullptr;
                    break;
                }
                if (fallback == nullptr) {
                    fallback = &anim;  // 同状态兜底
                }
            }
        }
        // 无精确匹配时使用兜底
        if (fallback != nullptr) {
            fallback->start();
            current_animation_ = fallback;
        }
    }

    if (current_animation_ != nullptr) {
        current_animation_->update(dt);
    }

    //帧位置插值
    sf::Vector2f interPolatePos = previous_position + alpha * (getCenter() - previous_position);
    sprite_.setPosition(interPolatePos);
}

void Actor::move(sf::Vector2f velocity) {
    moveCollider(velocity);
}

void Actor::accelerate(sf::Vector2f acceleration) {
    acceleration_ += acceleration;
}

void Actor::addAnimation(AnimationState state,
                         sf::Vector2i direction,
                         int frame_total,
                         int frame_start,
                         int frame_end,
                         float frame_duration,
                         sf::Vector2f scale_factor,
                         sf::Vector2f foothold) {
    animations_.emplace_back(sprite_, texture_, state, direction,
                             frame_total, frame_start, frame_end,
                             frame_duration, scale_factor, foothold);
    
}

AnimationState Actor::getAnimationState() const {
    // 默认逻辑：根据速度判断
    constexpr float threshold = 1.f;
    if (std::abs(velocity_.x) > threshold || std::abs(velocity_.y) > threshold) {
        return AnimationState::MOVING;
    }
    return AnimationState::IDLE;
}