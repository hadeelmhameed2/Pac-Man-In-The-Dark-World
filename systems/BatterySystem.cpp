#include "BatterySystem.h"

void BatterySystem::update(
    float deltaTime,
    Entity pacman,
    std::unordered_map<Entity, BatteryLifeComponent>& batteries,
    std::unordered_map<Entity, FlashlightComponent>& flashlights
) {
    if (!batteries.contains(pacman)) {
        return;
    }

    auto& battery = batteries[pacman];

    bool flashlightOn = false;

    if (flashlights.contains(pacman)) {
        flashlightOn = flashlights[pacman].isOn;
    }

    if (flashlightOn) {
        battery.current -= battery.flashlightDrainPerSecond * deltaTime;
    }
    else {
        battery.current -= battery.normalDrainPerSecond * deltaTime;
    }

    if (battery.current <= 0.0f) {
        battery.current = 0.0f;

        if (flashlights.contains(pacman)) {
            flashlights[pacman].isOn = false;
        }
    }

    if (battery.current > battery.max) {
        battery.current = battery.max;
    }
}