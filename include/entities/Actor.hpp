#pragma once
#include <SFML/system/Vector2.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "../core/Entity.hpp"
#include "../core/Renderable.hpp"
#include "../animation/Animation.hpp"
#include <vector>

/**
 * Actor
 * 可动实体类
 * 在Entity的基础上添加了有关移动的参数和方法
 * !!!移动和加速都是时间依赖的实现
*/
class Actor : public Entity, public Renderable {
protected:
    sf::Vector2f interPolatePos_;
    
private:
    sf::Texture texture_;
    sf::Sprite sprite_;
    sf::Vector2f foothold_;//落脚点记录，相对于显示纹理左上角的坐标，创建角色使设置
    sf::Vector2f velocity_;
    sf::Vector2f acceleration_;
    sf::Vector2f acceleration_limit_;
    float decefactor_;

    sf::Vector2f previous_position;

    // 动画系统
    std::vector<Animation> animations_;
    Animation* current_animation_ = nullptr;

public:
    Actor(sf::Vector2f center, sf::Vector2f halfsize, std::filesystem::path texture_path);

    Actor(sf::Vector2f center, float radius, std::filesystem::path texture_path);

    inline void setFootHold(sf::Vector2f foothold) {
        for (auto& anim : animations_) {
            anim.setFootHold(foothold);
        }
    }

    virtual void fixedUpDate(float fixed_dt);

    void toScreen(float alpha = 0.f, float dt = 0.f) override;

    void move(sf::Vector2f velocity);

    void accelerate(sf::Vector2f acceleration);

    /// 添加一个动画对象，由 Actor 统一管理
    void addAnimation(AnimationState state,
                      sf::Vector2i direction,
                      int frame_total,
                      int frame_start,
                      int frame_end,
                      float frame_duration = 0.15f,
                      sf::Vector2f scale_factor = {1.f, 1.f},
                      sf::Vector2f foothold = {0.f, 0.f});

    /// 子类可重写以根据自身状态返回对应的 AnimationState
    virtual AnimationState getAnimationState() const;

    inline sf::Vector2f getVelocity() const {
        return velocity_;
    }

    inline void setVelocity(sf::Vector2f velocity) {
        velocity_ = velocity;
    }

    inline sf::Vector2f getAcceleration() const {
        return acceleration_;
    }

    inline void setAcceleration(sf::Vector2f acceleration) {
        acceleration_ = acceleration;
    }

    inline const sf::Drawable &getDrawable() const {
        return sprite_;
    }

    inline float getRenderOrder() const {
        return getCenter().y;
    }

    inline void setAccelerationLimit(sf::Vector2f limit) {
        acceleration_limit_ = limit;
    }

    inline void setDecefactor(float dec) {
        decefactor_ = dec;
    }

    inline sf::Vector2i getDirection() const {
        sf::Vector2i dir = sf::Vector2i(0, 0);
        if (velocity_.x > 0.f) {
            dir += sf::Vector2i(1, 0);
        } else if (velocity_.x < 0.f) {
            dir += sf::Vector2i(-1, 0);
        }
        if (velocity_.y > 0.f) {
            dir += sf::Vector2i(0, 1);
        } else if (velocity_.y < 0.f) {
            dir += sf::Vector2i(0, -1);
        }
        return dir;
    }

    // 允许 Animation 直接访问 sprite_/texture_（也可通过 addAnimation 间接创建）
};