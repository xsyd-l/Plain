#pragma once
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "../core/Entity.hpp"
#include "../core/Renderable.hpp"
#include <vector>

namespace BuildingID {
    enum index {
        tree,
        stone
    };
};

struct BuildingConfig {
    ShapeTag shape;

    float radius;
    sf::Vector2f halfsize;
    
    sf::Vector2f foothold;
    std::filesystem::path texture_path;
};

/**
 * Building
 * 在Enity之上增加了纹理，
 * 使用bid从注册表中初始化，
 * 可以派生出可交互建筑与不可交互建筑（无法被摧毁）
*/
class Building : public Entity, public Renderable {
private:
    sf::Texture texture_;
    sf::Sprite sprite_;
public:
    Building(sf::Vector2f center, sf::Vector2f halfsize, std::filesystem::path texture_path);

    Building(sf::Vector2f center,float radius, std::filesystem::path texture_path);

    void toScreen(float alpha = 0.f, float dt = 0.f) override;

    inline const sf::Drawable &getDrawable() const {
        return sprite_;
    }

    inline float getRenderOrder() const {
        return getCenter().y;
    }

    inline void setFootHold(sf::Vector2f foothold) {
        sprite_.setOrigin(foothold);
    }
};

void alignToMap(sf::Vector2f &position);

std::unique_ptr<Building> createBuilding(int index, sf::Vector2f center);
