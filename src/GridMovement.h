#pragma once

#include "Components.h"
#include "Constants.h"
#include <cmath>

#include "Maze.h"

inline void enforceCardinalMovement(MovementComponent& move) {
    if (move.vx != 0.0f && move.vy != 0.0f) {
        if (std::abs(move.vx) >= std::abs(move.vy)) {
            move.vy = 0.0f;
        } else {
            move.vx = 0.0f;
        }
    }
}

inline void snapEntityToGridLane(PositionComponent& pos, const MovementComponent& move) {
    // Entities are 32x32. Their center coordinates are pos.x + 16, pos.y + 16.
    // We snap them so that the entity center aligns with the tile center:
    // tileCenterX = MAZE_START_X + col * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f
    // tileCenterY = MAZE_START_Y + row * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f
    if (std::abs(move.vx) > 0.1f) {
        float row = std::round((pos.y + 16.0f - MAZE_START_Y - MAZE_TILE_SIZE * 0.5f) / MAZE_TILE_SIZE);
        pos.y = MAZE_START_Y + row * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f - 16.0f;
    } else if (std::abs(move.vy) > 0.1f) {
        float col = std::round((pos.x + 16.0f - MAZE_START_X - MAZE_TILE_SIZE * 0.5f) / MAZE_TILE_SIZE);
        pos.x = MAZE_START_X + col * MAZE_TILE_SIZE + MAZE_TILE_SIZE * 0.5f - 16.0f;
    }
}

inline void clampToScreen(PositionComponent& pos, int width, int height) {
    if (pos.x < 0.0f) {
        pos.x = 0.0f;
    }
    if (pos.y < 0.0f) {
        pos.y = 0.0f;
    }
    if (pos.x + width > WINDOW_WIDTH) {
        pos.x = WINDOW_WIDTH - width;
    }
    if (pos.y + height > WINDOW_HEIGHT) {
        pos.y = WINDOW_HEIGHT - height;
    }
}
