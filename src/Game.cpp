#include "Game.h"
#include "Constants.h"

#include <iostream>

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
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    createPacman();

    running = true;
    return true;
}

void Game::createPacman() {
    positions[pacman] = PositionComponent{
        100.0f,
        100.0f
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
}

void Game::run() {
    Uint64 previousTime = SDL_GetTicks();

    while (running) {
        Uint64 currentTime = SDL_GetTicks();
        float deltaTime = static_cast<float>(currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;

        handleEvents();
        update(deltaTime);
        render();

        SDL_Delay(FRAME_DELAY_MS);
    }
}

void Game::handleEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                running = false;
            }

            auto& movement = movements[pacman];

            if (event.key.key == SDLK_RIGHT) {
                movement.vx = movement.speed;
                movement.vy = 0.0f;
            }
            else if (event.key.key == SDLK_LEFT) {
                movement.vx = -movement.speed;
                movement.vy = 0.0f;
            }
            else if (event.key.key == SDLK_UP) {
                movement.vx = 0.0f;
                movement.vy = -movement.speed;
            }
            else if (event.key.key == SDLK_DOWN) {
                movement.vx = 0.0f;
                movement.vy = movement.speed;
            }
        }
    }
}

void Game::update(float deltaTime) {
    auto& position = positions[pacman];
    auto& movement = movements[pacman];

    position.x += movement.vx * deltaTime;
    position.y += movement.vy * deltaTime;

    if (position.x < 0) {
        position.x = 0;
    }

    if (position.y < 0) {
        position.y = 0;
    }

    if (position.x + collisions[pacman].width > WINDOW_WIDTH) {
        position.x = WINDOW_WIDTH - collisions[pacman].width;
    }

    if (position.y + collisions[pacman].height > WINDOW_HEIGHT) {
        position.y = WINDOW_HEIGHT - collisions[pacman].height;
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    auto& position = positions[pacman];
    auto& drawing = drawings[pacman];

    SDL_FRect pacmanRect = {
        position.x,
        position.y,
        static_cast<float>(drawing.width),
        static_cast<float>(drawing.height)
    };

    SDL_SetRenderDrawColor(
        renderer,
        drawing.r,
        drawing.g,
        drawing.b,
        drawing.a
    );

    SDL_RenderFillRect(renderer, &pacmanRect);

    SDL_RenderPresent(renderer);
}

void Game::clean() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}