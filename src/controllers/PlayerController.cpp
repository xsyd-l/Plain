#include "../include/controllers/PlayerController.hpp"

PlayerController::PlayerController(Character* character, sf::RenderWindow &window) : Controller(ControllerType::player, character), window_(window) {
    getIntention().victimType = ControllerType::ai;
}

void PlayerController::update() {
    getIntention().attack = false;
    getIntention().accele = {0.f, 0.f};
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        getIntention().attack = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
        getIntention().accele += {0.f, -1.f};
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
        getIntention().accele += {-1.f, 0.f};
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
        getIntention().accele += {0.f, 1.f};
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
        getIntention().accele += {1.f, 0.f};
    }
    getIntention().aimDir = window_.mapPixelToCoords(sf::Mouse::getPosition(window_)) - getCharacter()->getCenter();
}