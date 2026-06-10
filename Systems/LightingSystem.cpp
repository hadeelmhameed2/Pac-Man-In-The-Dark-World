#include "LightingSystem.h"
#include "Constants.h"
#include <cmath>
#include <algorithm>

namespace {
SDL_Renderer* gLightingRenderer = nullptr;
SDL_Texture* gShadowMaskTexture = nullptr;

bool pointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy) {
    const float d1 = (px - bx) * (ay - by) - (ax - bx) * (py - by);
    const float d2 = (px - cx) * (by - cy) - (bx - cx) * (py - cy);
    const float d3 = (px - ax) * (cy - ay) - (cx - ax) * (py - ay);

    const bool hasNeg = (d1 < 0.0f) || (d2 < 0.0f) || (d3 < 0.0f);
    const bool hasPos = (d1 > 0.0f) || (d2 > 0.0f) || (d3 > 0.0f);
    return !(hasNeg && hasPos);
}

void drawFilledCircle(SDL_Renderer* renderer, float centerX, float centerY, float radius) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

    for (int y = static_cast<int>(-radius); y <= static_cast<int>(radius); ++y) {
        for (int x = static_cast<int>(-radius); x <= static_cast<int>(radius); ++x) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderPoint(renderer, centerX + static_cast<float>(x), centerY + static_cast<float>(y));
            }
        }
    }
}

void drawConeCutout(SDL_Renderer* renderer, float centerX, float centerY, Direction direction, float length, float width) {
    float x1 = centerX, y1 = centerY;
    float x2 = centerX, y2 = centerY;
    float x3 = centerX, y3 = centerY;

    if (direction == Direction::Right) {
        x2 = centerX + length; y2 = centerY - width * 0.5f;
        x3 = centerX + length; y3 = centerY + width * 0.5f;
    } else if (direction == Direction::Left) {
        x2 = centerX - length; y2 = centerY - width * 0.5f;
        x3 = centerX - length; y3 = centerY + width * 0.5f;
    } else if (direction == Direction::Up) {
        x2 = centerX - width * 0.5f; y2 = centerY - length;
        x3 = centerX + width * 0.5f; y3 = centerY - length;
    } else if (direction == Direction::Down) {
        x2 = centerX - width * 0.5f; y2 = centerY + length;
        x3 = centerX + width * 0.5f; y3 = centerY + length;
    } else {
        return;
    }

    SDL_Vertex vertices[3];
    for (int i = 0; i < 3; ++i) {
        vertices[i].color = {0.0f, 0.0f, 0.0f, 0.0f};
    }
    vertices[0].position = {x1, y1};
    vertices[1].position = {x2, y2};
    vertices[2].position = {x3, y3};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_RenderGeometry(renderer, nullptr, vertices, 3, nullptr, 0);
}

void clearShadowMask() {
    if (gLightingRenderer == nullptr) {
        return;
    }

    if (gShadowMaskTexture == nullptr) {
        gShadowMaskTexture = SDL_CreateTexture(
            gLightingRenderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            WINDOW_WIDTH,
            WINDOW_HEIGHT
        );
        if (gShadowMaskTexture != nullptr) {
            SDL_SetTextureBlendMode(gShadowMaskTexture, SDL_BLENDMODE_BLEND);
        }
    }

    if (gShadowMaskTexture == nullptr) {
        return;
    }

    SDL_SetRenderTarget(gLightingRenderer, gShadowMaskTexture);
    SDL_SetRenderDrawBlendMode(gLightingRenderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(gLightingRenderer, 0, 0, 0, 255);
    SDL_FRect full = {0.0f, 0.0f, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)};
    SDL_RenderFillRect(gLightingRenderer, &full);
}

void cutoutLight(SDL_Renderer* renderer, float centerX, float centerY, float radius, bool directional, Direction direction, float coneLength, float coneWidth) {
    if (radius > 0.0f) {
        drawFilledCircle(renderer, centerX, centerY, radius);
    }
    if (directional) {
        drawConeCutout(renderer, centerX, centerY, direction, coneLength, coneWidth);
    }
}
} // namespace

// Small helper to convert Direction enum to a unit vector. We keep this
// function inline and trivial so systems can cheaply compute cone checks
// without branching on stringly-typed values.
static inline void directionToVector(Direction dir, float &outX, float &outY) {
    switch (dir) {
        case Direction::Up:    outX = 0.0f;  outY = -1.0f; break;
        case Direction::Down:  outX = 0.0f;  outY = 1.0f;  break;
        case Direction::Left:  outX = -1.0f; outY = 0.0f;  break;
        case Direction::Right: outX = 1.0f;  outY = 0.0f;  break;
        default:               outX = 0.0f;  outY = 0.0f;  break;
    }
}

void LightingSystem::setRenderer(SDL_Renderer* renderer) {
    if (gLightingRenderer != renderer) {
        gLightingRenderer = renderer;
        if (gShadowMaskTexture != nullptr) {
            SDL_DestroyTexture(gShadowMaskTexture);
            gShadowMaskTexture = nullptr;
        }
    }
}

SDL_Texture* LightingSystem::getShadowMaskTexture() const {
    return gShadowMaskTexture;
}

void LightingSystem::update(float deltaTime) {
    // Query for the pacman (player) entity. We identify the player as the
    // first entity that has an InputComponent and a PositionComponent.
    static const bagel::Mask playerMask = bagel::MaskBuilder()
        .set<InputComponent>()
        .set<PositionComponent>()
        .set<DirectionComponent>()
        .set<FlashlightComponent>()
        .set<BatteryLifeComponent>()
        .build();

    static int playerQ = bagel::World::createQuery(playerMask);
    if (bagel::World::eof(playerQ)) {
        // No player present - nothing to do.
        return;
    }

    bagel::Entity player = bagel::World::first(playerQ);
    const auto& ppos = player.get<PositionComponent>();
    const auto& pdir = player.get<DirectionComponent>();
    const auto& pflash = player.get<FlashlightComponent>();
    const auto& pbatt = player.get<BatteryLifeComponent>();
    const bool hasLightComp = player.has<LightComponent>();
    auto* pLight = hasLightComp ? &player.get<LightComponent>() : nullptr;

    // Player center (assume TILE_SIZE sized entity for centering consistency
    // with other systems that use TILE_SIZE). Using the entity's center is
    // important for smooth lighting / cone math.
    const float px = ppos.x + TILE_SIZE * 0.5f;
    const float py = ppos.y + TILE_SIZE * 0.5f;

    // Effective battery fraction in [0..1]
    const float batteryFraction = (pbatt.max > 0.0f) ? (pbatt.current / pbatt.max) : 0.0f;

    // Base light radius scaled by battery level. If LightComponent exists we
    // respect its settings; otherwise we fall back to the flashlight state.
    float effectiveRadius = hasLightComp ? pLight->baseRadius : LIGHT_RADIUS;
    effectiveRadius *= batteryFraction;
    const bool playerLightEnabled = hasLightComp ? (pLight->enabled && pflash.isOn && pbatt.current > 0.0f) : (pflash.isOn && pbatt.current > 0.0f);
    if (!playerLightEnabled) {
        effectiveRadius *= 0.0f;
    }
    if (hasLightComp) {
        pLight->currentRadius = effectiveRadius;
    }

    // Prepare directional cone parameters if the player's flashlight is
    // treated as directional. We use a dot-product comparison (fast) to
    // determine whether a target lies inside the cone. This avoids costly
    // atan2 / trig per target.
    float dirX = 0.0f, dirY = 0.0f;
    directionToVector(pdir.current, dirX, dirY);

    // Convert half-angle to cosine for fast dot-product comparison
    const float coneHalfAngleDeg = hasLightComp ? pLight->coneAngleDeg : 45.0f; // tuned default; can be changed per-entity
    const float coneHalfAngleRad = coneHalfAngleDeg * (3.14159265358979323846f / 180.0f);
    const float coneCos = std::cos(coneHalfAngleRad);
    const bool directionalLight = hasLightComp ? pLight->directional : true;
    const float coneLength = hasLightComp ? pLight->coneLength : 320.0f;

    // Smooth and fade parameters - mirror VisibilitySystem's behaviour so
    // transitions match rendering.
    const float fadeMargin = LIGHT_FADE_MARGIN;
    const float smoothSpeed = 6.0f * deltaTime;

    // Query all entities that have a VisibilityComponent and a Position -
    // these are the targets we will update (e.g. ghosts and other objects).
    static const bagel::Mask targetMask = bagel::MaskBuilder()
        .set<PositionComponent>()
        .set<VisibilityComponent>()
        .build();

    static int targetQ = bagel::World::createQuery(targetMask);

    for (bagel::Entity e = bagel::World::first(targetQ); !bagel::World::eof(targetQ); e = bagel::World::next(targetQ)) {
        auto& vis = e.get<VisibilityComponent>();
        const auto& tpos = e.get<PositionComponent>();

        const float tx = tpos.x + TILE_SIZE * 0.5f;
        const float ty = tpos.y + TILE_SIZE * 0.5f;

        const float dx = tx - px;
        const float dy = ty - py;
        const float dist = std::sqrt(dx * dx + dy * dy);

        float litAmount = 0.0f;

        // If within the basic radial radius, fully lit (subject to cone if
        // flashlight is directional and on). If outside but within fade
        // margin, compute a smooth falloff.
        if (dist < effectiveRadius) {
            litAmount = 1.0f;

            // If flashlight is directional and turned on, additionally
            // check the cone. Use dot product test: (v dot dir) / (|v||dir|) >= cos(angle)
            if (playerLightEnabled && directionalLight && (dirX != 0.0f || dirY != 0.0f)) {
                // Compute v normalized on-the-fly with division by dist.
                if (dist > 0.001f) {
                    const float invDist = 1.0f / dist;
                    const float vx = dx * invDist;
                    const float vy = dy * invDist;
                    const float dot = vx * dirX + vy * dirY;

                    // If the dot product is less than coneCos we are outside
                    // the cone; reduce litAmount accordingly.
                    if (dot < coneCos) {
                        litAmount = 0.0f;
                    }
                }
            }
        } else if (dist < effectiveRadius + fadeMargin) {
            // Smooth falloff outside radius
            float fall = 1.0f - ((dist - effectiveRadius) / fadeMargin);
            litAmount = std::max(0.0f, fall);
        }

        // Small proximity boost for very near entities (matching previous
        // behaviour). This helps make nearby ghosts slightly more visible.
        if (dist <= GHOST_NEAR_DISTANCE) {
            litAmount = std::min(1.0f, litAmount + 0.25f);
        }

        const float targetOpacity = GHOST_MIN_OPACITY + litAmount * (1.0f - GHOST_MIN_OPACITY);

        // Smoothly interpolate toward the target opacity to avoid popping.
        vis.opacity += (targetOpacity - vis.opacity) * std::min(1.0f, smoothSpeed);
        vis.isVisible = true;
    }

    // Build the shadow mask texture for Student 1 to composite on top of the
    // scene. The texture is solid black with transparent cutouts for lights.
    if (gLightingRenderer != nullptr) {
        clearShadowMask();

        if (gShadowMaskTexture != nullptr) {
            // First add the player's light cutout.
            const float pxCenter = px;
            const float pyCenter = py;
            const float playerConeWidth = hasLightComp ? pLight->baseRadius * 0.85f : 120.0f;
            if (playerLightEnabled) {
                cutoutLight(
                    gLightingRenderer,
                    pxCenter,
                    pyCenter,
                    effectiveRadius,
                    directionalLight,
                    pdir.current,
                    coneLength,
                    playerConeWidth
                );
            }

            // Any other entities with LightComponent get their own cutout.
            static const bagel::Mask lightMask = bagel::MaskBuilder()
                .set<PositionComponent>()
                .set<LightComponent>()
                .build();
            static int lightQ = bagel::World::createQuery(lightMask);

            for (bagel::Entity e = bagel::World::first(lightQ); !bagel::World::eof(lightQ); e = bagel::World::next(lightQ)) {
                if (e.entity().id == player.entity().id) {
                    continue;
                }

                const auto& pos = e.get<PositionComponent>();
                const auto& light = e.get<LightComponent>();

                bool enabled = light.enabled;
                if (e.has<FlashlightComponent>()) {
                    enabled = enabled && e.get<FlashlightComponent>().isOn;
                }
                if (e.has<BatteryLifeComponent>()) {
                    enabled = enabled && e.get<BatteryLifeComponent>().current > 0.0f;
                }

                if (!enabled) {
                    continue;
                }

                const float centerX = pos.x + TILE_SIZE * 0.5f;
                const float centerY = pos.y + TILE_SIZE * 0.5f;
                const float radius = light.currentRadius > 0.0f ? light.currentRadius : light.baseRadius;
                const bool isDirectional = light.directional;
                const float otherConeWidth = light.baseRadius * 0.85f;
                Direction dir = Direction::None;
                if (e.has<DirectionComponent>()) {
                    dir = e.get<DirectionComponent>().current;
                }

                cutoutLight(
                    gLightingRenderer,
                    centerX,
                    centerY,
                    radius,
                    isDirectional,
                    dir,
                    light.coneLength,
                    otherConeWidth
                );
            }

            SDL_SetRenderTarget(gLightingRenderer, nullptr);
        }
    }
}



