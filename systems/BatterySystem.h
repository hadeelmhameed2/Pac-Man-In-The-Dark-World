#pragma once

#include <unordered_map>

#include "Entity.h"
#include "Components.h"

class BatterySystem {
public:
    void update(
        float deltaTime,
        Entity pacman,
        std::unordered_map<Entity, BatteryLifeComponent>& batteries,
        std::unordered_map<Entity, FlashlightComponent>& flashlights
    );
};