#pragma once

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr int TILE_SIZE = 32;

constexpr int FPS = 60;
constexpr int FRAME_DELAY_MS = 1000 / FPS;

constexpr float PACMAN_SPEED = 80.0f;
constexpr float GHOST_SPEED = 75.0f;
constexpr float GHOST_SCATTER_TIME = 7.0f;
constexpr float GHOST_CHASE_TIME = 20.0f;
constexpr float LIGHT_RADIUS = 180.0f;
constexpr float LIGHT_FADE_MARGIN = 60.0f;
constexpr float GHOST_MIN_OPACITY = 0.55f;

constexpr float BATTERY_DRAIN_RATE = 2.0f;
constexpr float BATTERY_DRAIN_PER_NEAR_GHOST = 0.8f;
constexpr float GHOST_NEAR_DISTANCE = 3.0f * TILE_SIZE;
constexpr float LOW_BATTERY_THRESHOLD = 25.0f;
constexpr float CLYDE_SHY_DISTANCE = 8.0f * TILE_SIZE;
constexpr float GHOST_HOUSE_X = WINDOW_WIDTH * 0.5f;
constexpr float GHOST_HOUSE_Y = WINDOW_HEIGHT * 0.5f;