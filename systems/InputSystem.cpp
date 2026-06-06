#include "InputSystem.h"

void InputSystem::handleInput(
bool& running,
 bagel::Entity pacman,
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

            /*if (!movements.contains(pacman)) {
                return;
            }*/
            auto& movement = pacman.get<MovementComponent>();
            auto& direction = pacman.get<DirectionComponent>();
            auto& flashlight = pacman.get<FlashlightComponent>();
            auto& battery = pacman.get<BatteryLifeComponent>();


            if (event.key.key == SDLK_RIGHT) {
                movement.vx = movement.speed;
                movement.vy = 0.0f;
                direction.current = Direction::Right;
            }
            else if (event.key.key == SDLK_LEFT) {
                movement.vx = -movement.speed;
                movement.vy = 0.0f;
                 direction.current = Direction::Left;
            }
            else if (event.key.key == SDLK_UP) {
                movement.vx = 0.0f;
                movement.vy = -movement.speed;
                 direction.current = Direction::Up;
            }
            else if (event.key.key == SDLK_DOWN) {
                movement.vx = 0.0f;
                movement.vy = movement.speed;
                 direction.current = Direction::Down;
            }
            else if (event.key.key == 'f' || event.key.key == 'F') {
                if (
                    flashlight.isAvailable &&
                    battery.current > 0.0f
                ) {
                    flashlight.isOn = !flashlight.isOn;
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