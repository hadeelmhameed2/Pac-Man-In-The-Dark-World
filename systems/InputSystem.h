#pragma once

#include <SDL3/SDL.h>

#include "bagel.h"
#include "Components.h"

class InputSystem {
public:
    void handleInput(
        bool& running,
        VisionMode& visionMode,
        bool& resetRequested
    );
};