#pragma once

#include <SDL3/SDL.h>

#include "bagel.h"
#include "Components.h"

class Game {
public:
    bool init();
    void run();
    void clean();

private:
    void handleEvents();
    void update(float deltaTime);
    void render();

    void createPacman();
    void createGameState();
    void createGhosts();
    void updateSystems(float deltaTime);

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    bool running = false;

    bagel::ent_type pacman{};
    bagel::ent_type gameStateId{};
};
