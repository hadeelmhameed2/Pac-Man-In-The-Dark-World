#pragma once

#include "bagel.h"
#include <SDL3/SDL.h>

enum class Direction {
    Up,
    Down,
    Left,
    Right,
    None
};

enum class VisionMode {
    Full,
    MediumDark,
    FlashlightOnly
};

struct PositionComponent {
    float x = 0.0f;
    float y = 0.0f;
};

struct MovementComponent {
    float vx = 0.0f;
    float vy = 0.0f;
    float speed = 160.0f;
};

struct DrawingComponent {
    int width = 32;
    int height = 32;

    Uint8 r = 255;
    Uint8 g = 255;
    Uint8 b = 0;
    Uint8 a = 255;
};

struct CollisionComponent {
    int width = 32;
    int height = 32;
    bool isSolid = false;
};

struct InputComponent {
    bool controlledByPlayer = true;
};

enum class GhostState { CHASE, SCATTER, FRIGHTENED, EATEN };

struct GhostAI {
    GhostState state = GhostState::SCATTER;
    SDL_FPoint target{};
    int ghostType = 0;
    int lastTurnTileCol = -1;
    int lastTurnTileRow = -1;
};

struct VisibilityComponent {
    bool isVisible = false;
    float opacity = 0.0f;
};

struct GameStateComponent {
    float batteryLevel = 100.0f;
    int score = 0;
    bool isGameOver = false;
    bool isLowBattery = false;
    int nearbyGhosts = 0;
};

struct DirectionComponent {
    Direction current = Direction::None;
};

struct FlashlightComponent {
    bool isOn = false;
    bool isAvailable = true;
};

struct BatteryLifeComponent {
    float current = 100.0f;
    float max = 100.0f;
    float normalDrainPerSecond = 0.3f;
    float flashlightDrainPerSecond = 1.2f;
};

template <> struct bagel::Storage<PositionComponent> final : bagel::NoInstance {
    using type = bagel::PackedStorage<PositionComponent>;
};
template <> struct bagel::Storage<MovementComponent> final : bagel::NoInstance {
    using type = bagel::PackedStorage<MovementComponent>;
};
template <> struct bagel::Storage<DrawingComponent> final : bagel::NoInstance {
    using type = bagel::PackedStorage<DrawingComponent>;
};
template <> struct bagel::Storage<CollisionComponent> final : bagel::NoInstance {
    using type = bagel::PackedStorage<CollisionComponent>;
};
template <> struct bagel::Storage<DirectionComponent> final : bagel::NoInstance {
    using type = bagel::PackedStorage<DirectionComponent>;
};
template <> struct bagel::Storage<FlashlightComponent> final : bagel::NoInstance {
    using type = bagel::PackedStorage<FlashlightComponent>;
};
template <> struct bagel::Storage<BatteryLifeComponent> final : bagel::NoInstance {
    using type = bagel::PackedStorage<BatteryLifeComponent>;
};
template <> struct bagel::Storage<GhostAI> final : bagel::NoInstance {
    using type = bagel::PackedStorage<GhostAI>;
};
template <> struct bagel::Storage<VisibilityComponent> final : bagel::NoInstance {
    using type = bagel::PackedStorage<VisibilityComponent>;
};
template <> struct bagel::Storage<GameStateComponent> final : bagel::NoInstance {
    using type = bagel::PackedStorage<GameStateComponent>;
};
