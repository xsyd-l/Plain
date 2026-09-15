#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics/Drawable.hpp>

class Renderable {
public:
    virtual void toScreen(float alpha = 0.f, float dt = 0.f) = 0;
    virtual const sf::Drawable &getDrawable() const = 0;
    virtual float getRenderOrder() const = 0;
};