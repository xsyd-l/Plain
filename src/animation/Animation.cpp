#include "../include/animation/Animation.hpp"
#include <iostream>

Animation::Animation(sf::Sprite& sprite,
                     std::filesystem::path texture_path,
                     AnimationState state,
                     sf::Vector2i direction,
                     int frame_total,
                     int frame_start,
                     int frame_end,
                     float frame_duration,
                     sf::Vector2f scale_factor,
                     sf::Vector2f foothold)
    : sprite_(sprite),
      foothold_(foothold),
      state_(state),
      direction_(direction),
      frame_total_(frame_total),
      frame_start_(frame_start),
      frame_end_(frame_end),
      current_frame_(frame_start),
      frame_duration_(frame_duration),
      elapsed_(0.f),
      scale_factor_(scale_factor)
{
    if (!texture_.loadFromFile(texture_path)) {
        std::cerr << "Animation texture load failed: " << texture_path << std::endl;
    }

    // 根据总帧数设置初始纹理矩形
    sf::Vector2u texSize = texture_.getSize();
    int frameW = static_cast<int>(texSize.x) / frame_total_;
    int frameH = static_cast<int>(texSize.y);
    sprite_.setTextureRect(sf::IntRect({frame_start_ * frameW, 0},
                                       {frameW, frameH}));
    sprite_.setOrigin(foothold_);
}

void Animation::start() {
    // 绑定本动画自己的纹理
    sprite_.setTexture(texture_, true);
    sprite_.setScale(scale_factor_);
    current_frame_ = frame_start_;
    elapsed_ = 0.f;
    sprite_.setOrigin(foothold_);

    // 立即显示 start 帧
    sf::Vector2u texSize = texture_.getSize();
    int frameW = static_cast<int>(texSize.x) / frame_total_;
    int frameH = static_cast<int>(texSize.y);
    sprite_.setTextureRect(sf::IntRect({current_frame_ * frameW, 0},
                                       {frameW, frameH}));
}

void Animation::update(float dt) {

    elapsed_ += dt;
    if (elapsed_ >= frame_duration_) {
        elapsed_ -= frame_duration_;   // 保留余量，避免跳帧

        ++current_frame_;
        if (current_frame_ > frame_end_) {
            current_frame_ = frame_start_;
        }
        sf::Vector2u texSize = texture_.getSize();
        int frameW = static_cast<int>(texSize.x) / frame_total_;
        int frameH = static_cast<int>(texSize.y);
        sprite_.setTextureRect(sf::IntRect({current_frame_ * frameW, 0},
                                           {frameW, frameH}));
        sprite_.setOrigin(foothold_);
    }
}

