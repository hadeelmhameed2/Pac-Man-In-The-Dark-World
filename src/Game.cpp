#include "Game.h"
#include "Constants.h"

#include <cmath>
#include <iostream>

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
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
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
    bagel::Entity entity = bagel::Entity::create();
    pacman = entity.entity();

    entity.add(PositionComponent{ 10.0f * TILE_SIZE, 14.0f * TILE_SIZE });
    entity.add(MovementComponent{ 0.0f, 0.0f, PACMAN_SPEED });
    entity.add(DrawingComponent{ TILE_SIZE, TILE_SIZE, 255, 255, 0, 255 });
    entity.add(CollisionComponent{ TILE_SIZE, TILE_SIZE, false });
    entity.add(InputComponent{ true });
}

void Game::createGhosts() {
    spawnGhost(0, 12.0f * TILE_SIZE, 9.0f * TILE_SIZE, 0.0f, -GHOST_SPEED, 255, 0, 0);
    spawnGhost(1, 12.0f * TILE_SIZE, 11.0f * TILE_SIZE, 0.0f, GHOST_SPEED, 255, 182, 193);
    spawnGhost(2, 10.0f * TILE_SIZE, 10.0f * TILE_SIZE, -GHOST_SPEED, 0.0f, 0, 255, 255);
    spawnGhost(3, 14.0f * TILE_SIZE, 10.0f * TILE_SIZE, GHOST_SPEED, 0.0f, 255, 165, 0);
}

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

        handleEvents();
        update(deltaTime);
        render();

        SDL_Delay(FRAME_DELAY_MS);
    }
}

void Game::handleEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                running = false;
            }

            auto& movement = bagel::World::getComponent<MovementComponent>(pacman);

            if (event.key.key == SDLK_RIGHT) {
                movement.vx = movement.speed;
                movement.vy = 0.0f;
            }
            else if (event.key.key == SDLK_LEFT) {
                movement.vx = -movement.speed;
                movement.vy = 0.0f;
            }
            else if (event.key.key == SDLK_UP) {
                movement.vx = 0.0f;
                movement.vy = -movement.speed;
            }
            else if (event.key.key == SDLK_DOWN) {
                movement.vx = 0.0f;
                movement.vy = movement.speed;
            }
        }
    }
}

void Game::update(float deltaTime) {
    updateSystems(deltaTime);

    static const bagel::Mask moveMask = bagel::MaskBuilder()
        .set<PositionComponent>()
        .set<MovementComponent>()
        .build();
    static int moveQ = bagel::World::createQuery(moveMask);

    for (bagel::Entity e = bagel::World::first(moveQ); !bagel::World::eof(moveQ); e = bagel::World::next(moveQ)) {
        auto& position = e.get<PositionComponent>();
        auto& movement = e.get<MovementComponent>();

        enforceCardinalMovement(movement);

        position.x += movement.vx * deltaTime;
        position.y += movement.vy * deltaTime;

        snapEntityToGridLane(position, movement);

        const auto& drawing = e.get<DrawingComponent>();
        clampToScreen(position, drawing.width, drawing.height);
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    const auto& position = bagel::World::getComponent<PositionComponent>(pacman);
    const auto& drawing = bagel::World::getComponent<DrawingComponent>(pacman);
    SDL_FRect pacmanRect = { position.x, position.y, static_cast<float>(drawing.width), static_cast<float>(drawing.height) };
    SDL_SetRenderDrawColor(renderer, drawing.r, drawing.g, drawing.b, drawing.a);
    SDL_RenderFillRect(renderer, &pacmanRect);

    static const bagel::Mask renderMask = bagel::MaskBuilder()
        .set<PositionComponent>()
        .set<DrawingComponent>()
        .set<VisibilityComponent>()
        .build();
    static int q = bagel::World::createQuery(renderMask);

    for (bagel::Entity e = bagel::World::first(q); !bagel::World::eof(q); e = bagel::World::next(q)) {
        const auto& vis = e.get<VisibilityComponent>();
        const auto& pos = e.get<PositionComponent>();
        const auto& draw = e.get<DrawingComponent>();

        SDL_FRect entRect = { pos.x, pos.y, static_cast<float>(draw.width), static_cast<float>(draw.height) };

        const float drawOpacity = std::max(GHOST_MIN_OPACITY, vis.opacity);
        Uint8 alpha = static_cast<Uint8>(drawOpacity * 255.0f);
        SDL_SetRenderDrawColor(renderer, draw.r, draw.g, draw.b, alpha);

        SDL_RenderFillRect(renderer, &entRect);
    }

    SDL_RenderPresent(renderer);
}

void Game::clean() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}
