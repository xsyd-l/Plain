#pragma once
#include <SFML/System/Vector2.hpp>
#include <variant>

class Collider {
public:
    virtual bool containDot(sf::Vector2f dot) const = 0;
};

class Collider_Rect;

class Collider_Circle;

using collider = std::variant<Collider_Rect, Collider_Circle>;

class Collider_Rect : public Collider {
private:
    sf::Vector2f center_;
    sf::Vector2f halfsize_;
public:
    Collider_Rect(sf::Vector2f center, sf::Vector2f halfsize);

    bool containDot(sf::Vector2f dot) const;

    inline sf::Vector2f getCenter() const {
        return center_;
    }

    inline float getCenterX() const {
        return center_.x;
    }

    inline float getCenterY() const {
        return center_.y;
    }

    inline void setCenter(sf::Vector2f center) {
        center_ = center;
    }

    inline void move(sf::Vector2f delt_position) {
        center_ += delt_position;
    }

    inline sf::Vector2f getHalfSize() const {
        return halfsize_;
    }

    inline void setHalfSize(sf::Vector2f halfsize) {
        halfsize_ = halfsize;
    }

    inline float left() const {
        return center_.x - halfsize_.x;
    }

    inline float right() const {
        return center_.x + halfsize_.x;
    }

    inline float bottom() const {
        return center_.y - halfsize_.y;
    }

    inline float top() const {
        return center_.y + halfsize_.y;
    }
};

class Collider_Circle : public Collider {
private:
    sf::Vector2f center_;
    float radius_;
public:
    Collider_Circle(sf::Vector2f center, float radius);

    bool containDot(sf::Vector2f dot) const;

    inline sf::Vector2f getCenter() const {
        return center_;
    }

    inline void move(sf::Vector2f delt_position) {
        center_ += delt_position;
    }
    
    inline float getCenterX() const {
        return center_.x;
    }

    inline float getCenterY() const {
        return center_.y;
    }

    inline void setCenter(sf::Vector2f center) {
        center_ = center;
    }

    inline float getRadius() const {
        return radius_;
    }

    inline void setRadius(float radius) {
        radius_ = radius;
    }
};

sf::Vector2f sprtAFrmB(const collider &A,const collider &B);

bool itrsctng(const collider &A,const collider &B);