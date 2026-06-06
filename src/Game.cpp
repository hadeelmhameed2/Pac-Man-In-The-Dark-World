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

    running = true;
    return true;
}

void Game::createPacman() {
    /*positions[pacman] = PositionComponent{
        388.0f,
        690.0f
    };

    movements[pacman] = MovementComponent{
        0.0f,
        0.0f,
        160.0f
    };

    drawings[pacman] = DrawingComponent{
        32,
        32,
        255,
        255,
        0,
        255
    };

    collisions[pacman] = CollisionComponent{
        32,
        32,
        false
    };

    inputs[pacman] = InputComponent{
        true
    };

    directions[pacman] = DirectionComponent{
        Direction::Right
    };

    flashlights[pacman] = FlashlightComponent{
        false,
        true
    };

    batteries[pacman] = BatteryLifeComponent{
        100.0f,
        100.0f,
        0.3f,
        1.2f
    };*/

    pacman=bagel::Entity::create();
    pacman.addAll(
        BatteryLifeComponent{
        100.0f,
        100.0f,
        0.3f,
        1.2f
    },
    FlashlightComponent{
        false,
        true
    },
    DirectionComponent{
        Direction::Right
    },
    InputComponent{
        true
    },
    PositionComponent{
        388.0f,
        690.0f
    },

   MovementComponent{
        0.0f,
        0.0f,
        160.0f
    },

    DrawingComponent{
        32,
        32,
        255,
        255,
        0,
        255
    },

    CollisionComponent{
        32,
        32,
        false
    }
    );
}

void Game::run() {
    Uint64 previousTime = SDL_GetTicks();

    while (running) {
        Uint64 currentTime = SDL_GetTicks();
        float deltaTime = static_cast<float>(currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;

        inputSystem.handleInput(
            running,
            pacman,
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
        deltaTime,
        pacman
    );

    movementSystem.update(
        deltaTime,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
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