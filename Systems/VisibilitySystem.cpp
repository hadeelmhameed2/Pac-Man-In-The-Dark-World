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
    for (bagel::Entity e = bagel::World::first(q); !bagel::World::eof(q); e = bagel::World::next(q)) {
        if (e.entity().id == pacmanId.id) {
            e.get<VisibilityComponent>().isVisible = true;
            continue;
        }

        auto& vis = e.get<VisibilityComponent>();
        const auto& entPos = e.get<PositionComponent>();

        float dx = entPos.x - pacPos.x;
        float dy = entPos.y - pacPos.y;
        float distSq = dx * dx + dy * dy;

        bool inGlow = (distSq < (lightRadius * lightRadius));
        bool inFlashlight = false;
        if (pacMove.vx != 0 || pacMove.vy != 0) {
            float dist = std::sqrt(distSq);
            float mag = std::sqrt(pacMove.vx * pacMove.vx + pacMove.vy * pacMove.vy);
            float dot = (dx / dist) * (pacMove.vx / mag) + (dy / dist) * (pacMove.vy / mag);

            if (dot > 0.85f && dist < lightRadius * 2.0f) {
                inFlashlight = true;
            }
        }

        vis.isVisible = inGlow || inFlashlight;
    }
}