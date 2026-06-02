#include "InputSystem.h"

void InputSystem::handleInput(
    bool& running,
    Entity pacman,
    std::unordered_map<Entity, MovementComponent>& movements,
    std::unordered_map<Entity, DirectionComponent>& directions,
    std::unordered_map<Entity, FlashlightComponent>& flashlights,
    std::unordered_map<Entity, BatteryLifeComponent>& batteries,
    VisionMode& visionMode
) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                running = false;
                return;
            }

            if (!movements.contains(pacman)) {
                return;
            }

            auto& movement = movements[pacman];

            if (event.key.key == SDLK_RIGHT) {
                movement.vx = movement.speed;
                movement.vy = 0.0f;
                directions[pacman].current = Direction::Right;
            }
            else if (event.key.key == SDLK_LEFT) {
                movement.vx = -movement.speed;
                movement.vy = 0.0f;
                directions[pacman].current = Direction::Left;
            }
            else if (event.key.key == SDLK_UP) {
                movement.vx = 0.0f;
                movement.vy = -movement.speed;
                directions[pacman].current = Direction::Up;
            }
            else if (event.key.key == SDLK_DOWN) {
                movement.vx = 0.0f;
                movement.vy = movement.speed;
                directions[pacman].current = Direction::Down;
            }
            else if (event.key.key == 'f' || event.key.key == 'F') {
                if (
                    flashlights.contains(pacman) &&
                    flashlights[pacman].isAvailable &&
                    batteries.contains(pacman) &&
                    batteries[pacman].current > 0.0f
                ) {
                    flashlights[pacman].isOn = !flashlights[pacman].isOn;
                }
            }
            else if (event.key.key == '1') {
                visionMode = VisionMode::Full;
            }
            else if (event.key.key == '2') {
                visionMode = VisionMode::MediumDark;
            }
            else if (event.key.key == '3') {
                visionMode = VisionMode::FlashlightOnly;
            }
        }
    }
}