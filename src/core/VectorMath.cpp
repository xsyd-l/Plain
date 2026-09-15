#include "../include/core/VectorMath.hpp"
#include <cmath>

float vMod(sf::Vector2f vector) {
    return sqrt(vector.x * vector.x + vector.y * vector.y);
}