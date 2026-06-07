#pragma once

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

struct WallComponent{};