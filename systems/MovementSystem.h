#pragma once

#include <unordered_map>

#include "Entity.h"
#include "Components.h"

class MovementSystem {
public:
    void update(
        float deltaTime,
        std::unordered_map<Entity, PositionComponent>& positions,
        std::unordered_map<Entity, MovementComponent>& movements,
        std::unordered_map<Entity, CollisionComponent>& collisions,
        int windowWidth,
        int windowHeight
    );
};