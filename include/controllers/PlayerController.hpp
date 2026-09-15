#pragma once
#include "Controller.hpp"
#include "../entities/Character.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

class PlayerController : public Controller {
private:
    sf::RenderWindow &window_;
public:
    PlayerController(Character* character, sf::RenderWindow &window);

    void update();
};