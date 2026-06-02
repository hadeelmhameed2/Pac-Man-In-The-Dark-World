#pragma once
#include "../ECS/bagel.h"
#include "../ECS/Components.h"

class GameStateSystem {
public:
    static void update(bagel::ent_type gameStateId, float deltaTime);
};