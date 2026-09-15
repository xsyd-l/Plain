#pragma once
#include <SFML/Graphics.hpp>

class Menu {
private:
    sf::RenderWindow& window_;
public:
    Menu(sf::RenderWindow& window);
    void run();
};