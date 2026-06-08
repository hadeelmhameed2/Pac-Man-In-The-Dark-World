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
        b2WorldId worldId,
        int ghostType,
        float x, float y,
        float vx, float vy,
        Uint8 r, Uint8 g, Uint8 b
    ) {
        // Create dynamic Box2D body for the ghost
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = b2Vec2{ x + 16.0f, y + 16.0f };
        bodyDef.motionLocks.angularZ = true;
        b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.filter.categoryBits = CATEGORY_GHOST;
        shapeDef.filter.maskBits = CATEGORY_WALL;
        shapeDef.material.friction = 0.0f;
        shapeDef.material.restitution = 0.0f;

        b2Circle circle;
        circle.center = b2Vec2{ 0.0f, 0.0f };
        circle.radius = 11.0f;
        b2CreateCircleShape(bodyId, &shapeDef, &circle);

        b2Body_SetLinearVelocity(bodyId, b2Vec2{ vx, vy });

        bagel::Entity ghost = bagel::Entity::create();
        ghost.add(PositionComponent{ x, y });
        ghost.add(MovementComponent{ vx, vy, GHOST_SPEED });
        ghost.add(DrawingComponent{ TILE_SIZE, TILE_SIZE, r, g, b, 255 });
        ghost.add(GhostAI{ GhostState::SCATTER, scatterCorner(ghostType), ghostType });
        ghost.add(VisibilityComponent{ true, GHOST_MIN_OPACITY });
        ghost.add(PhysicsComponent{ bodyId });
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

    // Initialize Box2D world
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{ 0.0f, 0.0f };
    worldId = b2CreateWorld(&worldDef);

    // Create static body for the maze walls
    b2BodyDef wallBodyDef = b2DefaultBodyDef();
    wallBodyDef.type = b2_staticBody;
    wallBodyDef.position = b2Vec2{ 0.0f, 0.0f };
    b2BodyId mazeBodyId = b2CreateBody(worldId, &wallBodyDef);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter.categoryBits = CATEGORY_WALL;
    shapeDef.filter.maskBits = CATEGORY_PACMAN | CATEGORY_GHOST;
    shapeDef.material.friction = 0.0f;
    shapeDef.material.restitution = 0.0f;

    for (int row = 0; row < MAZE_ROWS; ++row) {
        for (int col = 0; col < MAZE_COLS; ++col) {
            if (MAZE_LAYOUT[row][col] == '#') {
                float centerX = MAZE_START_X + col * MAZE_TILE_SIZE + MAZE_TILE_SIZE / 2.0f;
                float centerY = MAZE_START_Y + row * MAZE_TILE_SIZE + MAZE_TILE_SIZE / 2.0f;

                b2Polygon box = b2MakeOffsetBox(12.0f, 12.0f, b2Vec2{ centerX, centerY }, b2Rot_identity);
                b2CreatePolygonShape(mazeBodyId, &shapeDef, &box);
            }
        }
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

    // Create dynamic Box2D body for Pacman
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2{ 378.0f + 16.0f, 723.0f + 16.0f };
    bodyDef.motionLocks.angularZ = true;
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter.categoryBits = CATEGORY_PACMAN;
    shapeDef.filter.maskBits = CATEGORY_WALL;
    shapeDef.material.friction = 0.0f;
    shapeDef.material.restitution = 0.0f;

    b2Circle circle;
    circle.center = b2Vec2{ 0.0f, 0.0f };
    circle.radius = 11.0f;
    b2CreateCircleShape(bodyId, &shapeDef, &circle);

    entity.addAll(
        BatteryLifeComponent{ 100.0f, 100.0f, 0.3f, 1.2f },
        FlashlightComponent{ false, true },
        DirectionComponent{ Direction::Right },
        InputComponent{ true },
        PositionComponent{ 378.0f, 723.0f },
        MovementComponent{ 0.0f, 0.0f, 160.0f },
        DrawingComponent{ 32, 32, 255, 255, 0, 255 },
        CollisionComponent{ 32, 32, false },
        PhysicsComponent{ bodyId }
    );
}

void Game::createGhosts() {
    // Spawn Blinky (red, type 0) at col 13, row 11 (outside ghost house)
    spawnGhost(worldId, 0, 378.0f, 315.0f, 0.0f, -GHOST_SPEED, 255, 0, 0);
    // Spawn Pinky (pink, type 1) at col 13, row 14 (inside ghost house center)
    spawnGhost(worldId, 1, 378.0f, 387.0f, 0.0f, GHOST_SPEED, 255, 120, 200);
    // Spawn Inky (cyan, type 2) at col 12, row 14 (inside ghost house left)
    spawnGhost(worldId, 2, 354.0f, 387.0f, -GHOST_SPEED, 0.0f, 0, 180, 255);
    // Spawn Clyde (orange, type 3) at col 15, row 14 (inside ghost house right)
    spawnGhost(worldId, 3, 426.0f, 387.0f, GHOST_SPEED, 0.0f, 255, 150, 0);
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
        .set<PhysicsComponent>()
        .build();
    static int q = bagel::World::createQuery(ghostMask);

    for (bagel::Entity e = bagel::World::first(q); !bagel::World::eof(q); e = bagel::World::next(q)) {
        auto& movement = e.get<MovementComponent>();
        auto& physics = e.get<PhysicsComponent>();

        enforceCardinalMovement(movement);
        
        // Apply velocity to Box2D body
        b2Body_SetLinearVelocity(physics.bodyId, b2Vec2{ movement.vx, movement.vy });

        // Snap body center to grid lane
        b2Vec2 pos = b2Body_GetPosition(physics.bodyId);
        if (std::abs(movement.vx) > 0.1f) {
            float row = std::round((pos.y - MAZE_START_Y - MAZE_TILE_SIZE * 0.5f) / MAZE_TILE_SIZE);
            float targetY = MAZE_START_Y + row * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f;
            b2Body_SetTransform(physics.bodyId, b2Vec2{ pos.x, targetY }, b2Rot_identity);
        } else if (std::abs(movement.vy) > 0.1f) {
            float col = std::round((pos.x - MAZE_START_X - MAZE_TILE_SIZE * 0.5f) / MAZE_TILE_SIZE);
            float targetX = MAZE_START_X + col * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f;
            b2Body_SetTransform(physics.bodyId, b2Vec2{ targetX, pos.y }, b2Rot_identity);
        }
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

    // Step physics
    b2World_Step(worldId, deltaTime, 4);

    // Sync positions from Box2D body to PositionComponent
    static const bagel::Mask syncMask = bagel::MaskBuilder()
        .set<PositionComponent>()
        .set<PhysicsComponent>()
        .build();
    static int syncQ = bagel::World::createQuery(syncMask);
    for (bagel::Entity e = bagel::World::first(syncQ); !bagel::World::eof(syncQ); e = bagel::World::next(syncQ)) {
        b2BodyId bId = e.get<PhysicsComponent>().bodyId;
        b2Vec2 pos = b2Body_GetPosition(bId);
        auto& position = e.get<PositionComponent>();
        position.x = pos.x - 16.0f;
        position.y = pos.y - 16.0f;
    }
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

    b2DestroyWorld(worldId);

    SDL_Quit();
}
