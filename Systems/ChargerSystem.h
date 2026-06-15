#pragma once

#include "bagel.h"
#include "Components.h"
#include "LightingComponents.h"

// ChargerSystem manages charger/boost pad entities and detects when Pacman
// occupies charger tiles. It currently uses distance-based detection as a
// temporary fallback; this will be replaced with Box2D sensor/contact-listener
// based detection in a future refactor.
//
// ChargerSystem is responsible for:
// 1. Creating charger entities from 'F' tiles in the maze layout
// 2. Detecting if Pacman is on a charger or boost pad (distance-based for now)
// 3. Coordinating with BatterySystem to apply charge/boost effects

class ChargerSystem {
public:
    // Scan maze layout for 'F' tiles and create charger entities.
    // Call once during initialization.
    void initializeChargersFromMaze();

    // Update charger occupancy detection and apply effects.
    // Call each frame. Returns true if Pacman is currently on any charger.
    void update(float deltaTime, bagel::ent_type pacmanId);

    // Query methods for charger occupancy
    bool isPacmanOnCharger() const { return pacmanOnCharger_; }
    bool isPacmanOnBoostPad() const { return pacmanOnBoostPad_; }

    // Get the charger entity that Pacman is currently on (if any)
    bagel::ent_type getPacmanCharger() const { return pacmanChargerId_; }

private:
    // Temporary storage for charger occupancy state
    bool pacmanOnCharger_ = false;
    bool pacmanOnBoostPad_ = false;
    bagel::ent_type pacmanChargerId_{};

    // Placeholder for future Box2D contact listener
    // This will replace distance-based detection.
    void initializeContactListener();
};

