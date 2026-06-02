#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>

#include "Entity.h"
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

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    bool running = false;

    Entity pacman = 1;

    std::unordered_map<Entity, PositionComponent> positions;
    std::unordered_map<Entity, MovementComponent> movements;
    std::unordered_map<Entity, DrawingComponent> drawings;
    std::unordered_map<Entity, CollisionComponent> collisions;
    std::unordered_map<Entity, InputComponent> inputs;
};