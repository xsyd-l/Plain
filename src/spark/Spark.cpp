#include "../../include/spark/Spark.hpp"
#include <queue>
#include <deque>
#include <iostream>
#include <random>

namespace {

// 同一帧内可能产生多个火花，用随机的水平初速度把它们散开，避免数字互相重叠
float randomSpread(float amplitude) {
    static thread_local std::mt19937 rng{std::random_device{}()};
    static thread_local std::uniform_real_distribution<float> dist{-1.f, 1.f};
    return dist(rng) * amplitude;
}

sf::Color toColor(SparkColor color) {
    switch (color) {
        case SparkColor::green:  return sf::Color(90, 220, 100);  // 治疗
        case SparkColor::yellow: return sf::Color(255, 205, 60);  // 暴击
        case SparkColor::red:    return sf::Color(235, 70, 70);   // 普通伤害
    }
    return sf::Color::White;
}

} // namespace

SparkSoA::SparkSoA(std::filesystem::path font_path) {
    if (!font_.openFromFile(font_path)) {
        std::cout<<"error:connot load font";
        exit(0);
    }
}

void SparkSoA::spawnSpark(SparkType type, std::string content, sf::Vector2f position) {
    // 单位说明：
    //   life / point 的单位是物理步（SPARK_UPDATE_DT ≈ 1/60 秒）；
    //   velocity 的单位是"像素/步"，acceleration 的单位是"像素/步²"。
    // 先给一套默认值（普通伤害的手感），再按类型覆盖。
    unsigned int character_size = 26;            // 字号：暴击 > 普通伤害 > 治疗
    SparkColor   color          = SparkColor::red;
    int          life           = 48;            // 总寿命 ≈ 0.8s
    int          point          = 12;            // 最后 12 步播放"缩小到消失"动画
    sf::Vector2f velocity       = {0.f, -2.9f};  // 初速度：向上弹出
    sf::Vector2f acceleration   = {0.f, 0.12f};  // 恒定加速度：模拟重力
    float        spread         = 0.45f;         // 水平散开幅度（像素/步）

    switch (type) {
        case SparkType::heal: {
            // 治疗：绿色、字号略小、缓慢上浮（几乎不受重力影响），散开最小
            color          = SparkColor::green;
            character_size = 24;
            life           = 45;    // 0.75s
            point          = 15;    // 最后 0.25s 缩小消失
            velocity       = {0.f, -1.2f};
            acceleration   = {0.f, -0.01f};
            spread         = 0.15f;
            break;
        }
        case SparkType::damage: {
            // 普通伤害：红色、向上弹出后受重力落回原位（抛物线落点与起点重合）
            color          = SparkColor::red;
            character_size = 26;
            life           = 48;    // 0.8s
            point          = 12;    // 最后 0.2s 缩小消失
            velocity       = {0.f, -2.9f};
            acceleration   = {0.f, 0.12f};
            spread         = 0.45f;
            break;
        }
        case SparkType::damage_crit: {
            // 暴击：黄色、字号更大、初速度更快、重力更强，飞得更高更远
            color          = SparkColor::yellow;
            character_size = 32;
            life           = 50;    // ≈0.83s
            point          = 16;    // 最后 ≈0.27s 缩小消失
            velocity       = {0.f, -4.0f};
            acceleration   = {0.f, 0.16f};
            spread         = 0.8f;
            break;
        }
        default: {
            break;
        }
    }

    // 构造时即绑定字体与内容（sf::Text 在 SFML 3 没有默认构造）
    spawn_queue_.emplace(font_, content, character_size);
    auto& spark = spawn_queue_.back();

    spark.life         = life;
    spark.point        = point;
    spark.zoom         = 1.0f;   // 必须为 1.0：消失动画按 1/point 递减，才能正好在 life 归零时缩到 0
    spark.color        = color;
    spark.velocity     = velocity;
    spark.velocity.x  += randomSpread(spread);
    spark.acceleration = acceleration;
    spark.direction    = {0.f, -1.f};
    spark.rotate       = 0.f;
    spark.dcceleration = {0.f, 0.f};

    spark.text.setFillColor(toColor(color));
    spark.text.setOutlineColor(sf::Color(0, 0, 0, 180)); // 黑描边：保证在任何背景上都可读
    spark.text.setOutlineThickness(2.f);
    // 以文字中心为原点，让死亡动画的缩放围绕数字中心进行
    const sf::FloatRect bounds = spark.text.getLocalBounds();
    spark.text.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
    spark.text.setPosition(position);
}

void SparkSoA::updateSpark() {
    // 遍历deque，更新活跃的spark，并使用等待的spark填补空位置
    for (auto& spark: active_deque_) {
        if (spark.life > 0) {
            spark.life--;
            //速度积分
            spark.velocity += spark.acceleration;
            //位移积分
            spark.text.move(spark.velocity);
            if (spark.life <= spark.point) {
                spark.zoom -= (float)1/ spark.point;
                if (spark.zoom < 0) {spark.zoom = 0;}
                spark.text.setScale({spark.zoom, spark.zoom});
            }
        } else {
            // 当一个spark的生命小于0时，若存在等待的spark，就替换掉它
            if (spawn_queue_.size() > 0) {

            }
        }
        // 当所有的空位都被填补了还存在等待的spark时,把queue的所有队首元素添加到deque的队位
        while (!spawn_queue_.empty()) {
            active_deque_.push_back(spawn_queue_.front());
            spawn_queue_.pop();
        }
    }
}

void SparkSoA::draw(sf::RenderWindow& window) {
    for (auto &spark: active_deque_) {
        window.draw(spark.text);
    }
}