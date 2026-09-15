#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "../entities/Character.hpp"

enum ControllerType {
    player,
    ai
};

struct Intention {
    ControllerType victimType;
    bool attack;
    sf::Vector2f accele;
    sf::Vector2f aimDir;
    inline void setZero() {
        accele = {0.f, 0.f};
        attack = false;
    }
};

class Controller {
private:
    ControllerType type_;
    Character *character_;
    Intention intention_;
public:
    Controller(ControllerType type, Character* character);
    
    virtual void update() = 0;

    inline ControllerType getType() const {
        return type_;
    }
    
    inline Character *getCharacter() {
        return character_;
    }
    
    inline Intention &getIntention() {
        return intention_;
    }
    
    virtual ~Controller();
};
