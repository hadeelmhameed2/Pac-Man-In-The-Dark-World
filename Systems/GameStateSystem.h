#pragma once
#include "bagel.h"
#include "Components.h"
#include "RenderSystem.h"

constexpr float DAMAGE = 35;

class GameStateSystem {
public:
    static void update(bagel::ent_type pacmanId, float deltaTime);
    static GameStateComponent* getState();
};
