#pragma once

#include "bagel.h"
#include <cstdint>

// PowerMode describes higher-level battery states. This is POD and contains
// only data (no logic) so it can be stored as a component or referenced by
// systems when needed.
enum class PowerMode : uint8_t {
    Normal = 0,
    PowerSaving,
    Boost,
    OutOfPower
};

// LightComponent holds configuration for an entity that emits light.
// This is POD only: systems will read and update the mutable fields.
struct LightComponent {
    // Base radius (when battery is full / no modifiers)
    float baseRadius = 180.0f;

    // Whether the light is directional (a cone) or omnidirectional (circle)
    bool directional = false;

    // Cone half-angle in degrees (if directional == true)
    float coneAngleDeg = 60.0f;

    // Maximum cone length (if directional)
    float coneLength = 320.0f;

    // Is the light enabled (e.g. flashlight toggled on/off)
    bool enabled = true;

    // Current effective radius (updated by LightingSystem every frame).
    // Systems should write to this, renderers read it.
    float currentRadius = 180.0f;
};

// ChargerComponent marks entities that can recharge batteries (e.g. charger
// pads). It is intentionally tiny so chargers can be spawned cheaply.
struct ChargerComponent {
    // How many battery units (same units as BatteryLifeComponent::current)
    // are restored per second while an entity is on/near this charger.
    float chargePerSecond = 25.0f;

    // If true, this charger is a 'boost' pad that grants a temporary power
    // boost when touched. The BatterySystem will use these fields to
    // activate a temporary PowerMode::Boost for nearby players.
    bool isBoost = false;
    float boostDuration = 5.0f; // seconds
    // Multiplier applied to light radius during boost (for visual effect).
    float boostRadiusMultiplier = 1.5f;
    // Multiplier applied to drain while boosted (boosts typically cost more).
    float boostDrainMultiplier = 1.5f;
};

// Storage specializations so bagel::World will use packed storage for fast
// iteration and cache-friendly access (consistent with other components).
template <> struct bagel::Storage<LightComponent> final : bagel::NoInstance {
    using type = bagel::PackedStorage<LightComponent>;
};

template <> struct bagel::Storage<ChargerComponent> final : bagel::NoInstance {
    using type = bagel::PackedStorage<ChargerComponent>;
};


