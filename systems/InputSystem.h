#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>

#include "Entity.h"
#include "bagel.h"
#include "Components.h"

class InputSystem {
public:
    void handleInput(
    bool& running,
    bagel::Entity pacman,
    VisionMode& visionMode
);
};