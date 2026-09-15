#include "../include/entities/Building.hpp"
#include <cmath>

Building::Building(sf::Vector2f center, sf::Vector2f halfsize, std::filesystem::path texture_path):
Entity(center, halfsize, MoveTag::immovable), sprite_(texture_){
    texture_.loadFromFile(texture_path);
    sprite_.setTexture(texture_, true);
}

Building::Building(sf::Vector2f center, float radius, std::filesystem::path texture_path):
Entity(center, radius, MoveTag::immovable), sprite_(texture_){
    texture_.loadFromFile(texture_path);
    sprite_.setTexture(texture_, true);
}

void alignToMap(sf::Vector2f &position) {
    position.x = (std::floor(position.x / 16.0f) * 16.0f) + 8.0f;
    position.y = (std::floor(position.y / 16.0f) * 16.0f) + 8.0f;
}

void Building::toScreen(float alpha, float dt) {
    sprite_.setPosition(getCenter());
}

std::unique_ptr<Building> createBuilding(int index, sf::Vector2f center) {
    static const std::vector<BuildingConfig> registry = [](){
        std::vector<BuildingConfig> tmp;
        BuildingConfig *config;
        config = &tmp.emplace_back();
        config->shape = ShapeTag::circle;
        config->radius = 8.f;
        config->texture_path = "./texture/tree.png";
        config->foothold = {24.f, 86.f};
        config = &tmp.emplace_back();
        config->shape = ShapeTag::circle;
        config->radius = 5.f;
        config->foothold = {9.f, 13.f};
        config->texture_path = "./texture/stone.png";
        return tmp;
    }();
    alignToMap(center);
    if (registry[index].shape == ShapeTag::rectangle) {
        std::unique_ptr<Building> building_ptr = std::make_unique<Building>(center, registry[index].halfsize, registry[index].texture_path);
        building_ptr->setFootHold(registry[index].foothold);
        return std::move(building_ptr);
    }
    else {
        std::unique_ptr<Building> building_ptr = std::make_unique<Building>(center, registry[index].radius, registry[index].texture_path);
        building_ptr->setFootHold(registry[index].foothold);
        return std::move(building_ptr);
    }
}
