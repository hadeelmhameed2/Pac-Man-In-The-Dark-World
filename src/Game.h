#pragma once

#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <vector>
#include <string>

#include "bagel.h"
#include "Components.h"

#include "InputSystem.h"
#include "MovementSystem.h"
#include "RenderSystem.h"
#include "BatterySystem.h"
#include "LightingSystem.h"
#include "ChargerSystem.h"

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
    void reset();
    void createChargers();

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    bool running = false;
    VisionMode visionMode = VisionMode::Full;
    bool wasBatteryAbove50 = true;

    SDL_Texture* mainMenuTexture = nullptr;
    SDL_Texture* gameOverTexture = nullptr;
    SDL_Texture* victoryTexture = nullptr;
    std::vector<std::string> initialMazeLayout;

    InputSystem inputSystem;
    MovementSystem movementSystem;
    RenderSystem renderSystem;
    BatterySystem batterySystem;
    LightingSystem lightingSystem;
    ChargerSystem chargerSystem;

    bagel::ent_type pacman{};
    bagel::ent_type gameStateId{};
    b2WorldId worldId{};
};
