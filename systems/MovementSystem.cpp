#include "MovementSystem.h"

bool MovementSystem::canMove(
    float x,
    float y,
    int width,
    int height,
    Direction direction,
    float probeDistance,
    bool forgiving)
{
    float testX = x;
    float testY = y;

    switch (direction)
    {
        case Direction::Left:
            testX -= probeDistance;
            break;

        case Direction::Right:
            testX += probeDistance;
            break;

        case Direction::Up:
            testY -= probeDistance;
            break;

        case Direction::Down:
            testY += probeDistance;
            break;

        case Direction::None:
            return false;
    }

    if (forgiving)
    {
        constexpr float TILE_SIZE = 24.0f;

        const float marginX =
            (TILE_SIZE - width) / 2.0f;

        const float marginY =
            (TILE_SIZE - height) / 2.0f;

        switch (direction)
        {
            case Direction::Up:
            case Direction::Down:
            {
                testY -= marginY;
                height += marginY * 2;
                break;
            }

            case Direction::Left:
            case Direction::Right:
            {
                testX -= marginX;
                width += marginX * 2;
                break;
            }

            default:
                break;
        }
    }

    static const bagel::Mask wallMask =
        bagel::MaskBuilder()
            .set<WallComponent>()
            .set<PositionComponent>()
            .set<CollisionComponent>()
            .build();

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
            return false;
        }
    }

    return true;
}


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

        float step = movement.speed * deltaTime;

        constexpr float TURN_STEP = 2.0f;

        // Try desired direction first
        bool desiredBlocked =
            !canMove(
                position.x,
                position.y,
                width,
                height,
                direction.desired,
                TURN_STEP,
                true      // forgiving
            );


        // Check current movement
        bool currentBlocked =
            !canMove(
                position.x,
                position.y,
                width,
                height,
                direction.current,
                step,
                false     // exact collision
            );



        if (currentBlocked)
        {
            direction.current = Direction::None;
        }

        if (!desiredBlocked)
        {
            direction.current = direction.desired;
        }


        // Move
        switch (direction.current)
        {
            case Direction::Left:
                position.x -= step;
                break;

            case Direction::Right:
                position.x += step;
                break;

            case Direction::Up:
                position.y -= step;
                break;

            case Direction::Down:
                position.y += step;
                break;

            default:
                break;
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