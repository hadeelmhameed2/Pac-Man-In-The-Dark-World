#include "VisibilitySystem.h"
#include <cmath>

void VisibilitySystem::update(bagel::ent_type pacmanId, bagel::ent_type gameStateId) {
    static const bagel::Mask mask = bagel::MaskBuilder()
        .set<PositionComponent>()
        .set<VisibilityComponent>()
        .build();

    static int q = bagel::World::createQuery(mask);

    const auto& pacPos = bagel::World::getComponent<PositionComponent>(pacmanId);
    const auto& gameState = bagel::World::getComponent<GameStateComponent>(gameStateId);
    const auto& pacMove = bagel::World::getComponent<MovementComponent>(pacmanId);

    float lightRadius = 150.0f * (gameState.batteryLevel / 100.0f);
    float fadeMargin = 40.0f;

    for (bagel::Entity e = bagel::World::first(q); !bagel::World::eof(q); e = bagel::World::next(q)) {
        auto& vis = e.get<VisibilityComponent>();
        const auto& entPos = e.get<PositionComponent>();

        float dx = entPos.x - pacPos.x;
        float dy = entPos.y - pacPos.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        float glowOpacity = 0.0f;
        if (dist < lightRadius) {
            glowOpacity = 1.0f;
        } else if (dist < lightRadius + fadeMargin) {
            glowOpacity = 1.0f - ((dist - lightRadius) / fadeMargin);
        }

        float flashOpacity = 0.0f;
        if (pacMove.vx != 0 || pacMove.vy != 0) {
            float mag = std::sqrt(pacMove.vx * pacMove.vx + pacMove.vy * pacMove.vy);
            float dot = (dx / dist) * (pacMove.vx / mag) + (dy / dist) * (pacMove.vy / mag);

            if (dot > 0.70f && dist < lightRadius * 2.5f) {
                flashOpacity = (dot - 0.70f) / (1.0f - 0.70f);
                flashOpacity *= (1.0f - (dist / (lightRadius * 2.5f)));
            }
        }

        vis.opacity = std::max(glowOpacity, flashOpacity);
        vis.isVisible = (vis.opacity > 0.01f);
    }
}