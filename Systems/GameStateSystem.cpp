#include "GameStateSystem.h"
#include "Constants.h"
#include <cmath>

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

        if (state.isGameOver) {
            state.isLowBattery = true;
            state.nearbyGhosts = nearbyGhosts;
            continue;
        }

        state.nearbyGhosts = nearbyGhosts;
        state.batteryLevel -= (BATTERY_DRAIN_RATE + nearbyGhosts * BATTERY_DRAIN_PER_NEAR_GHOST) * deltaTime;

        if (state.batteryLevel < 0.0f) {
            state.batteryLevel = 0.0f;
            state.isGameOver = true;
        }

        if (state.batteryLevel > 100.0f) {
            state.batteryLevel = 100.0f;
        }

        state.isLowBattery = state.batteryLevel <= LOW_BATTERY_THRESHOLD;
    }
}
