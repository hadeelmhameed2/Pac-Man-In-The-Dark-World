#include "Game.h"
#include "Constants.h"

#include <cmath>
#include <iostream>
#include <string>

#include "GridMovement.h"
#include "GameStateSystem.h"
#include "GhostAISystem.h"
#include "VisibilitySystem.h"

namespace {
    SDL_FPoint scatterCorner(int ghostType) {
        switch (ghostType) {
            case 0: return { WINDOW_WIDTH - TILE_SIZE, TILE_SIZE };
            case 1: return { TILE_SIZE, TILE_SIZE };
            case 2: return { WINDOW_WIDTH - TILE_SIZE, WINDOW_HEIGHT - TILE_SIZE };
            default: return { TILE_SIZE, WINDOW_HEIGHT - TILE_SIZE };
        }
    }

    void spawnGhost(
        int ghostType,
        float x, float y,
        float vx, float vy,
        Uint8 r, Uint8 g, Uint8 b
    ) {
        bagel::Entity ghost = bagel::Entity::create();
        ghost.add(PositionComponent{ x, y });
        ghost.add(MovementComponent{ vx, vy, GHOST_SPEED });
        ghost.add(DrawingComponent{ TILE_SIZE, TILE_SIZE, r, g, b, 255 });
        ghost.add(GhostAI{ GhostState::SCATTER, scatterCorner(ghostType), ghostType });
        ghost.add(VisibilityComponent{ true, GHOST_MIN_OPACITY });
    }
}

bool Game::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(
        "Pacman in the Dark World",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    if (!window) {
        std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        window = nullptr;
        SDL_Quit();
        return false;
    }

    createGameState();
    createPacman();
    createGhosts();

    running = true;
    return true;
}

void Game::createGameState() {
    bagel::Entity stateEnt = bagel::Entity::create();
    stateEnt.add(GameStateComponent{ 100.0f, 0, false, false, 0 });
    gameStateId = stateEnt.entity();
}

void Game::createPacman() {

    auto pacman = bagel::Entity::create();

    pacman.addAll(
        BatteryLifeComponent{100.0f,100.0f,0.3f,1.2f},
        FlashlightComponent{false,true},
        DirectionComponent{Direction::Right},
        InputComponent{true},
        PositionComponent{388.0f,690.0f},
        MovementComponent{0.0f,0.0f,160.0f},
        DrawingComponent{32,32,255,255,0,255},
        CollisionComponent{32,32,false}
    );

    std::cout << "\n=== PACMAN COMPONENTS ===\n";
void Game::createGhosts() {
    spawnGhost(0, 12.0f * TILE_SIZE, 9.0f * TILE_SIZE, 0.0f, -GHOST_SPEED, 255, 0, 0);
    spawnGhost(1, 12.0f * TILE_SIZE, 11.0f * TILE_SIZE, 0.0f, GHOST_SPEED, 255, 182, 193);
    spawnGhost(2, 10.0f * TILE_SIZE, 10.0f * TILE_SIZE, -GHOST_SPEED, 0.0f, 0, 255, 255);
    spawnGhost(3, 14.0f * TILE_SIZE, 10.0f * TILE_SIZE, GHOST_SPEED, 0.0f, 255, 165, 0);
}

    if (pacman.has<PositionComponent>())
    {
        auto& p = pacman.get<PositionComponent>();
        std::cout
            << "Position: x=" << p.x
            << " y=" << p.y
            << std::endl;
    }

    if (pacman.has<MovementComponent>())
    {
        auto& m = pacman.get<MovementComponent>();
        std::cout
            << "Movement: vx=" << m.vx
            << " vy=" << m.vy
            << " speed=" << m.speed
            << std::endl;
    }

    if (pacman.has<DrawingComponent>())
    {
        auto& d = pacman.get<DrawingComponent>();
        std::cout
            << "Drawing: w=" << d.width
            << " h=" << d.height
            << " rgba=("
            << (int)d.r << ","
            << (int)d.g << ","
            << (int)d.b << ","
            << (int)d.a << ")"
            << std::endl;
    }

    if (pacman.has<CollisionComponent>())
    {
        auto& c = pacman.get<CollisionComponent>();
        std::cout
            << "Collision: w=" << c.width
            << " h=" << c.height
            << " solid=" << c.isSolid
            << std::endl;
    }

    if (pacman.has<DirectionComponent>())
    {
        auto& dir = pacman.get<DirectionComponent>();
        std::cout
            << "Direction: "
            << static_cast<int>(dir.current)
            << std::endl;
    }

    if (pacman.has<FlashlightComponent>())
    {
        auto& f = pacman.get<FlashlightComponent>();
        std::cout
            << "Flashlight: on=" << f.isOn
            << " available=" << f.isAvailable
            << std::endl;
    }

    if (pacman.has<BatteryLifeComponent>())
    {
        auto& b = pacman.get<BatteryLifeComponent>();
        std::cout
            << "Battery: current=" << b.current
            << " max=" << b.max
            << " drain=" << b.normalDrainPerSecond
            << " flashlightDrain=" << b.flashlightDrainPerSecond
            << std::endl;
    }

    if (pacman.has<InputComponent>())
    {
        auto& i = pacman.get<InputComponent>();
        std::cout
            << "Input: controlled="
            << i.controlledByPlayer
            << std::endl;
    }

    std::cout << "=========================\n";

void Game::updateSystems(float deltaTime) {
    GameStateSystem::update(pacman, deltaTime);
    GhostAISystem::update(pacman, deltaTime);
    VisibilitySystem::update(pacman, gameStateId, deltaTime);
}

void Game::run() {
    Uint64 previousTime = SDL_GetTicks();

    while (running) {
        Uint64 currentTime = SDL_GetTicks();
        float deltaTime = static_cast<float>(currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;

        inputSystem.handleInput(
            running,
            visionMode
        );

        renderSystem.drawStatus(window,visionMode);
        update(deltaTime);
        render();

        SDL_Delay(FRAME_DELAY_MS);
    }
}

void Game::update(float deltaTime) {


    batterySystem.update(
        deltaTime
    );

    movementSystem.update(
        deltaTime
    );


}

void Game::render() {
    renderSystem.render(
        renderer,
        visionMode
    );
}

void Game::clean() {
    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}
