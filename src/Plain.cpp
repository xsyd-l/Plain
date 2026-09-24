#include "../include/Game.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Plain");
    Game game(window);
    game.run();
    return 0;
}