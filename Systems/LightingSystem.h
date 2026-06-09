#pragma once

#include <SDL3/SDL.h>
#include "bagel.h"
#include "Components.h"
#include "../ecs/LightingComponents.h"

// LightingSystem is responsible for computing which entities are lit by the
// player (and other light sources) and updating their VisibilityComponent
// values. All logic lives in this system; components remain POD.
class LightingSystem {
public:
    // Optional hook for the renderer. Student 1 can call this once after the
    // SDL_Renderer is created so the lighting system can own a shadow-mask
    // texture without exposing any extra global state.
    void setRenderer(SDL_Renderer* renderer);

    // Update lighting and visibility for the frame. deltaTime is seconds.
    void update(float deltaTime);

    // Returns the internally managed shadow mask texture. Student 1 can draw
    // it over the scene with SDL_RenderTexture(...).
    SDL_Texture* getShadowMaskTexture() const;
};

