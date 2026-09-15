#include "../include/controllers/Controller.hpp"

Controller::Controller(ControllerType type, Character* character)
    : type_(type), character_(character) {
}

Controller::~Controller() {
    delete character_;
}