#include "MovementSystem.h"
#include "Maze.h"
#include "GridMovement.h"
#include <cmath>

namespace {
    bool isAtTileCenter(float x, float y) {
        const float cx = x + 16.0f;
        const float cy = y + 16.0f;
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
         .build();

    for (bagel::Entity e = bagel::Entity::first();
         !e.eof();
         e.next())
    {
        if (!e.test(moveMask))
            continue;

        auto& position = e.get<PositionComponent>();
        auto& movement = e.get<MovementComponent>();

        if (e.has<InputComponent>() && e.has<DirectionComponent>())
        {
            auto& direction = e.get<DirectionComponent>();

            // Calculate current tile
            float cx = position.x + 16.0f;
            float cy = position.y + 16.0f;
            int col = static_cast<int>(std::floor((cx - MAZE_START_X) / MAZE_TILE_SIZE));
            int row = static_cast<int>(std::floor((cy - MAZE_START_Y) / MAZE_TILE_SIZE));

            const bool atCenter = isAtTileCenter(position.x, position.y);

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
                        position.x = MAZE_START_X + col * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f - 16.0f;
                        position.y = MAZE_START_Y + row * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f - 16.0f;
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
                    position.x = MAZE_START_X + col * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f - 16.0f;
                    position.y = MAZE_START_Y + row * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f - 16.0f;
                } else {
                    // Re-assert velocities
                    if (direction.current == Direction::Right) { movement.vx = movement.speed; movement.vy = 0.0f; }
                    else if (direction.current == Direction::Left) { movement.vx = -movement.speed; movement.vy = 0.0f; }
                    else if (direction.current == Direction::Up) { movement.vx = 0.0f; movement.vy = -movement.speed; }
                    else if (direction.current == Direction::Down) { movement.vx = 0.0f; movement.vy = movement.speed; }
                }
            }
        }

        position.x += movement.vx * deltaTime;
        position.y += movement.vy * deltaTime;

        if (e.has<InputComponent>()) {
            enforceCardinalMovement(movement);
            snapEntityToGridLane(position, movement);
        }

        int width = 32;
        int height = 32;

        if (e.has<CollisionComponent>())
        {
            auto& collision = e.get<CollisionComponent>();
            width = collision.width;
            height = collision.height;
        }

        // Handle wrapping in side tunnels (row 14)
        if (std::abs(position.y - 387.0f) < 5.0f) {
            if (position.x + 16.0f < 70.0f) {
                position.x = 742.0f - 16.0f;
            } else if (position.x + 16.0f > 742.0f) {
                position.x = 70.0f - 16.0f;
            }
        } else {
            if (position.x < mazeLeft) {
                position.x = mazeLeft;
            }
            if (position.x + width > mazeRight) {
                position.x = mazeRight - width;
            }
        }

        if (position.y < mazeTop) {
            position.y = mazeTop;
        }

        if (position.y + height > mazeBottom) {
            position.y = mazeBottom - height;
        }

    }
}