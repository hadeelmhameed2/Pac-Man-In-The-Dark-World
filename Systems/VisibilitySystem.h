#pragma once
#include "bagel.h"
#include "Components.h"

class VisibilitySystem {
public:
    static void update(bagel::ent_type pacmanId, bagel::ent_type gameStateId, float deltaTime);
};