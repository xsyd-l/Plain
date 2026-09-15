#pragma once
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

/// 动画状态 —— 每个 Animation 对象只管理一种状态下的纹理帧
enum class AnimationState {
    IDLE,
    MOVING,
    IDLE_ATTACK,
    MOVING_ATTACK,
    DEAD
};

/// Animation 负责管理 sprite_ 的纹理帧显示
/// 一个 Actor 可持有多个 Animation，但同一时刻只有一个在工作
class Animation {
private:
    sf::Sprite&   sprite_;
    sf::Texture&  texture_;
    sf::Vector2f   foothold_;      // 落脚点坐标，相对于纹理左上角
    AnimationState state_;          // 本动画对应的角色状态
    sf::Vector2i   direction_;     // 动画对应的方向副本
    int           frame_total_;     // 纹理中的总帧数
    int           frame_start_;     // 本动画的起始帧索引 (含)
    int           frame_end_;       // 本动画的结束帧索引 (含)
    int           current_frame_;   // 当前显示的帧索引
    float         frame_duration_;  // 每帧持续时间 (秒)
    float         elapsed_;         // 累计时间，用于推进帧
    sf::Vector2f   scale_factor_;
public:
    Animation(sf::Sprite& sprite,
              sf::Texture& texture,
              AnimationState state,
              sf::Vector2i direction,
              int frame_total,
              int frame_start,
              int frame_end,
              float frame_duration = 0.15f,
              sf::Vector2f scale_factor = {1.f, 1.f},
              sf::Vector2f foothold = {0.f, 0.f});

    void setFootHold(sf::Vector2f foothold) {
        foothold_ = foothold;
    }

    /// 启动动画：将 current_frame_ 置为 frame_start_，重置计时器，取消挂起
    void start();

    /// 每帧调用，dt 为距离上次调用的时间 (秒)，自动推进帧
    void update(float dt);
    //状态和方向是动画的唯一标识
    AnimationState getState() const { return state_; }
    sf::Vector2i getDirection() const { return direction_; }
};