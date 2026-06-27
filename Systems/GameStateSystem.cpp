#include "GameStateSystem.h"
#include "Constants.h"
#include "Maze.h"
#include <box2d/box2d.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include "SoundManager.h"

namespace {
    float entityCenterX(float x) {
        return x + TILE_SIZE * 0.5f;
    }

    float entityCenterY(float y) {
        return y + TILE_SIZE * 0.5f;
    }
}

GameStateComponent* GameStateSystem::getState() {
    static const bagel::Mask mask = bagel::MaskBuilder()
        .set<GameStateComponent>()
        .build();
    static int q = bagel::World::createQuery(mask);

    if (bagel::World::eof(q)) {
        return nullptr;
    }

    bagel::Entity e = bagel::World::first(q);
    return &e.get<GameStateComponent>();
}

void GameStateSystem::update(bagel::ent_type pacmanId, float deltaTime) {
    static const bagel::Mask stateMask = bagel::MaskBuilder()
        .set<GameStateComponent>()
        .build();
    static const bagel::Mask ghostMask = bagel::MaskBuilder()
        .set<GhostAI>()
        .set<PositionComponent>()
        .build();

    static int stateQ = bagel::World::createQuery(stateMask);
    static int ghostQ = bagel::World::createQuery(ghostMask);

    const auto& pacPos = bagel::World::getComponent<PositionComponent>(pacmanId);
    const float pacCenterX = entityCenterX(pacPos.x);
    const float pacCenterY = entityCenterY(pacPos.y);

    // 1. Calculate Pacman's current tile coordinates
    const float pacCenterX_logical = pacPos.x + 16.0f;
    const float pacCenterY_logical = pacPos.y + 16.0f;
    int pacCol = static_cast<int>(std::floor((pacCenterX_logical - MAZE_START_X) / MAZE_TILE_SIZE));
    int pacRow = static_cast<int>(std::floor((pacCenterY_logical - MAZE_START_Y) / MAZE_TILE_SIZE));

    static float frightenedTimer = 0.0f;
    static float invincibilityTimer = 0.0f;

    if (frightenedTimer > 0.0f) {
        frightenedTimer -= deltaTime;
    }
    if (invincibilityTimer > 0.0f) {
        invincibilityTimer -= deltaTime;
        auto& battery = bagel::World::getComponent<BatteryLifeComponent>(pacmanId);

        //Reduce battery gradually
        battery.current = battery.current - DAMAGE/(2.0/deltaTime);
    }

    GameStateComponent* statePtr = nullptr;
    if (!bagel::World::eof(stateQ)) {
        bagel::Entity stateEnt = bagel::World::first(stateQ);
        statePtr = &stateEnt.get<GameStateComponent>();
    }
    else {
        stateQ = bagel::World::createQuery(stateMask);
        bagel::Entity stateEnt = bagel::World::first(stateQ);
        statePtr = &stateEnt.get<GameStateComponent>();
    }


    if (statePtr != nullptr && !statePtr->isGameOver) {
        // 2. Dot & Energizer eating
        if (pacRow >= 0 && pacRow < MAZE_ROWS && pacCol >= 0 && pacCol < MAZE_COLS) {

            char& cell = MAZE_LAYOUT[pacRow][pacCol];
            if (cell == '.') {
                auto& battery = bagel::World::getComponent<BatteryLifeComponent>(pacmanId);
                battery.current = std::min(100.0f, battery.current + 0.3f);
                cell = ' ';
                statePtr->score += 10;
                SoundManager::getInstance().playSFX("eating");
            } else if (cell == 'o') {
                auto& battery = bagel::World::getComponent<BatteryLifeComponent>(pacmanId);
                battery.current = std::min(100.0f, battery.current + 0.3f);
                cell = ' ';
                statePtr->score += 50;
                frightenedTimer = 7.0f; // Frightened mode lasts 7 seconds
                RenderSystem::ghostState = GhostState::FRIGHTENED;
            } else if (cell == 'F') {
                auto& battery = bagel::World::getComponent<BatteryLifeComponent>(pacmanId);
                battery.current = std::min(100.0f, battery.current + 20.0f);
                cell = ' ';
                SoundManager::getInstance().playSFX("light_pellet");

                // Find and destroy the corresponding charger entity
                static const bagel::Mask chargerMask = bagel::MaskBuilder()
                    .set<PositionComponent>()
                    .set<ChargerComponent>()
                    .build();
                static int chargerQ = bagel::World::createQuery(chargerMask);
                
                float targetX = MAZE_START_X + pacCol * MAZE_TILE_SIZE;
                float targetY = MAZE_START_Y + pacRow * MAZE_TILE_SIZE;
                
                for (bagel::Entity c = bagel::World::first(chargerQ); !bagel::World::eof(chargerQ); c = bagel::World::next(chargerQ)) {
                    auto& cpos = c.get<PositionComponent>();
                    if (std::abs(cpos.x - targetX) < 1.0f && std::abs(cpos.y - targetY) < 1.0f) {
                        c.destroy();
                        break;
                    }
                }
            }
        }

        // 3. Set ghost state to Frightened if timer is active
        for (bagel::Entity ghost = bagel::World::first(ghostQ); !bagel::World::eof(ghostQ); ghost = bagel::World::next(ghostQ)) {

            auto& ai = ghost.get<GhostAI>();
            if (frightenedTimer > 0.0f) {
                if (ai.state != GhostState::EATEN) {
                    ai.state = GhostState::FRIGHTENED;
                }
            } else {
                if (ai.state == GhostState::FRIGHTENED) {
                    ai.state = GhostState::SCATTER; // Fallback to normal behavior
                    RenderSystem::ghostState = GhostState::SCATTER;
                }
            }
        }

        // Frightened siren logic
        static float vulnerableSirenTimer = 0.0f;
        if (frightenedTimer > 0.0f) {
            vulnerableSirenTimer -= deltaTime;
            if (vulnerableSirenTimer <= 0.0f) {
                SoundManager::getInstance().playSFX("ghost_vulnerable");
                vulnerableSirenTimer = 0.5f;
            }
        } else {
            vulnerableSirenTimer = 0.0f;
        }

        // 4. Ghost-Pacman collisions
        for (bagel::Entity ghost = bagel::World::first(ghostQ); !bagel::World::eof(ghostQ); ghost = bagel::World::next(ghostQ)) {
            auto& ghostPos = ghost.get<PositionComponent>();
            auto& ai = ghost.get<GhostAI>();
            const float ghostCenterX_logical = ghostPos.x + 16.0f;
            const float ghostCenterY_logical = ghostPos.y + 16.0f;

            const float dx = ghostCenterX_logical - pacCenterX_logical;
            const float dy = ghostCenterY_logical - pacCenterY_logical;
            const float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < 24.0f) { //Collision
                if (ai.state == GhostState::FRIGHTENED) {
                    // Pacman eats the ghost
                    ai.state = GhostState::EATEN;
                    statePtr->score += 200;

                    // Send the ghost back to the ghost house
                    ghostPos.x = GHOST_HOUSE_X - 16.0f;
                    ghostPos.y = GHOST_HOUSE_Y - 16.0f;

                    auto& ghostMove = ghost.get<MovementComponent>();
                    ghostMove.vx = 0.0f;
                    ghostMove.vy = 0.0f;

                    if (ghost.has<PhysicsComponent>()) {
                        b2BodyId bodyId = ghost.get<PhysicsComponent>().bodyId;
                        b2Body_SetTransform(bodyId, b2Vec2{ GHOST_HOUSE_X, GHOST_HOUSE_Y }, b2Rot_identity);
                        b2Body_SetLinearVelocity(bodyId, b2Vec2{ 0.0f, 0.0f });
                    }
                }
                else if (ai.state == GhostState::CHASE || ai.state == GhostState::SCATTER) {
                    // Normal ghost damage to Pacman
                    if (invincibilityTimer <= 0.0f) {
                        if (bagel::World::mask(pacmanId).test(bagel::Component<BatteryLifeComponent>::Bit)) {
                            auto& battery = bagel::World::getComponent<BatteryLifeComponent>(pacmanId);

                            if (battery.current <= 35.0f) //if pacman dies - reduce battery immediately
                                battery.current = battery.current-35.0f;
                            //otherwise - reduce battery gradually - upper code section


                            // Send the ghost back to the ghost house
                            ghostPos.x = GHOST_HOUSE_X - 16.0f;
                            ghostPos.y = GHOST_HOUSE_Y - 16.0f;

                            auto& ghostMove = ghost.get<MovementComponent>();
                            ghostMove.vx = 0.0f;
                            ghostMove.vy = 0.0f;

                            if (ghost.has<PhysicsComponent>()) {
                                b2BodyId bodyId = ghost.get<PhysicsComponent>().bodyId;
                                b2Body_SetTransform(bodyId, b2Vec2{ GHOST_HOUSE_X, GHOST_HOUSE_Y }, b2Rot_identity);
                                b2Body_SetLinearVelocity(bodyId, b2Vec2{ 0.0f, 0.0f });
                            }
                            if (battery.current < 0.0f) {
                                battery.current = 0.0f;
                            }
                        }
                        invincibilityTimer = 2.0f; // 2 seconds invincibility frame
                    }
                }
            }
        }

        // 5. Victory check (all dots eaten)
        int remainingDots = 0;
        for (int r = 0; r < MAZE_ROWS; ++r) {
            for (int c = 0; c < MAZE_COLS; ++c) {
                if (MAZE_LAYOUT[r][c] == '.' || MAZE_LAYOUT[r][c] == 'o') {
                    remainingDots++;
                }
            }
        }

        if (remainingDots == 0) {
            bool wasGameOver = statePtr->isGameOver;
            statePtr->shownVictoryPopup = true;
            statePtr->isGameOver = true;
            if (!wasGameOver) {
                SoundManager::getInstance().stopBGM();
                SoundManager::getInstance().playSFX("victory");
            }
        }
    }

    int nearbyGhosts = 0;
    for (bagel::Entity ghost = bagel::World::first(ghostQ); !bagel::World::eof(ghostQ); ghost = bagel::World::next(ghostQ)) {
        const auto& ghostPos = ghost.get<PositionComponent>();
        const float ghostCenterX = entityCenterX(ghostPos.x);
        const float ghostCenterY = entityCenterY(ghostPos.y);
        const float dx = ghostCenterX - pacCenterX;
        const float dy = ghostCenterY - pacCenterY;
        const float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= GHOST_NEAR_DISTANCE) {
            ++nearbyGhosts;
        }
    }

    for (bagel::Entity e = bagel::World::first(stateQ); !bagel::World::eof(stateQ); e = bagel::World::next(stateQ)) {
        auto& state = e.get<GameStateComponent>();
        state.nearbyGhosts = nearbyGhosts;

        if (bagel::World::mask(pacmanId).test(bagel::Component<BatteryLifeComponent>::Bit)) {
            auto& battery = bagel::World::getComponent<BatteryLifeComponent>(pacmanId);

            if (nearbyGhosts > 0) {
                battery.current -= nearbyGhosts * BATTERY_DRAIN_PER_NEAR_GHOST * deltaTime;
                if (battery.current < 0.0f) {
                    battery.current = 0.0f;
                }
            }

            state.batteryLevel = battery.current;
            state.isLowBattery = battery.current <= LOW_BATTERY_THRESHOLD;
            
            bool wasGameOver = state.isGameOver;
            state.isGameOver = state.isGameOver || (battery.current <= 0.0f);
            
            if (state.isGameOver && !wasGameOver) {
                SoundManager::getInstance().stopBGM();
                SoundManager::getInstance().playSFX("death");
            }

            // Heartbeat Logic
            float batteryPct = (battery.max > 0.0f) ? ((battery.current / battery.max) * 100.0f) : 0.0f;
            static float heartbeatTimer = 0.0f;
            if (batteryPct < 30.0f && !state.isGameOver) {
                heartbeatTimer -= deltaTime;
                if (heartbeatTimer <= 0.0f) {
                    SoundManager::getInstance().playSFX("heartbeat");
                    float interval = 0.4f + 0.8f * (batteryPct / 30.0f);
                    heartbeatTimer = interval;
                }
            } else {
                heartbeatTimer = 0.0f;
            }

            continue;
        }

        if (state.isGameOver) {
            state.isLowBattery = true;
            continue;
        }

        state.batteryLevel -= (BATTERY_DRAIN_RATE + nearbyGhosts * BATTERY_DRAIN_PER_NEAR_GHOST) * deltaTime;

        if (state.batteryLevel < 0.0f) {
            state.batteryLevel = 0.0f;
            state.isGameOver = true;
            SoundManager::getInstance().stopBGM();
            SoundManager::getInstance().playSFX("death");
        }

        if (state.batteryLevel > 100.0f) {
            state.batteryLevel = 100.0f;
        }

        state.isLowBattery = state.batteryLevel <= LOW_BATTERY_THRESHOLD;
    }
}
