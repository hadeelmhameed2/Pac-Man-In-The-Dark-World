#pragma once

#include <SDL3/SDL.h>

#include "bagel.h"
#include "Components.h"

#include "InputSystem.h"
#include "MovementSystem.h"
#include "RenderSystem.h"
#include "BatterySystem.h"

class Game {
public:
    bool init();
    void run();
    void clean();

private:
    void update(float deltaTime);
    void createPacman();
    void createGameState();
    void createGhosts();
    void updateSystems(float deltaTime);
    void applyGhostGridMovement();

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    bool running = false;
    VisionMode visionMode = VisionMode::Full;

    InputSystem inputSystem;
    MovementSystem movementSystem;
    RenderSystem renderSystem;
    BatterySystem batterySystem;

    bagel::ent_type pacman{};
    bagel::ent_type gameStateId{};
};
