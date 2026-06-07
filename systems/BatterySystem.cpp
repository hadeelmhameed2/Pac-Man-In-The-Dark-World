#include "BatterySystem.h"

void BatterySystem::update(float deltaTime) {
    for (bagel::Entity e = bagel::Entity::first();!e.eof();e.next())
    {
        if (!e.has<InputComponent>())
            continue;

        auto& battery = e.get<BatteryLifeComponent>();
        auto& flashlight = e.get<FlashlightComponent>();

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
}