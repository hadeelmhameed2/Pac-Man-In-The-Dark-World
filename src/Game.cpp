#include "Game.h"
#include "Constants.h"
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
        return false;
    }

    window = SDL_CreateWindow(
        "Pacman in the Dark World",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    if (!window) {
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
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
    bagel::Entity entity = bagel::Entity::create();
    pacman = entity.entity();

    entity.addAll(
        BatteryLifeComponent{ 100.0f, 100.0f, 0.3f, 1.2f },
        FlashlightComponent{ false, true },
        DirectionComponent{ Direction::Right },
        InputComponent{ true },
        PositionComponent{ 378.0f, 723.0f },
        MovementComponent{ 0.0f, 0.0f, 160.0f },
        DrawingComponent{ 32, 32, 255, 255, 0, 255 },
        CollisionComponent{ 32, 32, false }
    );
}

void Game::createGhosts() {
    // Spawn Blinky (red, type 0) at col 13, row 11 (outside ghost house)
    spawnGhost(0, 378.0f, 315.0f, 0.0f, -GHOST_SPEED, 255, 0, 0);
    // Spawn Pinky (pink, type 1) at col 13, row 14 (inside ghost house center)
    spawnGhost(1, 378.0f, 387.0f, 0.0f, GHOST_SPEED, 255, 120, 200);
    // Spawn Inky (cyan, type 2) at col 12, row 14 (inside ghost house left)
    spawnGhost(2, 354.0f, 387.0f, -GHOST_SPEED, 0.0f, 0, 180, 255);
    // Spawn Clyde (orange, type 3) at col 15, row 14 (inside ghost house right)
    spawnGhost(3, 426.0f, 387.0f, GHOST_SPEED, 0.0f, 255, 150, 0);
}

void Game::updateSystems(float deltaTime) {
    GameStateSystem::update(pacman, deltaTime);
    GhostAISystem::update(pacman, deltaTime);
    VisibilitySystem::update(pacman, gameStateId, deltaTime);
}

void Game::applyGhostGridMovement() {
    static const bagel::Mask ghostMask = bagel::MaskBuilder()
        .set<GhostAI>()
        .set<PositionComponent>()
        .set<MovementComponent>()
        .build();
    static int q = bagel::World::createQuery(ghostMask);

    for (bagel::Entity e = bagel::World::first(q); !bagel::World::eof(q); e = bagel::World::next(q)) {
        auto& position = e.get<PositionComponent>();
        auto& movement = e.get<MovementComponent>();

        enforceCardinalMovement(movement);
        snapEntityToGridLane(position, movement);
    }
}

void Game::run() {
    Uint64 previousTime = SDL_GetTicks();

    while (running) {
        Uint64 currentTime = SDL_GetTicks();
        float deltaTime = static_cast<float>(currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;

        inputSystem.handleInput(running, visionMode);
        renderSystem.drawStatus(window, visionMode);
        update(deltaTime);
        renderSystem.render(renderer, visionMode);
    }
}

void Game::update(float deltaTime) {
    updateSystems(deltaTime);
    batterySystem.update(deltaTime);
    
    // Check if Game Over (battery is 0%)
    static const bagel::Mask stateMask = bagel::MaskBuilder()
        .set<GameStateComponent>()
        .build();
    static int stateQ = bagel::World::createQuery(stateMask);
    if (!bagel::World::eof(stateQ)) {
        bagel::Entity stateEnt = bagel::World::first(stateQ);
        if (stateEnt.get<GameStateComponent>().isGameOver) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Game Over", "Your battery died! Game Over.", window);
            running = false;
            return;
        }
    }

    movementSystem.update(deltaTime);
    applyGhostGridMovement();
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
