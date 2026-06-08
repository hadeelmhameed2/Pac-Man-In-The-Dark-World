#include "MovementSystem.h"
#include "Maze.h"
#include "GridMovement.h"
#include <box2d/box2d.h>
#include <cmath>

namespace {
    bool isAtTileCenter(float cx, float cy) {
        const float rx = cx - MAZE_START_X;
        const float ry = cy - MAZE_START_Y;
        const float tx = std::fmod(std::fmod(rx, MAZE_TILE_SIZE) + MAZE_TILE_SIZE, MAZE_TILE_SIZE);
        const float ty = std::fmod(std::fmod(ry, MAZE_TILE_SIZE) + MAZE_TILE_SIZE, MAZE_TILE_SIZE);
        constexpr float eps = 4.0f;
        return std::abs(tx - MAZE_TILE_SIZE * 0.5f) <= eps && std::abs(ty - MAZE_TILE_SIZE * 0.5f) <= eps;
    }
}

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
         .set<PhysicsComponent>()
         .build();

    for (bagel::Entity e = bagel::Entity::first();
         !e.eof();
         e.next())
    {
        if (!e.test(moveMask))
            continue;

        auto& movement = e.get<MovementComponent>();
        auto& physics = e.get<PhysicsComponent>();
        b2BodyId bodyId = physics.bodyId;

        // Use Box2D body position as source of truth
        b2Vec2 pos = b2Body_GetPosition(bodyId);
        float cx = pos.x;
        float cy = pos.y;

        if (e.has<InputComponent>() && e.has<DirectionComponent>())
        {
            auto& direction = e.get<DirectionComponent>();

            int col = static_cast<int>(std::floor((cx - MAZE_START_X) / MAZE_TILE_SIZE));
            int row = static_cast<int>(std::floor((cy - MAZE_START_Y) / MAZE_TILE_SIZE));

            const bool atCenter = isAtTileCenter(cx, cy);

            // Handle turning at tile center
            if (atCenter) {
                if (direction.queued != Direction::None) {
                    int nextCol = col;
                    int nextRow = row;
                    if (direction.queued == Direction::Right) nextCol++;
                    else if (direction.queued == Direction::Left) nextCol--;
                    else if (direction.queued == Direction::Up) nextRow--;
                    else if (direction.queued == Direction::Down) nextRow++;

                    if (!isWall(nextCol, nextRow)) {
                        direction.current = direction.queued;
                        direction.queued = Direction::None;

                        // Snap perfectly to intersection
                        cx = MAZE_START_X + col * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f;
                        cy = MAZE_START_Y + row * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f;
                        b2Body_SetTransform(bodyId, b2Vec2{cx, cy}, b2Rot_identity);
                    }
                }

                // Check if current direction is blocked by a wall ahead
                int aheadCol = col;
                int aheadRow = row;
                if (direction.current == Direction::Right) aheadCol++;
                else if (direction.current == Direction::Left) aheadCol--;
                else if (direction.current == Direction::Up) aheadRow--;
                else if (direction.current == Direction::Down) aheadRow++;

                if (isWall(aheadCol, aheadRow)) {
                    movement.vx = 0.0f;
                    movement.vy = 0.0f;
                    // Snap perfectly to intersection
                    cx = MAZE_START_X + col * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f;
                    cy = MAZE_START_Y + row * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f;
                    b2Body_SetTransform(bodyId, b2Vec2{cx, cy}, b2Rot_identity);
                } else {
                    // Re-assert velocities
                    if (direction.current == Direction::Right) { movement.vx = movement.speed; movement.vy = 0.0f; }
                    else if (direction.current == Direction::Left) { movement.vx = -movement.speed; movement.vy = 0.0f; }
                    else if (direction.current == Direction::Up) { movement.vx = 0.0f; movement.vy = -movement.speed; }
                    else if (direction.current == Direction::Down) { movement.vx = 0.0f; movement.vy = movement.speed; }
                }
            }
        }

        // Apply velocity to the Box2D body
        b2Body_SetLinearVelocity(bodyId, b2Vec2{movement.vx, movement.vy});

        if (e.has<InputComponent>()) {
            enforceCardinalMovement(movement);
            
            // Snap to lane in Box2D
            b2Vec2 snappedPos = b2Body_GetPosition(bodyId);
            if (std::abs(movement.vx) > 0.1f) {
                float row = std::round((snappedPos.y - MAZE_START_Y - MAZE_TILE_SIZE * 0.5f) / MAZE_TILE_SIZE);
                float targetY = MAZE_START_Y + row * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f;
                b2Body_SetTransform(bodyId, b2Vec2{snappedPos.x, targetY}, b2Rot_identity);
            } else if (std::abs(movement.vy) > 0.1f) {
                float col = std::round((snappedPos.x - MAZE_START_X - MAZE_TILE_SIZE * 0.5f) / MAZE_TILE_SIZE);
                float targetX = MAZE_START_X + col * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f;
                b2Body_SetTransform(bodyId, b2Vec2{targetX, snappedPos.y}, b2Rot_identity);
            }
        }

        // Get latest position for boundary/wrap checks
        b2Vec2 finalPos = b2Body_GetPosition(bodyId);

        // Handle wrapping in side tunnels (row 14)
        if (std::abs(finalPos.y - 403.0f) < 5.0f) {
            if (finalPos.x < 70.0f) {
                b2Body_SetTransform(bodyId, b2Vec2{742.0f, finalPos.y}, b2Rot_identity);
            } else if (finalPos.x > 742.0f) {
                b2Body_SetTransform(bodyId, b2Vec2{70.0f, finalPos.y}, b2Rot_identity);
            }
        } else {
            // Apply boundary clamping
            float targetX = finalPos.x;
            float targetY = finalPos.y;
            if (finalPos.x < mazeLeft + 16.0f) {
                targetX = mazeLeft + 16.0f;
            } else if (finalPos.x > mazeRight - 16.0f) {
                targetX = mazeRight - 16.0f;
            }
            if (finalPos.y < mazeTop + 16.0f) {
                targetY = mazeTop + 16.0f;
            } else if (finalPos.y > mazeBottom - 16.0f) {
                targetY = mazeBottom - 16.0f;
            }

            if (targetX != finalPos.x || targetY != finalPos.y) {
                b2Body_SetTransform(bodyId, b2Vec2{targetX, targetY}, b2Rot_identity);
            }
        }
    }
}