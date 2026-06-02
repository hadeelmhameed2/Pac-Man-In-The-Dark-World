#include "GameStateSystem.h"

void GameStateSystem::update(bagel::ent_type gameStateId, float deltaTime) {
    auto& state = bagel::World::getComponent<GameStateComponent>(gameStateId);
    if (state.isGameOver) return;
    state.batteryLevel -= 2.0f * deltaTime;
    if (state.batteryLevel < 0) state.batteryLevel = 0;
    if (state.batteryLevel > 100) state.batteryLevel = 100;
    if (state.batteryLevel <= 0) {
        // state.isGameOver = true;
    }
}