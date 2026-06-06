#include "MovementSystem.h"

void MovementSystem::update(
    float deltaTime,
    int windowWidth,
    int windowHeight
) {
    const float mazeLeft = 70.0f;
    const float mazeTop = 55.0f;
    const float mazeRight = 742.0f;
    const float mazeBottom = 775.0f;

    static const bagel::Mask moveMask =
     bagel::MaskBuilder()
         .set<PositionComponent>()
         .set<MovementComponent>()
         .build();

    for (bagel::Entity e = bagel::Entity::first();
         !e.eof();
         e.next())
    {
        if (!e.test(moveMask))
            continue;

        auto& position = e.get<PositionComponent>();
        auto& movement = e.get<MovementComponent>();

        position.x += movement.vx * deltaTime;
        position.y += movement.vy * deltaTime;

        int width = 32;
        int height = 32;

        if (e.has<CollisionComponent>())
        {
            auto& collision = e.get<CollisionComponent>();
            width = collision.width;
            height = collision.height;
        }

        if (position.x < mazeLeft)
        {
            position.x = mazeLeft;
        }

        if (position.y < mazeTop)
        {
            position.y = mazeTop;
        }

        if (position.x + width > mazeRight)
        {
            position.x = mazeRight - width;
        }

        if (position.y + height > mazeBottom)
        {
            position.y = mazeBottom - height;
        }
    }
}