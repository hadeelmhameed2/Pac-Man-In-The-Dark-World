#include "BatterySystem.h"

void BatterySystem::update(
    float deltaTime,
    bagel::Entity pacman
) {


    auto& battery =
    pacman.get<BatteryLifeComponent>();

    auto& flashlight =
        pacman.get<FlashlightComponent>();

    bool flashlightOn = flashlight.isOn;


    if (flashlightOn) {
        battery.current -= battery.flashlightDrainPerSecond * deltaTime;
    }
    else {
        battery.current -= battery.normalDrainPerSecond * deltaTime;
    }

    if (battery.current <= 0.0f) {
        battery.current = 0.0f;
        flashlight.isOn = false;
    }

    if (battery.current > battery.max) {
        battery.current = battery.max;
    }
}