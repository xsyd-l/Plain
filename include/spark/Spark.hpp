#pragma once
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

void (*addSpark)(SparkType);

enum class SparkColor {
    green,
    red
};

enum class SparkType {
    damage,
    heal
};


class SparkSoA {
public:
    SparkSoA();
    
    void addSpark(SparkType type);
private:
    std::vector<float> velocities_;
    std::vector<float> accelerations_;
    std::vector<sf::Text> texts_;
    std::vector<sf::Vector2i> directions_;
    std::vector<float> rotates_;
    std::vector<SparkColor> colors_;
};