#pragma once
#include <SFML/System/Vector2.hpp>
#include <cmath>

/**
 * Vector相关的数学运算集合
 * 
*/
inline sf::Vector2f operator*(sf::Vector2f &vector, float scalar) {
    return sf::Vector2f(vector.x * scalar, vector.y * scalar);
}

inline sf::Vector2f operator*(float scalar, sf::Vector2f &vector) {
    return sf::Vector2f(scalar * vector.x, scalar * vector.y);
}

inline sf::Vector2f operator/(sf::Vector2f &vector, float scalar) {
    return sf::Vector2f(scalar / vector.x, scalar / vector.y);
}

inline float vMod_p2(sf::Vector2f vector) {
    return vector.x * vector.x + vector.y * vector.y;
}

float vMod(sf::Vector2f vector);

inline float operator*(sf::Vector2f v1, sf::Vector2f v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

inline float angleUnsign(sf::Vector2f v1, sf::Vector2f v2) {
    return std::acos(v1 * v2 / (sqrt(vMod_p2(v1) * vMod_p2(v2))));
}