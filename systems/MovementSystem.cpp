#include "MovementSystem.h"

void MovementSystem::update(float deltaTime)
{
    const float mazeLeft = 70.0f;
    const float mazeTop = 55.0f;
    const float mazeRight = 742.0f;
    const float mazeBottom = 775.0f;

    static const bagel::Mask moveMask =
        bagel::MaskBuilder()
            .set<PositionComponent>()
            .set<MovementComponent>()
            .set<CollisionComponent>()
            .set<DirectionComponent>()
            .build();

    static const bagel::Mask wallMask =
        bagel::MaskBuilder()
            .set<WallComponent>()
            .set<PositionComponent>()
            .set<CollisionComponent>()
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
        auto& direction = e.get<DirectionComponent>();

        const int width = collision.width;
        const int height = collision.height;

        const float step = movement.speed * deltaTime;

        // =====================================
        // Phase 1: Try desired direction
        // =====================================

        float testX = position.x;
        float testY = position.y;

        switch (direction.desired)
        {
        case Direction::Left:
            testX -= step;
            break;

        case Direction::Right:
            testX += step;
            break;

        case Direction::Up:
            testY -= step;
            break;

        case Direction::Down:
            testY += step;
            break;

        default:
            break;
        }

        bool canTurn = true;

        for (bagel::Entity wall = bagel::Entity::first();
             !wall.eof();
             wall.next())
        {
            if (!wall.test(wallMask))
                continue;

            auto& wallPos = wall.get<PositionComponent>();
            auto& wallCol = wall.get<CollisionComponent>();

            bool overlapX =
                testX < wallPos.x + wallCol.width &&
                testX + width > wallPos.x;

            bool overlapY =
                testY < wallPos.y + wallCol.height &&
                testY + height > wallPos.y;

            if (overlapX && overlapY)
            {
                canTurn = false;
                break;
            }
        }

        if (canTurn)
        {
            direction.current = direction.desired;
        }

        // =====================================
        // Phase 2: Move using current direction
        // =====================================

        float moveX = position.x;
        float moveY = position.y;

        switch (direction.current)
        {
        case Direction::Left:
            moveX -= step;
            break;

        case Direction::Right:
            moveX += step;
            break;

        case Direction::Up:
            moveY -= step;
            break;

        case Direction::Down:
            moveY += step;
            break;

        default:
            break;
        }

        // =====================================
        // Phase 3: Check movement collision
        // =====================================

        bool blocked = false;

        for (bagel::Entity wall = bagel::Entity::first();
             !wall.eof();
             wall.next())
        {
            if (!wall.test(wallMask))
                continue;

            auto& wallPos = wall.get<PositionComponent>();
            auto& wallCol = wall.get<CollisionComponent>();

            bool overlapX =
                moveX < wallPos.x + wallCol.width &&
                moveX + width > wallPos.x;

            bool overlapY =
                moveY < wallPos.y + wallCol.height &&
                moveY + height > wallPos.y;

            if (overlapX && overlapY)
            {
                blocked = true;
                break;
            }
        }

        if (!blocked)
        {
            position.x = moveX;
            position.y = moveY;
        }

        // =====================================
        // Maze boundaries
        // =====================================

        if (position.x < mazeLeft)
            position.x = mazeLeft;

        if (position.y < mazeTop)
            position.y = mazeTop;

        if (position.x + width > mazeRight)
            position.x = mazeRight - width;

        if (position.y + height > mazeBottom)
            position.y = mazeBottom - height;
    }
}