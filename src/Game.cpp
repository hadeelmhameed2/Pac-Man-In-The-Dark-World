#include "Game.h"
#include "Constants.h"

#include <iostream>
#include <string>

bool Game::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(
        "Pacman in the Dark World",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    if (!window) {
        std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        window = nullptr;
        SDL_Quit();
        return false;
    }

    createPacman();
    createWalls();
    running = true;
    return true;
}

void Game::createWalls()
{
    const float startX = 70.0f;
    const float startY = 55.0f;

    int count=0;

    for (int row = 0; row < static_cast<int>(maze.size()); ++row)
    {
        for (int col = 0; col < static_cast<int>(maze[row].size()); ++col)
        {
            if (maze[row][col] != '#')
                continue;

            float x = startX + col * WALL_SIZE;
            float y = startY + row * WALL_SIZE;

            bagel::Entity wall = bagel::Entity::create();

            wall.addAll(
                WallComponent{},

                PositionComponent{
                    x,
                    y
                },

                CollisionComponent{
                    static_cast<int>(WALL_SIZE),
                    static_cast<int>(WALL_SIZE),
                    true
                }
            );
            std::cout<<count++<<std::endl;
        }
    }

    std::cout << "\n=== Wall Created ===\n";
}

void Game::createPacman() {

    auto pacman = bagel::Entity::create();

    pacman.addAll(
        BatteryLifeComponent{100.0f,100.0f,0.3f,1.2f},
        FlashlightComponent{false,true},
        DirectionComponent{Direction::None, Direction::None},
        InputComponent{true},
        PositionComponent{360.0f,690.0f},
        MovementComponent{0.0f,0.0f,105.0f},
        DrawingComponent{20,20,255,255,0,255},
        CollisionComponent{20,20,false}
    );

    std::cout << "\n=== PACMAN COMPONENTS ===\n";

    if (pacman.has<PositionComponent>())
    {
        auto& p = pacman.get<PositionComponent>();
        std::cout
            << "Position: x=" << p.x
            << " y=" << p.y
            << std::endl;
    }

    if (pacman.has<MovementComponent>())
    {
        auto& m = pacman.get<MovementComponent>();
        std::cout
            << "Movement: vx=" << m.vx
            << " vy=" << m.vy
            << " speed=" << m.speed
            << std::endl;
    }

    if (pacman.has<DrawingComponent>())
    {
        auto& d = pacman.get<DrawingComponent>();
        std::cout
            << "Drawing: w=" << d.width
            << " h=" << d.height
            << " rgba=("
            << (int)d.r << ","
            << (int)d.g << ","
            << (int)d.b << ","
            << (int)d.a << ")"
            << std::endl;
    }

    if (pacman.has<CollisionComponent>())
    {
        auto& c = pacman.get<CollisionComponent>();
        std::cout
            << "Collision: w=" << c.width
            << " h=" << c.height
            << " solid=" << c.isSolid
            << std::endl;
    }

    if (pacman.has<DirectionComponent>())
    {
        auto& dir = pacman.get<DirectionComponent>();
        std::cout
            << "Direction: "
            << static_cast<int>(dir.current)
            << std::endl;
    }

    if (pacman.has<FlashlightComponent>())
    {
        auto& f = pacman.get<FlashlightComponent>();
        std::cout
            << "Flashlight: on=" << f.isOn
            << " available=" << f.isAvailable
            << std::endl;
    }

    if (pacman.has<BatteryLifeComponent>())
    {
        auto& b = pacman.get<BatteryLifeComponent>();
        std::cout
            << "Battery: current=" << b.current
            << " max=" << b.max
            << " drain=" << b.normalDrainPerSecond
            << " flashlightDrain=" << b.flashlightDrainPerSecond
            << std::endl;
    }

    if (pacman.has<InputComponent>())
    {
        auto& i = pacman.get<InputComponent>();
        std::cout
            << "Input: controlled="
            << i.controlledByPlayer
            << std::endl;
    }

    std::cout << "=========================\n";

}

void Game::run() {
    Uint64 previousTime = SDL_GetTicks();

    while (running) {
        Uint64 currentTime = SDL_GetTicks();
        float deltaTime = static_cast<float>(currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;

        inputSystem.handleInput(
            running,
            visionMode
        );

        renderSystem.drawStatus(window,visionMode);
        update(deltaTime);
        render();

        SDL_Delay(FRAME_DELAY_MS);
    }
}

void Game::update(float deltaTime) {


    batterySystem.update(
        deltaTime
    );

    movementSystem.update(
        deltaTime
    );


}

void Game::render() {
    renderSystem.render(
        renderer,
        visionMode
    );
}

void Game::clean() {
    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}