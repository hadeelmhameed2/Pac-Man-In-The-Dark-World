#pragma once

#include <unordered_map>

#include "Entity.h"
#include "Components.h"
#include "bagel.h"

class MovementSystem {
public:
    void update(
        float deltaTime
    );
};