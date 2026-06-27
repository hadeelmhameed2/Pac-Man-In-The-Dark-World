#include "BatterySystem.h"
#include "LightingDebugSystem.h"
#include "LightingComponents.h"
#include <algorithm>
#include <iostream>

void BatterySystem::update(float deltaTime) {
    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.has<InputComponent>())
            continue;

        auto& battery = e.get<BatteryLifeComponent>();
        auto& flashlight = e.get<FlashlightComponent>();
        auto& pos = e.get<PositionComponent>();

        // === Boost mode countdown ===
        if (battery.boostRemaining > 0.0f) {
            battery.boostRemaining -= deltaTime;
            if (battery.boostRemaining <= 0.0f) {
                battery.boostRemaining = 0.0f;
            }
        }

        // === Power mode state transitions ===
        const float pct = (battery.max > 0.0f) ? ((battery.current / battery.max) * 100.0f) : 0.0f;

        if (battery.current <= 0.0f) {
            battery.current = 0.0f;
            battery.mode = PowerMode::OutOfPower;
            flashlight.isOn = false;
            flashlight.isAvailable = false;
            if (e.has<LightComponent>()) {
                auto& light = e.get<LightComponent>();
                light.enabled = false;
                light.currentRadius = 0.0f;
            }
        }
        else if (battery.boostRemaining > 0.0f) {
            battery.mode = PowerMode::Boost;
        }
        else if (pct > 50.0f) {
            battery.mode = PowerMode::Normal;
            if (!flashlight.isAvailable) {
                flashlight.isAvailable = true;
            }
        }
        else if (pct > 0.0f) {
            battery.mode = PowerMode::PowerSaving;
        }
        else {
            battery.mode = PowerMode::OutOfPower;
        }

        if (battery.mode != PowerMode::OutOfPower) {
            flashlight.isAvailable = true;
        }

        // === Drain calculations ===
        if (battery.current > 0.0f) {
            float drainRate = 0.0f;

            if (flashlight.isOn) {
                drainRate = battery.flashlightDrainPerSecond;
            } else {
                drainRate = battery.normalDrainPerSecond;
            }

            // Apply multipliers based on mode
            if (battery.mode == PowerMode::PowerSaving) {
                drainRate *= 0.75f; // 25% reduction in drain
            }
            else if (battery.mode == PowerMode::Boost) {
                // Boost drains faster (default 1.5x from ChargerComponent::boostDrainMultiplier)
                drainRate *= 1.5f;
            }

            battery.current -= drainRate * deltaTime;
        }

        battery.current = std::clamp(battery.current, 0.0f, battery.max);

        LightingDebugSystem::setState(battery.current, battery.max, battery.mode, battery.boostRemaining);
    }
}
