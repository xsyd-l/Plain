#include "../include/core/collision.hpp"
#include <variant>
#include <SFML/System/Vector2.hpp>
#include "../include/core/VectorMath.hpp"
#include <algorithm>

Collider_Rect::Collider_Rect(sf::Vector2f center, sf::Vector2f halfsize) {
    center_ = center;
    halfsize_ = halfsize;
}

bool Collider_Rect::containDot(sf::Vector2f dot) const {
    return 
    dot.x <= center_.x + halfsize_.x &&
    dot.x >= center_.x - halfsize_.x &&
    dot.y <= center_.y + halfsize_.y &&
    dot.y >= center_.y - halfsize_.y;
}

Collider_Circle::Collider_Circle(sf::Vector2f center, float radius) {
    center_ = center;
    radius_ = radius;
}

bool Collider_Circle::containDot(sf::Vector2f dot) const {
    return
    (dot.y - center_.y) * (dot.y - center_.y) +
    (dot.x - center_.x) * (dot.x - center_.x) <=
    radius_ * radius_;
}

struct sprt {
    sf::Vector2f operator() (const Collider_Rect &cr, const Collider_Circle &cc) {
        if (cr.containDot(cc.getCenter())) {
                sf::Vector2f diff = cc.getCenter() - cr.getCenter();
                float overlapX = cr.getHalfSize().x - std::abs(diff.x);
                float overlapY = cr.getHalfSize().y - std::abs(diff.y);
                float moveX = cc.getRadius() + overlapX;
                float moveY = cc.getRadius() + overlapY;
            if (moveX <= moveY) {
                float sign = (diff.x < 0.0f) ? 1.0f : -1.0f;
                return sf::Vector2f(sign * moveX, 0.0f);
            } else {
                float sign = (diff.y < 0.0f) ? 1.0f : -1.0f;
                return sf::Vector2f(0.0f, sign * moveY);
            }
        }
        else {
            sf::Vector2f closetDot(std::clamp(cc.getCenterX(), cr.left(), cr.right()),
            std::clamp(cc.getCenterY(), cr.bottom(), cr.top()));
            if (
                vMod_p2(closetDot - cc.getCenter()) >= cc.getRadius() * cc.getRadius()
            ) {
                return sf::Vector2f(0.f, 0.f);
            }
            sf::Vector2f dist = closetDot - cc.getCenter();
            float dist_mod = vMod(dist);
            return ((cc.getRadius() - dist_mod) / dist_mod) * dist;
        }
    }

    sf::Vector2f operator() (const Collider_Circle &cc, const Collider_Rect &cr) {
        return -1.f * (*this)(cr, cc);
    }

    sf::Vector2f operator() (const Collider_Rect &cr1, const Collider_Rect &cr2) {
        float overlapL = cr1.right() - cr2.left();
        float overlapR = cr2.right() - cr1.left();
        float overlapB = cr1.top() - cr2.bottom();
        float overlapT = cr2.top() - cr1.bottom();
        if (
            overlapL <= 0 ||            
            overlapR <= 0 ||
            overlapB <= 0 ||
            overlapT <= 0            
        ) {
            return sf::Vector2f(0.f,0.f);
        }
        float X = overlapL <= overlapR ? -overlapL : overlapR, Y = overlapB <= overlapT ? -overlapB : overlapT;
        if (std::abs(X) <= std::abs(Y)) {
            return sf::Vector2f(X, 0.f);
        }
        else {
            return sf::Vector2f(0.f, Y);
        }
    }

    sf::Vector2f operator() (const Collider_Circle &cc1, const Collider_Circle &cc2) {
        if (
            vMod_p2(cc1.getCenter() - cc2.getCenter()) >=
            (cc1.getRadius() + cc2.getRadius()) * (cc1.getRadius() + cc2.getRadius())
        ) {
            return sf::Vector2f(0.f, 0.f);
        }
        
        if (cc1.getCenter() != cc2.getCenter()) {
            return 
            (cc1.getCenter() - cc2.getCenter()) *
            ((cc1.getRadius() + cc2.getRadius()) /
            vMod(cc1.getCenter() - cc2.getCenter()) - 1.f);
        }
        else {
            return sf::Vector2f(cc1.getRadius() + cc2.getRadius(), 0.f);
        }
    }
};

struct itrsct {
    bool operator() (const Collider_Rect &cr, const Collider_Circle &cc) {
        return 
        cr.containDot(cc.getCenter()) ||
        vMod_p2(sf::Vector2f(std::clamp(cc.getCenterX(), cr.left(), cr.right()),
        std::clamp(cc.getCenterY(), cr.bottom(), cr.top())) - cc.getCenter()) <
        cc.getRadius() * cc.getRadius();
    }
    
    bool operator() (const Collider_Circle &cc, const Collider_Rect &cr) {
        return (*this)(cr, cc);
    }
    
    bool operator() (const Collider_Rect &cr1, const Collider_Rect &cr2) {
        return !(
            cr1.right() <= cr2.left() ||
            cr1.left() >= cr2.right() ||
            cr1.top() <= cr2.bottom() ||
            cr1.bottom() >= cr2.top()
        );
    }
    
    bool operator() (const Collider_Circle &cc1, const Collider_Circle &cc2) {
        return 
        vMod_p2(cc1.getCenter() - cc2.getCenter()) <
        (cc1.getRadius() + cc2.getRadius()) * (cc1.getRadius() + cc2.getRadius());
    }
};

sf::Vector2f sprtAFrmB(const collider &A,const collider &B) {
    return std::visit(sprt(), A, B);
}

bool itrsctng(const collider &A,const collider &B) {
    return std::visit(itrsct(), A, B);
}