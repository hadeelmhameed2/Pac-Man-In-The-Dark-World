#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>

#include "Entity.h"
#include "Components.h"

class InputSystem {
public:
    void handleInput(
        bool& running,
        Entity pacman,
        std::unordered_map<Entity, MovementComponent>& movements,
        std::unordered_map<Entity, DirectionComponent>& directions,
        std::unordered_map<Entity, FlashlightComponent>& flashlights,
        std::unordered_map<Entity, BatteryLifeComponent>& batteries,
        VisionMode& visionMode
    );
};