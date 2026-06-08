#pragma once
#include "bagel.h"
#include "Components.h"

class GhostAISystem {
public:
    static void update(bagel::ent_type pacmanId, float deltaTime);
};