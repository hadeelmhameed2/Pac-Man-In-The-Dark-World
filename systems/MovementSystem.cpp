#include "MovementSystem.h"

void MovementSystem::update(
    float deltaTime
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
        auto& collision = e.get<CollisionComponent>();

        int width = collision.width;
        int height = collision.height;

        float oldX = position.x;
        float oldY = position.y;

        position.x += movement.vx * deltaTime;
        position.y += movement.vy * deltaTime;

        position.x += movement.vx * deltaTime;
        position.y += movement.vy * deltaTime;

        static const bagel::Mask wallMask =
            bagel::MaskBuilder()
        .set<WallComponent>()
        .set<PositionComponent>()
        .set<CollisionComponent>()
        .build();

        for (bagel::Entity e1 = bagel::Entity::first();!e1.eof(); e1.next()) {
            if (!e1.test(wallMask))
                continue;

            auto& positionwall = e1.get<PositionComponent>();
            auto& collisionwall = e1.get<CollisionComponent>();

            int widthwall = collisionwall.width;
            int heightwall = collisionwall.height;

            bool overlapx =
            position.x < positionwall.x + widthwall &&
            position.x + width > positionwall.x;

            bool overlapy =
            position.y < positionwall.y + heightwall &&
            position.y + height > positionwall.y;

            if (overlapx && overlapy)
            {
                position.x = oldX;
                position.y = oldY;
                break;
            }
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