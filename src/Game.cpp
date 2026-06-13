#include "Game.h"
#include "Constants.h"
#include "GridMovement.h"
#include <SDL3_image/SDL_image.h>

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

    gameOverTexture = IMG_LoadTexture(renderer, "images/GameOver.png");
    if (gameOverTexture == nullptr) {
        SDL_Log("Failed to load GameOver.png: %s", SDL_GetError());
    }
    victoryTexture = IMG_LoadTexture(renderer, "images/Victory.png");
    if (victoryTexture == nullptr) {
        SDL_Log("Failed to load Victory.png: %s", SDL_GetError());
    }
    initialMazeLayout = MAZE_LAYOUT;

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
    stateEnt.add(GameStateComponent{ 100.0f, 0, false, false, 0, false, false });
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
    wasBatteryAbove50 = true;
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

        bool resetRequested = false;
        inputSystem.handleInput(running, visionMode, resetRequested);
        renderSystem.drawStatus(window, visionMode);

        if (resetRequested) {
            reset();
            previousTime = SDL_GetTicks();
            continue;
        }

        bool isGameOver = false;
        static const bagel::Mask stateMask = bagel::MaskBuilder()
            .set<GameStateComponent>()
            .build();
        static int stateQ = bagel::World::createQuery(stateMask);
        if (!bagel::World::eof(stateQ)) {
            isGameOver = bagel::World::getComponent<GameStateComponent>(bagel::World::first(stateQ)).isGameOver;
        }

        if (!isGameOver) {
            update(deltaTime);
        }
        renderSystem.render(renderer, visionMode, gameOverTexture, victoryTexture);
    }
}

void Game::update(float deltaTime) {
    updateSystems(deltaTime);
    batterySystem.update(deltaTime);

    if (bagel::World::mask(pacman).test(bagel::Component<BatteryLifeComponent>::Bit)) {
        const auto& battery = bagel::World::getComponent<BatteryLifeComponent>(pacman);
        float batteryPct = (battery.max > 0.0f) ? ((battery.current / battery.max) * 100.0f) : 0.0f;
        if (batteryPct > 50.0f) {
            if (!wasBatteryAbove50) {
                wasBatteryAbove50 = true;
                visionMode = VisionMode::Full;
            }
        } else {
            if (wasBatteryAbove50) {
                wasBatteryAbove50 = false;
                visionMode = VisionMode::MediumDark;
            }
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

void Game::reset() {
    MAZE_LAYOUT = initialMazeLayout;

    if (bagel::World::mask(pacman).test(bagel::Component<BatteryLifeComponent>::Bit)) {
        auto& battery = bagel::World::getComponent<BatteryLifeComponent>(pacman);
        battery.current = 100.0f;
    }
    if (bagel::World::mask(pacman).test(bagel::Component<FlashlightComponent>::Bit)) {
        auto& flashlight = bagel::World::getComponent<FlashlightComponent>(pacman);
        flashlight.isOn = false;
    }
    if (bagel::World::mask(pacman).test(bagel::Component<PositionComponent>::Bit)) {
        auto& position = bagel::World::getComponent<PositionComponent>(pacman);
        position.x = 378.0f;
        position.y = 723.0f;
    }
    if (bagel::World::mask(pacman).test(bagel::Component<MovementComponent>::Bit)) {
        auto& movement = bagel::World::getComponent<MovementComponent>(pacman);
        movement.vx = 0.0f;
        movement.vy = 0.0f;
    }
    if (bagel::World::mask(pacman).test(bagel::Component<DirectionComponent>::Bit)) {
        auto& direction = bagel::World::getComponent<DirectionComponent>(pacman);
        direction.current = Direction::Right;
        direction.queued = Direction::None;
    }
    if (bagel::World::mask(pacman).test(bagel::Component<PhysicsComponent>::Bit)) {
        auto& physics = bagel::World::getComponent<PhysicsComponent>(pacman);
        b2Body_SetTransform(physics.bodyId, b2Vec2{ 378.0f + 16.0f, 723.0f + 16.0f }, b2Rot_identity);
        b2Body_SetLinearVelocity(physics.bodyId, b2Vec2{ 0.0f, 0.0f });
    }

    wasBatteryAbove50 = true;
    visionMode = VisionMode::Full;

    static const bagel::Mask stateMask = bagel::MaskBuilder()
        .set<GameStateComponent>()
        .build();
    static int stateQ = bagel::World::createQuery(stateMask);
    if (!bagel::World::eof(stateQ)) {
        bagel::Entity stateEnt = bagel::World::first(stateQ);
        auto& state = stateEnt.get<GameStateComponent>();
        state.batteryLevel = 100.0f;
        state.score = 0;
        state.isGameOver = false;
        state.isLowBattery = false;
        state.nearbyGhosts = 0;
        state.shownGameOverPopup = false;
        state.shownVictoryPopup = false;
    }

    static const bagel::Mask ghostMask = bagel::MaskBuilder()
        .set<GhostAI>()
        .set<PositionComponent>()
        .set<MovementComponent>()
        .set<PhysicsComponent>()
        .build();
    static int ghostQ = bagel::World::createQuery(ghostMask);
    for (bagel::Entity ghost = bagel::World::first(ghostQ); !bagel::World::eof(ghostQ); ghost = bagel::World::next(ghostQ)) {
        auto& ai = ghost.get<GhostAI>();
        auto& pos = ghost.get<PositionComponent>();
        auto& move = ghost.get<MovementComponent>();
        auto& physics = ghost.get<PhysicsComponent>();

        float initialX = 378.0f;
        float initialY = 387.0f;
        float initialVx = 0.0f;
        float initialVy = 0.0f;

        if (ai.ghostType == 0) {
            initialX = 378.0f; initialY = 315.0f;
            initialVx = 0.0f; initialVy = -GHOST_SPEED;
        } else if (ai.ghostType == 1) {
            initialX = 378.0f; initialY = 387.0f;
            initialVx = 0.0f; initialVy = GHOST_SPEED;
        } else if (ai.ghostType == 2) {
            initialX = 354.0f; initialY = 387.0f;
            initialVx = -GHOST_SPEED; initialVy = 0.0f;
        } else if (ai.ghostType == 3) {
            initialX = 426.0f; initialY = 387.0f;
            initialVx = GHOST_SPEED; initialVy = 0.0f;
        }

        pos.x = initialX;
        pos.y = initialY;
        move.vx = initialVx;
        move.vy = initialVy;

        ai.state = GhostState::SCATTER;
        ai.target = scatterCorner(ai.ghostType);
        ai.lastTurnTileCol = -1;
        ai.lastTurnTileRow = -1;

        if (ghost.has<VisibilityComponent>()) {
            auto& vis = ghost.get<VisibilityComponent>();
            vis.isVisible = true;
            vis.opacity = GHOST_MIN_OPACITY;
        }

        b2Body_SetTransform(physics.bodyId, b2Vec2{ initialX + 16.0f, initialY + 16.0f }, b2Rot_identity);
        b2Body_SetLinearVelocity(physics.bodyId, b2Vec2{ initialVx, initialVy });
    }
}

void Game::clean() {
    if (gameOverTexture != nullptr) {
        SDL_DestroyTexture(gameOverTexture);
        gameOverTexture = nullptr;
    }
    if (victoryTexture != nullptr) {
        SDL_DestroyTexture(victoryTexture);
        victoryTexture = nullptr;
    }

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
