#pragma once

#include <unordered_map>

#include "Entity.h"
#include "Components.h"
#include "bagel.h"

class BatterySystem {
public:
    void update(
        float deltaTime,
        bagel::Entity pacman
    );
};