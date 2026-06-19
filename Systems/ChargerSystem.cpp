#include "ChargerSystem.h"
#include "Constants.h"
#include "Maze.h"
#include "SoundManager.h"
#include <cmath>
#include <algorithm>

void ChargerSystem::initializeChargersFromMaze() {
    // Scan MAZE_LAYOUT for 'F' tiles and create charger entities
    for (int row = 0; row < MAZE_ROWS; ++row) {
        for (int col = 0; col < MAZE_COLS; ++col) {
            if (MAZE_LAYOUT[row][col] == 'F') {
                // Convert maze grid coordinates to pixel coordinates
                float x = MAZE_START_X + col * MAZE_TILE_SIZE;
                float y = MAZE_START_Y + row * MAZE_TILE_SIZE;

                // Create charger entity
                bagel::Entity charger = bagel::Entity::create();
                charger.add(PositionComponent{ x, y });

                // For now, create a mix of regular chargers and boost pads.
                // In a real scenario, you'd have a different marker for boost pads,
                // or configure them via a data file. For testing, let's make every
                // other charger a boost pad.
                bool isBoost = (row + col) % 3 == 0; // Simple deterministic pattern

                charger.add(ChargerComponent{
                    25.0f,     // chargePerSecond
                    isBoost,   // isBoost
                    5.0f,      // boostDuration
                    1.5f,      // boostRadiusMultiplier
                    1.5f       // boostDrainMultiplier
                });
            }
        }
    }
}

void ChargerSystem::update(float deltaTime, bagel::ent_type pacmanId) {
    // Reset occupancy flags each frame
    pacmanOnCharger_ = false;
    pacmanOnBoostPad_ = false;
    pacmanChargerId_ = bagel::ent_type{};

    // Get Pacman's position
    if (!bagel::World::mask(pacmanId).test(bagel::Component<PositionComponent>::Bit)) {
        return; // Pacman has no position component
    }

    const auto& pacPos = bagel::World::getComponent<PositionComponent>(pacmanId);
    float pacCenterX = pacPos.x + TILE_SIZE * 0.5f;
    float pacCenterY = pacPos.y + TILE_SIZE * 0.5f;

    // Query all charger entities and check distance-based occupancy
    static const bagel::Mask chargerMask = bagel::MaskBuilder()
        .set<PositionComponent>()
        .set<ChargerComponent>()
        .build();

    static int chargerQ = bagel::World::createQuery(chargerMask);

    const float CHARGER_OCCUPANCY_RADIUS = TILE_SIZE * 0.6f; // Detection radius

    for (bagel::Entity e = bagel::World::first(chargerQ); !bagel::World::eof(chargerQ); e = bagel::World::next(chargerQ)) {
        const auto& cPos = e.get<PositionComponent>();
        const auto& charger = e.get<ChargerComponent>();

        float chargerCenterX = cPos.x + TILE_SIZE * 0.5f;
        float chargerCenterY = cPos.y + TILE_SIZE * 0.5f;

        float dx = pacCenterX - chargerCenterX;
        float dy = pacCenterY - chargerCenterY;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= CHARGER_OCCUPANCY_RADIUS) {
            // Pacman is on this charger
            pacmanOnCharger_ = true;
            pacmanChargerId_ = e.entity();

            if (charger.isBoost) {
                pacmanOnBoostPad_ = true;
            }

            // === Apply charger effects to Pacman's battery ===
            if (bagel::World::mask(pacmanId).test(bagel::Component<BatteryLifeComponent>::Bit)) {
                auto& battery = bagel::World::getComponent<BatteryLifeComponent>(pacmanId);

                if (charger.isBoost) {
                    // Boost pad: trigger boost mode
                    if (battery.boostRemaining <= 0.0f) {
                        // Only trigger boost if not already active (or refresh if shorter)
                        battery.boostRemaining = charger.boostDuration;
                        SoundManager::getInstance().playSFX("turbine_surge");
                    } else if (charger.boostDuration > battery.boostRemaining) {
                        // Refresh if incoming boost is longer
                        battery.boostRemaining = charger.boostDuration;
                    }

                    // Also apply radius multiplier to LightComponent if present
                    if (e.has<LightComponent>()) {
                        // Note: This is applied to the charger's light, not Pacman's.
                        // Pacman's light boost should be applied in LightingSystem.
                    }
                } else {
                    // Regular charger: restore battery
                    battery.current += charger.chargePerSecond * deltaTime;
                    battery.current = std::min(battery.current, battery.max);
                }
            }

            // Only apply to the first charger Pacman is on (in case of overlap)
            break;
        }
    }
}

void ChargerSystem::initializeContactListener() {
    // Placeholder for Box2D contact listener integration.
    // In a future refactor:
    // 1. Create Box2D sensor fixtures for chargers with isSensor=true
    // 2. Implement BeginContact/EndContact listener
    // 3. Update pacmanOnCharger_/pacmanOnBoostPad_ flags in response to contact events
    // 4. Remove distance-based detection above
    //
    // This keeps the space reserved for the planned improvement.
}

