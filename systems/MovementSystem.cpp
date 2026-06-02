#include "MovementSystem.h"

void MovementSystem::update(
    float deltaTime,
    std::unordered_map<Entity, PositionComponent>& positions,
    std::unordered_map<Entity, MovementComponent>& movements,
    std::unordered_map<Entity, CollisionComponent>& collisions,
    int windowWidth,
    int windowHeight
) {
    const float mazeLeft = 70.0f;
    const float mazeTop = 55.0f;
    const float mazeRight = 742.0f;
    const float mazeBottom = 775.0f;

    for (auto& [entity, movement] : movements) {
        if (!positions.contains(entity)) {
            continue;
        }

        auto& position = positions[entity];

        position.x += movement.vx * deltaTime;
        position.y += movement.vy * deltaTime;

        int width = 32;
        int height = 32;

        if (collisions.contains(entity)) {
            width = collisions[entity].width;
            height = collisions[entity].height;
        }

        if (position.x < mazeLeft) {
            position.x = mazeLeft;
        }

        if (position.y < mazeTop) {
            position.y = mazeTop;
        }

        if (position.x + width > mazeRight) {
            position.x = mazeRight - width;
        }

        if (position.y + height > mazeBottom) {
            position.y = mazeBottom - height;
        }
    }
}