#pragma once
#include "Entity.hpp"
#include "Renderable.hpp"

class GameObject : public Entity, public Renderable {
public:
    using Entity::Entity;
    
    virtual ~GameObject() = default;
};
