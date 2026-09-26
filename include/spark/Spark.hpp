#pragma once
#include <filesystem>
#include <functional>
#include <string>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <deque>
#include <queue>

#define SPARK_UPDATE_DT 0.0167F

// 代码说明：
// 只能实现两段动画函数，并且只允许定义第一段，
// 即弹幕的出现效果，所有弹幕的消失行为都默认为消失成一个点,
// 为了避免开发给其它类过多权限，以及在添加弹幕时搜索空位置的性能消耗，
// 使用了spawn_queue_和active_queue两种队列
// 系统在更新active_queue_时，如果遇到了一个弹幕已经死亡，就在spawn_queue_取出队首替换掉它，
// 当走到active_queue_的队尾时就添加完所有spawm_queue_中的等待弹幕
// 伤害数字的抛体效果也使用物理步更新，避免因为不同设备的性能差异产生火花物理效果的变化，
// 但是不再追求插值了，因为要为每个火花记录privious_position和current_position，
// 太浪费内存和性能了

enum class SparkColor {
    green,
    yellow,
    red
};

enum class SparkType {
    damage,
    damage_crit,
    heal
};

// 开放的全局添加接口：任何模块只要 include 本头文件即可产生火花，
// 无需持有 SparkSoA 实例（主循环里把它指向自己的实现即可）。
// 声明必须位于 SparkType 之后，且写成 inline 以避免多个翻译单元重复定义。
inline std::function<void(SparkType, std::string, sf::Vector2f)> spawnSpark;

struct Spark {
    // sf::Text 在 SFML 3 中没有默认构造函数，必须在构造时绑定字体与内容
    Spark(const sf::Font& font, const std::string& content, unsigned int character_size)
        : text(font, content, character_size) {}

    int life = 0; // life的计量单位是物理步更新次数
    int point = 0;// 过point时，停止计算物理，开始计算死亡动画
    float zoom = 1.0f; //管理大小变化的缩放
    sf::Vector2f velocity;
    sf::Vector2f dcceleration; // 保留字段：当前 updateSpark 尚未使用
    sf::Vector2f acceleration;
    sf::Text text;
    sf::Vector2f direction;    // 保留字段：当前 updateSpark 尚未使用
    float rotate = 0.f;        // 保留字段：当前 updateSpark 尚未使用
    SparkColor color = SparkColor::red;
};

class SparkSoA {
public:
    float accumulator_ = 0.0f;
    SparkSoA(std::filesystem::path font_path);
    void spawnSpark(SparkType type, std::string content, sf::Vector2f position);
    void updateSpark();
    void draw(sf::RenderWindow& window);
private:
    std::queue<Spark> spawn_queue_;
    std::deque<Spark> active_deque_;
    sf::Font font_;
};