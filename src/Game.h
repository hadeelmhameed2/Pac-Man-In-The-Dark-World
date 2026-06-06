#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>
#include <box2d/box2d.h>

#include "bagel.h"
#include "Entity.h"
#include "Components.h"

#include "InputSystem.h"
#include "MovementSystem.h"
#include "RenderSystem.h"
#include "BatterySystem.h"

//using namespace bagel;

/*
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
};*/

/*
template <> struct bagel::Storage<FlashlightComponent> final : NoInstance {
    using type = SparseStorage<FlashlightComponent>;
};

template <> struct bagel::Storage<BatteryLifeComponent> final : NoInstance {
    using type = SparseStorage<BatteryLifeComponent>;
};


template <> struct bagel::Storage<InputComponent> final : NoInstance {
    using type = TaggedStorage<InputComponent>;
};*/



class Game {
public:
    bool init();
    void run();
    void clean();

private:
    void update(float deltaTime);
    void render();
    void createPacman();

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    bool running = false;

    bagel::Entity pacman = bagel::Entity{{-1}};

    VisionMode visionMode = VisionMode::Full;

    /*std::unordered_map<Entity, PositionComponent> positions;
    std::unordered_map<Entity, MovementComponent> movements;
    std::unordered_map<Entity, DrawingComponent> drawings;
    std::unordered_map<Entity, CollisionComponent> collisions;
    std::unordered_map<Entity, InputComponent> inputs;
    std::unordered_map<Entity, DirectionComponent> directions;
    std::unordered_map<Entity, FlashlightComponent> flashlights;
    std::unordered_map<Entity, BatteryLifeComponent> batteries;*/

    InputSystem inputSystem;
    MovementSystem movementSystem;
    RenderSystem renderSystem;
    BatterySystem batterySystem;
};