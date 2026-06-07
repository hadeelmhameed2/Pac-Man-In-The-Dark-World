#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

#include "bagel.h"
#include "Components.h"

#include "InputSystem.h"
#include "MovementSystem.h"
#include "RenderSystem.h"
#include "BatterySystem.h"


inline const std::vector<std::string> maze = {
    "############################",
    "#............##............#",
    "#.####.#####.##.#####.####.#",
    "#o####.#####.##.#####.####o#",
    "#.####.#####.##.#####.####.#",
    "#..........................#",
    "#.####.##.########.##.####.#",
    "#.####.##.########.##.####.#",
    "#......##....##....##......#",
    "######.##### ## #####.######",
    "     #.##### ## #####.#     ",
    "     #.##          ##.#     ",
    "     #.## ###--### ##.#     ",
    "######.## #      # ##.######",
    "      .   #      #   .      ",
    "######.## #      # ##.######",
    "     #.## ######## ##.#     ",
    "     #.##          ##.#     ",
    "     #.## ######## ##.#     ",
    "######.## ######## ##.######",
    "#............##............#",
    "#.####.#####.##.#####.####.#",
    "#o..##................##..o#",
    "###.##.##.########.##.##.###",
    "###.##.##.########.##.##.###",
    "#......##....##....##......#",
    "#.##########.##.##########.#",
    "#.##########.##.##########.#",
    "#..........................#",
    "############################"
};

constexpr int WALL_SIZE = 24;



template <> struct bagel::Storage<MovementComponent> final :NoInstance {
    using type = PackedStorage<MovementComponent>;
};


template <> struct bagel::Storage<CollisionComponent> final :NoInstance {
    using type = PackedStorage<CollisionComponent>;
};

template <> struct bagel::Storage<DrawingComponent> final :NoInstance {
    using type = PackedStorage<DrawingComponent>;
};

template <> struct bagel::Storage<PositionComponent> final :NoInstance {
    using type = PackedStorage<PositionComponent>;
};

template <> struct bagel::Storage<DirectionComponent> final : NoInstance {
    using type = PackedStorage<DirectionComponent>;
};




class Game {
public:
    bool init();
    void run();
    void clean();

private:
    void update(float deltaTime);
    void render();
    void createPacman();
    void createWalls();

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    bool running = false;
    VisionMode visionMode = VisionMode::Full;
    InputSystem inputSystem;
    MovementSystem movementSystem;
    RenderSystem renderSystem;
    BatterySystem batterySystem;
};