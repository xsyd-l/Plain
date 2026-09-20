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
    // 记录最后朝向（8 方向，各分量独立取符号），静止时用于选择 idle 动画
    if (velocity_ != sf::Vector2f(0.f, 0.f)) {
        facing_ = sf::Vector2i(velocity_.x > 0.f ? 1 : (velocity_.x < 0.f ? -1 : 0),
                               velocity_.y > 0.f ? 1 : (velocity_.y < 0.f ? -1 : 0));
    }
    moveCollider(velocity_ * fixed_dt);
    acceleration_ = {0.f, 0.f};
}

void Actor::toScreen(float alpha, float dt) {
    //帧选择--由方向和状态确定动画示例
    AnimationState currentState = getAnimationState();
    sf::Vector2i rawDir = getDirection();

    // 方向收敛：各分量独立取符号，得到 8 方向单位向量（含对角 {±1,±1}）
    sf::Vector2i matchDir = sf::Vector2i(rawDir.x > 0 ? 1 : (rawDir.x < 0 ? -1 : 0),
                                         rawDir.y > 0 ? 1 : (rawDir.y < 0 ? -1 : 0));
    // 触发切换的三种情况：首次进入游戏 / 状态切换 / 方向改变
    if (current_animation_ == nullptr || current_animation_->getState() != currentState || current_animation_->getDirection() != matchDir) {
        const bool diagonal = (matchDir.x != 0 && matchDir.y != 0);
        const sf::Vector2i hDir = {matchDir.x, 0};   // 对角退化候选：水平
        const sf::Vector2i vDir = {0, matchDir.y};   // 对角退化候选：垂直

        Animation* best = nullptr;        // 精确匹配（8 方向）
        Animation* fallbackH = nullptr;   // 对角退化：水平优先
        Animation* fallbackV = nullptr;   // 对角退化：垂直
        Animation* fallbackAny = nullptr; // 任意同状态兜底

        for (auto& anim : animations_) {
            if (anim.getState() != currentState) continue;
            sf::Vector2i d = anim.getDirection();
            if (d == matchDir) { best = &anim; break; }
            if (diagonal) {
                if (fallbackH == nullptr && d == hDir) fallbackH = &anim;
                if (fallbackV == nullptr && d == vDir) fallbackV = &anim;
            }
            if (fallbackAny == nullptr) fallbackAny = &anim;
        }

        // 匹配优先级：精确 > 水平退化 > 垂直退化 > 同状态任意
        Animation* chosen = best ? best : (fallbackH ? fallbackH : (fallbackV ? fallbackV : fallbackAny));
        // 仅在真正换到另一个动画时才重启，避免同一动画每帧被反复 start 导致卡在第一帧
        if (chosen != nullptr && chosen != current_animation_) {
            chosen->start();
            current_animation_ = chosen;
            std::cout<<"动画切换\n";
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

void Actor::addAnimation(std::filesystem::path texture_path,
                         AnimationState state,
                         sf::Vector2i direction,
                         int frame_total,
                         int frame_start,
                         int frame_end,
                         float frame_duration,
                         sf::Vector2f scale_factor,
                         sf::Vector2f foothold) {
    animations_.emplace_back(sprite_, texture_path, state, direction,
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