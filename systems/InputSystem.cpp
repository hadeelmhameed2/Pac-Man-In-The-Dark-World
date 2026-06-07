#include "InputSystem.h"

void InputSystem::handleInput(bool& running, VisionMode& visionMode) {

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

            for (bagel::Entity e = bagel::Entity::first();!e.eof();e.next())
            {
                    if (!e.has<InputComponent>())
                        continue;
                
                auto& direction = e.get<DirectionComponent>();
                auto& flashlight = e.get<FlashlightComponent>();
                auto& battery = e.get<BatteryLifeComponent>();


                if (event.key.key == SDLK_RIGHT) {
                    direction.desired = Direction::Right;
                }
                else if (event.key.key == SDLK_LEFT) {
                    direction.desired = Direction::Left;
                }
                else if (event.key.key == SDLK_UP) {
                    direction.desired = Direction::Up;
                }
                else if (event.key.key == SDLK_DOWN) {
                    direction.desired = Direction::Down;
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
}