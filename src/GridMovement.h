#pragma once

#include "Components.h"
#include "Constants.h"
#include <cmath>

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
    if (std::abs(move.vx) > 0.1f) {
        pos.y = std::round(pos.y / TILE_SIZE) * TILE_SIZE;
    } else if (std::abs(move.vy) > 0.1f) {
        pos.x = std::round(pos.x / TILE_SIZE) * TILE_SIZE;
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
