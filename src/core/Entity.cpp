#include "../include/core/Entity.hpp"
#include "../include/core/collision.hpp"

Entity::Entity(sf::Vector2f center, sf::Vector2f halfsize, MoveTag movetag):
collider_(std::in_place_index<0>, center, halfsize), movetag_(movetag) {}

Entity::Entity(sf::Vector2f center, float radius, MoveTag movetag):
collider_(std::in_place_index<1>, center, radius), movetag_(movetag){}

bool Entity::containDot(sf::Vector2f dot) {
    return std::visit([dot](auto &obj){
        return obj.containDot(dot);
    }, collider_);
}

sf::Vector2f Entity::getCenter() const {
    return std::visit([](auto & collider){
        return collider.getCenter();
    }, collider_);
}

void Entity::setCenter(sf::Vector2f center) {
    std::visit([center](auto& collider){
        collider.setCenter(center);
    }, collider_);
}

std::optional<sf::Vector2f> Entity::getHalfSize() const {
    return std::visit([](auto& collider) -> std::optional<sf::Vector2f> {
        if constexpr (std::is_same_v<std::decay_t<decltype(collider)>, Collider_Rect>) {
            return collider.getHalfSize();
        }
        return std::nullopt;
    }, collider_);
}

void Entity::setHalfSize(sf::Vector2f halfsize) {
    std::visit([&](auto& collider) {
        if constexpr (std::is_same_v<std::decay_t<decltype(collider)>, Collider_Rect>) {
            collider.setHalfSize(halfsize);
        }
    }, collider_);
}

std::optional<float> Entity::getRadius() const {
    return std::visit([](auto &collider) -> std::optional<float> {
        if constexpr (std::is_same_v<std::decay_t<decltype(collider)>, Collider_Circle>) {
            return collider.getRadius();
        }
        return std::nullopt;
    }, collider_);
}

void Entity::setRadius(float radius) {
    std::visit([radius](auto &collider){
        if constexpr (std::is_same_v<std::decay_t<decltype(collider)>, Collider_Circle>) {
            collider.setRadius(radius);
        }
    }, collider_);
}

void Entity::moveCollider(sf::Vector2f delt_position) {
    std::visit([&](auto &collider){
        collider.move(delt_position);
    }, collider_);
}

void Entity::setCollider(sf::Vector2f position) {
    std::visit([&](auto &collider){
        collider.setCenter(position);
    }, collider_);
}