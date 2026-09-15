#pragma once
#include "collision.hpp"
#include <optional>

enum class ShapeTag {
    rectangle,
    circle
};

enum class MoveTag {
    immovable,
    movable
};

/**
 * Entity
 * 所有事物的基础
*/
class Entity
{
private:
    collider collider_;
    MoveTag movetag_;
public:
    //方形实体
    Entity(sf::Vector2f center, sf::Vector2f halfsize, MoveTag movetag);
    //圆形实体
    Entity(sf::Vector2f center, float radius, MoveTag movetag);

    bool containDot(sf::Vector2f);

    inline const collider& getCollider() const {
        return collider_;
    }

    inline void setCollider(collider &collider) {
        collider_ = collider;
    }

    inline MoveTag getMoveTag() const {
        return movetag_;
    }

    inline void setMoveTag(MoveTag &movetag) {
        movetag_ = movetag;
    }

    sf::Vector2f getCenter() const;

    void setCenter(sf::Vector2f center);
    //如果是方形碰撞箱体就返回halfsize,圆形则返回{r, r}
    std::optional<sf::Vector2f> getHalfSize() const;

    void setHalfSize(sf::Vector2f halfsize);

    std::optional<float> getRadius() const;

    void setRadius(float radius);

protected:
    void moveCollider(sf::Vector2f delt_position);

    void setCollider(sf::Vector2f position);
};