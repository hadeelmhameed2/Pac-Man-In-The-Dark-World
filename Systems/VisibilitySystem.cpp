#include "VisibilitySystem.h"
#include "Constants.h"
#include <cmath>
#include <algorithm>

void VisibilitySystem::update(bagel::ent_type pacmanId, bagel::ent_type gameStateId, float deltaTime) {
    static const bagel::Mask mask = bagel::MaskBuilder()
        .set<PositionComponent>()
        .set<VisibilityComponent>()
        .build();

    static int q = bagel::World::createQuery(mask);
    static float pulseTimer = 0.0f;
    pulseTimer += deltaTime;

    const auto& pacPos = bagel::World::getComponent<PositionComponent>(pacmanId);
    const auto& gameState = bagel::World::getComponent<GameStateComponent>(gameStateId);

    const float pacCenterX = pacPos.x + TILE_SIZE * 0.5f;
    const float pacCenterY = pacPos.y + TILE_SIZE * 0.5f;

    float lightRadius = LIGHT_RADIUS * (gameState.batteryLevel / 100.0f);
    if (gameState.isGameOver) {
        lightRadius = LIGHT_RADIUS * 0.25f;
    } else if (gameState.isLowBattery) {
        const float flicker = 0.85f + 0.15f * std::sin(pulseTimer * 8.0f);
        lightRadius *= flicker;
    }

    const float fadeMargin = LIGHT_FADE_MARGIN;
    const float smoothSpeed = 6.0f * deltaTime;

    for (bagel::Entity e = bagel::World::first(q); !bagel::World::eof(q); e = bagel::World::next(q)) {
        auto& vis = e.get<VisibilityComponent>();
        const auto& entPos = e.get<PositionComponent>();

        const float entCenterX = entPos.x + TILE_SIZE * 0.5f;
        const float entCenterY = entPos.y + TILE_SIZE * 0.5f;
        const float dx = entCenterX - pacCenterX;
        const float dy = entCenterY - pacCenterY;
        const float dist = std::sqrt(dx * dx + dy * dy);

        float litAmount = 0.0f;
        if (dist < lightRadius) {
            litAmount = 1.0f;
        } else if (dist < lightRadius + fadeMargin) {
            litAmount = 1.0f - ((dist - lightRadius) / fadeMargin);
        }

        if (dist <= GHOST_NEAR_DISTANCE) {
            const float dangerPulse = 0.15f * std::sin(pulseTimer * 12.0f);
            litAmount = std::min(1.0f, litAmount + 0.25f + dangerPulse);
        }

        const float targetOpacity = GHOST_MIN_OPACITY + litAmount * (1.0f - GHOST_MIN_OPACITY);
        vis.opacity += (targetOpacity - vis.opacity) * std::min(1.0f, smoothSpeed);
        vis.isVisible = true;
    }
}
