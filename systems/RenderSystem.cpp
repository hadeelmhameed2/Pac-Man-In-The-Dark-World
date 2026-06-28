#include "RenderSystem.h"
#include "Constants.h"
#include "Maze.h"
#include "LightingDebugSystem.h"
#include <vector>
#include <string>
#include <algorithm>


#include <cmath>
#include <iostream>

#include "LightingDebugSystem.h"

namespace {
    float sign(float px, float py, float ax, float ay, float bx, float by) {
        return (px - bx) * (ay - by) - (ax - bx) * (py - by);
    }

    bool pointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy) {
        float d1 = sign(px, py, ax, ay, bx, by);
        float d2 = sign(px, py, bx, by, cx, cy);
        float d3 = sign(px, py, cx, cy, ax, ay);

        bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(has_neg && has_pos);
    }

    bool isGhostLit(
        float gx, float gy,
        float px, float py,
        Direction pacmanDir,
        bool flashlightOn,
        VisionMode visionMode,
        float lightRadius
    ) {
        if (visionMode == VisionMode::Full) {
            return true;
        }

        float gx_center = gx + 16.0f;
        float gy_center = gy + 16.0f;
        float px_center = px + 16.0f;
        float py_center = py + 16.0f;

        float dx = gx_center - px_center;
        float dy = gy_center - py_center;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (visionMode == VisionMode::MediumDark) {
            return dist <= lightRadius + 20.0f;
        }

        if (visionMode == VisionMode::FlashlightOnly) {
            if (dist <= 40.0f) {
                return true;
            }

            if (flashlightOn) {
                float length = 320.0f;
                float width = 120.0f;

                float x1 = px_center; float y1 = py_center;
                float x2 = px_center; float y2 = py_center;
                float x3 = px_center; float y3 = py_center;

                if (pacmanDir == Direction::Right) {
                    x2 = px_center + length; y2 = py_center - width * 0.5f;
                    x3 = px_center + length; y3 = py_center + width * 0.5f;
                }
                else if (pacmanDir == Direction::Left) {
                    x2 = px_center - length; y2 = py_center - width * 0.5f;
                    x3 = px_center - length; y3 = py_center + width * 0.5f;
                }
                else if (pacmanDir == Direction::Up) {
                    x2 = px_center - width * 0.5f; y2 = py_center - length;
                    x3 = px_center + width * 0.5f; y3 = py_center - length;
                }
                else if (pacmanDir == Direction::Down) {
                    x2 = px_center - width * 0.5f; y2 = py_center + length;
                    x3 = px_center + width * 0.5f; y3 = py_center + length;
                }

                return pointInTriangle(gx_center, gy_center, x1, y1, x2, y2, x3, y3);
            }
        }

        return false;
    }

    float ghostGlowFactor(
    float gx, float gy,
    float px, float py,
    Direction pacmanDir,
    bool flashlightOn,
    VisionMode visionMode,
    float lightRadius
) {
    if (visionMode == VisionMode::Full) {
        return 0.0f;
    }

    float gx_center = gx + 16.0f;
    float gy_center = gy + 16.0f;
    float px_center = px + 16.0f;
    float py_center = py + 16.0f;

    float dx = gx_center - px_center;
    float dy = gy_center - py_center;

    float dist = std::sqrt(dx * dx + dy * dy);

    // fully illuminated -> no glowing eyes
    if (dist <= lightRadius) {
        return 0.0f;
    }

    // MediumDark mode
    if (visionMode == VisionMode::MediumDark) {

        float maxDist = lightRadius + 250.0f;

        if (dist >= maxDist)
            return 0.2f;      // weak glow far away

        float t =
            (dist - lightRadius) /
            (maxDist - lightRadius);

        return std::clamp(t, 0.0f, 1.0f);
    }

    // Flashlight mode
    if (visionMode == VisionMode::FlashlightOnly) {

        bool illuminated = false;

        if (dist <= 40.0f) {
            illuminated = true;
        }

        if (!illuminated && flashlightOn) {

            float length = 320.0f;
            float width  = 120.0f;

            float x1 = px_center;
            float y1 = py_center;

            float x2 = px_center;
            float y2 = py_center;

            float x3 = px_center;
            float y3 = py_center;

            if (pacmanDir == Direction::Right) {
                x2 = px_center + length;
                y2 = py_center - width * 0.5f;

                x3 = px_center + length;
                y3 = py_center + width * 0.5f;
            }
            else if (pacmanDir == Direction::Left) {
                x2 = px_center - length;
                y2 = py_center - width * 0.5f;

                x3 = px_center - length;
                y3 = py_center + width * 0.5f;
            }
            else if (pacmanDir == Direction::Up) {
                x2 = px_center - width * 0.5f;
                y2 = py_center - length;

                x3 = px_center + width * 0.5f;
                y3 = py_center - length;
            }
            else if (pacmanDir == Direction::Down) {
                x2 = px_center - width * 0.5f;
                y2 = py_center + length;

                x3 = px_center + width * 0.5f;
                y3 = py_center + length;
            }

            illuminated =
                pointInTriangle(
                    gx_center, gy_center,
                    x1, y1,
                    x2, y2,
                    x3, y3
                );
        }

        if (illuminated)
            return 0.0f;

        float maxDist = 500.0f;

        float t =
            std::clamp(
                dist / maxDist,
                0.0f,
                1.0f
            );

        return t;
    }

    return 1.0f;
}

    void drawCircleHelper(SDL_Renderer* renderer, float centerX, float centerY, float radius, Uint8 a) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, 0, 0, 12, a);

        for (int y = static_cast<int>(-radius); y <= static_cast<int>(radius); ++y) {
            for (int x = static_cast<int>(-radius); x <= static_cast<int>(radius); ++x) {
                if (x * x + y * y <= radius * radius) {
                    SDL_RenderPoint(
                        renderer,
                        centerX + static_cast<float>(x),
                        centerY + static_cast<float>(y)
                    );
                }
            }
        }
    }

    void drawTriangleHelper(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, float x3, float y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
        SDL_Vertex vertices[3];
        vertices[0].position = { x1, y1 };
        vertices[0].color = { static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f };
        vertices[1].position = { x2, y2 };
        vertices[1].color = { static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f };
        vertices[2].position = { x3, y3 };
        vertices[2].color = { static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f };

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_RenderGeometry(renderer, nullptr, vertices, 3, nullptr, 0);
    }

    void drawGlowingEyes(
        SDL_Renderer* renderer,
        float x,
        float y,
        Uint8 r,
        Uint8 g,
        Uint8 b,
        Uint8 a
    ) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, r, g, b, a);

        for (int ey = -4; ey <= 4; ++ey) {
            for (int ex = -4; ex <= 4; ++ex) {
                if (ex * ex + ey * ey <= 16) {
                    SDL_RenderPoint(renderer, x + 9.0f + ex, y + 12.0f + ey);
                    SDL_RenderPoint(renderer, x + 20.0f + ex, y + 12.0f + ey);
                }
            }
        }

        Uint8 cr = std::min(255, r + 100);
        Uint8 cg = std::min(255, g + 100);
        Uint8 cb = std::min(255, b + 100);
        SDL_SetRenderDrawColor(renderer, cr, cg, cb, a);

        for (int ey = -2; ey <= 2; ++ey) {
            for (int ex = -2; ex <= 2; ++ex) {
                if (ex * ex + ey * ey <= 4) {
                    SDL_RenderPoint(renderer, x + 9.0f + ex, y + 12.0f + ey);
                    SDL_RenderPoint(renderer, x + 20.0f + ex, y + 12.0f + ey);
                }
            }
        }
    }
}

GhostState RenderSystem::ghostState = GhostState::SCATTER;

void RenderSystem::render(
    SDL_Renderer* renderer,
    VisionMode visionMode,
    SDL_Texture* mainMenuTexture,
    SDL_Texture* gameOverTexture,
    SDL_Texture* victoryTexture,
    SDL_Texture* shadowMask
) {
    if (renderer == nullptr) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(renderer, 0, 0, 12, 255);
    SDL_RenderClear(renderer);

    static const bagel::Mask stateMask = bagel::MaskBuilder()
        .set<GameStateComponent>()
        .build();
    static int stateQ = bagel::World::createQuery(stateMask);

    GameState state = GameState::MainMenu;
    if (!bagel::World::eof(stateQ)) {
        bagel::Entity stateEnt = bagel::World::first(stateQ);
        state = stateEnt.get<GameStateComponent>().state;
    }

    if (state == GameState::MainMenu) {
        if (mainMenuTexture != nullptr) {
            SDL_RenderTexture(renderer, mainMenuTexture, nullptr, nullptr);

            // Handle hover highlights
            float mx = 0.0f, my = 0.0f;
            SDL_GetMouseState(&mx, &my);

            float SCALE_X = static_cast<float>(WINDOW_WIDTH) / 1920.0f;
            float SCALE_Y = static_cast<float>(WINDOW_HEIGHT) / 1080.0f;

            SDL_FRect playRect = { PLAY_AGAIN_X * SCALE_X, PLAY_AGAIN_Y * SCALE_Y, PLAY_AGAIN_W * SCALE_X, PLAY_AGAIN_H * SCALE_Y };
            SDL_FRect exitRect = { QUIT_GAME_X * SCALE_X, QUIT_GAME_Y * SCALE_Y, QUIT_GAME_W * SCALE_X, QUIT_GAME_H * SCALE_Y };

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            if (mx >= playRect.x && mx <= playRect.x + playRect.w &&
                my >= playRect.y && my <= playRect.y + playRect.h) {
                drawButtonHoverEffect(renderer, playRect, SCALE_X, SCALE_Y, false);
            }
            if (mx >= exitRect.x && mx <= exitRect.x + exitRect.w &&
                my >= exitRect.y && my <= exitRect.y + exitRect.h) {
                drawButtonHoverEffect(renderer, exitRect, SCALE_X, SCALE_Y, false);
            }
        }
        SDL_RenderPresent(renderer);
        return;
    }

    drawMaze(renderer);
    drawDots(renderer);

    static const bagel::Mask pacmanMask = bagel::MaskBuilder()
        .set<InputComponent>()
        .set<PositionComponent>()
        .set<DrawingComponent>()
        .set<DirectionComponent>()
        .build();
    static int pacmanQ = bagel::World::createQuery(pacmanMask);

    float px = 388.0f;
    float py = 690.0f;
    Direction pacmanDir = Direction::Right;
    bool flashlightOn = false;
    float pacWidth = 32.0f;
    float pacHeight = 32.0f;

    if (!bagel::World::eof(pacmanQ)) {
        bagel::Entity pacmanEnt = bagel::World::first(pacmanQ);
        auto& pos = pacmanEnt.get<PositionComponent>();
        px = pos.x;
        py = pos.y;
        pacmanDir = pacmanEnt.get<DirectionComponent>().current;
        auto& drawing = pacmanEnt.get<DrawingComponent>();
        pacWidth = drawing.width;
        pacHeight = drawing.height;
        if (pacmanEnt.has<FlashlightComponent>()) {
            flashlightOn = pacmanEnt.get<FlashlightComponent>().isOn;
        }
    }


    float batteryLevel = 100.0f;
    bool isGameOver = false;
    bool isLowBattery = false;
    if (!bagel::World::eof(stateQ)) {
        bagel::Entity stateEnt = bagel::World::first(stateQ);
        auto& state = stateEnt.get<GameStateComponent>();
        batteryLevel = state.batteryLevel;
        isGameOver = state.isGameOver;
        isLowBattery = state.isLowBattery;
    }

    if (isGameOver) {
        visionMode = VisionMode::Full;
        shadowMask = nullptr;
    }

    float pulseTimer = static_cast<float>(SDL_GetTicks()) / 1000.0f;
    float lightRadius = LIGHT_RADIUS * (batteryLevel / 100.0f);
    bool isBoosted = false;
    if (!bagel::World::eof(pacmanQ)) {
        bagel::Entity pacmanEnt = bagel::World::first(pacmanQ);
        if (pacmanEnt.has<BatteryLifeComponent>()) {
            const auto& battery = pacmanEnt.get<BatteryLifeComponent>();
            if (battery.mode == PowerMode::Boost) {
                isBoosted = true;
                lightRadius = LIGHT_RADIUS * 1.5f;
            }
        }
    }
    if (isGameOver) {
        lightRadius = LIGHT_RADIUS * 0.25f;
    } else if (isLowBattery && !isBoosted) {
        const float flicker = 0.85f + 0.15f * std::sin(pulseTimer * 8.0f);
        lightRadius *= flicker;
    }

    static const bagel::Mask drawMask = bagel::MaskBuilder()
        .set<PositionComponent>()
        .set<DrawingComponent>()
        .build();

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(drawMask))
            continue;

        auto& position = e.get<PositionComponent>();
        auto& drawing  = e.get<DrawingComponent>();

        if (e.has<InputComponent>())
        {
            auto& direction = e.get<DirectionComponent>();

            drawPacman(renderer, position, drawing, direction);
        }
        else if (e.has<GhostAI>())
        {
            bool lit = isGhostLit(position.x, position.y, px, py, pacmanDir, flashlightOn, visionMode, lightRadius);
            if (lit) {
                Uint8 alpha = drawing.a;
                if (e.has<VisibilityComponent>() && visionMode != VisionMode::Full) {
                    const float opacity = std::max(GHOST_MIN_OPACITY, e.get<VisibilityComponent>().opacity);
                    alpha = static_cast<Uint8>(opacity * 255.0f);
                }
                drawGhost(renderer, position.x, position.y, drawing.r, drawing.g, drawing.b, alpha);
            }
        }
        else
        {
            SDL_FRect rect = {
                position.x,
                position.y,
                static_cast<float>(drawing.width),
                static_cast<float>(drawing.height)
            };

            SDL_SetRenderDrawColor(renderer, drawing.r, drawing.g, drawing.b, drawing.a);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    if (visionMode != VisionMode::Full)
    {
        static SDL_Texture* lightMask = nullptr;
        if (lightMask == nullptr) {
            lightMask = SDL_CreateTexture(
                renderer,
                SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_TARGET,
                WINDOW_WIDTH,
                WINDOW_HEIGHT
            );
            if (lightMask != nullptr) {
                SDL_SetTextureBlendMode(lightMask, SDL_BLENDMODE_BLEND);
            }
        }

        if (lightMask != nullptr) {
            SDL_SetRenderTarget(renderer, lightMask);

            float batteryFactor = batteryLevel / 100.0f;
            Uint8 baseAlpha = 255;
            if (visionMode == VisionMode::MediumDark) {
                baseAlpha = static_cast<Uint8>(140.0f + 115.0f * (1.0f - batteryFactor));
            }

            SDL_FRect clearRect = { 0.0f, 0.0f, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT) };
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(renderer, 0, 0, 12, baseAlpha);
            SDL_RenderFillRect(renderer, &clearRect);

            float px_center = px + pacWidth * 0.5f;
            float py_center = py + pacHeight * 0.5f;

            if (visionMode == VisionMode::MediumDark) {
                Uint8 a1 = static_cast<Uint8>(180 + 75 * (1.0f - batteryFactor));
                Uint8 a2 = static_cast<Uint8>(100 + 155 * (1.0f - batteryFactor));
                Uint8 a3 = static_cast<Uint8>(255 * (1.0f - batteryFactor));
                drawCircleHelper(renderer, px_center, py_center, lightRadius, a1);
                drawCircleHelper(renderer, px_center, py_center, lightRadius * 0.85f, a2);
                drawCircleHelper(renderer, px_center, py_center, lightRadius * 0.6f, a3);
            }
            else if (visionMode == VisionMode::FlashlightOnly) {
                float innerCircleRadius = 40.0f * batteryFactor;
                Uint8 a1 = static_cast<Uint8>(150 + 105 * (1.0f - batteryFactor));
                Uint8 a2 = static_cast<Uint8>(255 * (1.0f - batteryFactor));
                drawCircleHelper(renderer, px_center, py_center, innerCircleRadius, a1);
                drawCircleHelper(renderer, px_center, py_center, innerCircleRadius * 0.5f, a2);

                if (flashlightOn) {
                    float length = 320.0f * batteryFactor;
                    float width = 120.0f * batteryFactor;

                    float x1 = px_center; float y1 = py_center;
                    float x2 = px_center; float y2 = py_center;
                    float x3 = px_center; float y3 = py_center;

                    float x2_wide = px_center; float y2_wide = py_center;
                    float x3_wide = px_center; float y3_wide = py_center;

                    if (pacmanDir == Direction::Right) {
                        x2 = px_center + length; y2 = py_center - width * 0.5f;
                        x3 = px_center + length; y3 = py_center + width * 0.5f;
                        x2_wide = px_center + length + 20.0f * batteryFactor; y2_wide = py_center - width * 0.65f;
                        x3_wide = px_center + length + 20.0f * batteryFactor; y3_wide = py_center + width * 0.65f;
                    }
                    else if (pacmanDir == Direction::Left) {
                        x2 = px_center - length; y2 = py_center - width * 0.5f;
                        x3 = px_center - length; y3 = py_center + width * 0.5f;
                        x2_wide = px_center - length - 20.0f * batteryFactor; y2_wide = py_center - width * 0.65f;
                        x3_wide = px_center - length - 20.0f * batteryFactor; y3_wide = py_center + width * 0.65f;
                    }
                    else if (pacmanDir == Direction::Up) {
                        x2 = px_center - width * 0.5f; y2 = py_center - length;
                        x3 = px_center + width * 0.5f; y3 = py_center - length;
                        x2_wide = px_center - width * 0.65f; y2_wide = py_center - length - 20.0f * batteryFactor;
                        x3_wide = px_center + width * 0.65f; y3_wide = py_center - length - 20.0f * batteryFactor;
                    }
                    else if (pacmanDir == Direction::Down) {
                        x2 = px_center - width * 0.5f; y2 = py_center + length;
                        x3 = px_center + width * 0.5f; y3 = py_center + length;
                        x2_wide = px_center - width * 0.65f; y2_wide = py_center + length + 20.0f * batteryFactor;
                        x3_wide = px_center + width * 0.65f; y3_wide = py_center + length + 20.0f * batteryFactor;
                    }

                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                    drawTriangleHelper(renderer, x1, y1, x2_wide, y2_wide, x3_wide, y3_wide, 0, 0, 0, a1);
                    drawTriangleHelper(renderer, x1, y1, x2, y2, x3, y3, 0, 0, 0, a2);
                }
            }

            SDL_SetRenderTarget(renderer, nullptr);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_RenderTexture(renderer, lightMask, nullptr, nullptr);
        }
    }

    if (visionMode != VisionMode::Full)
    {
        for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
        {
            if (!e.test(drawMask) || !e.has<GhostAI>())
                continue;

            auto& position = e.get<PositionComponent>();
            auto& drawing  = e.get<DrawingComponent>();

            float glow =
            ghostGlowFactor(position.x,position.y,px,py,pacmanDir,flashlightOn,visionMode,lightRadius);
            if (glow > 0.01f) {
               Uint8 alpha = static_cast<Uint8>(255.0f * glow);
                drawGlowingEyes(
                    renderer,
                    position.x,
                    position.y,
                    drawing.r,
                    drawing.g,
                    drawing.b,
                    alpha
                );
            }
        }
    }

    // Draw Custom Image End Screens if Game Over
    if (isGameOver) {
        bool isVictory = false;
        if (!bagel::World::eof(stateQ)) {
            bagel::Entity stateEnt = bagel::World::first(stateQ);
            isVictory = stateEnt.get<GameStateComponent>().shownVictoryPopup;
        }

        SDL_Texture* activeTexture = isVictory ? victoryTexture : gameOverTexture;
        if (activeTexture != nullptr) {
            SDL_RenderTexture(renderer, activeTexture, nullptr, nullptr);

            // Handle hover highlights
            float mx = 0.0f, my = 0.0f;
            SDL_GetMouseState(&mx, &my);

            float SCALE_X = static_cast<float>(WINDOW_WIDTH) / 1920.0f;
            float SCALE_Y = static_cast<float>(WINDOW_HEIGHT) / 1080.0f;

            SDL_FRect newGameRect = { PLAY_AGAIN_X * SCALE_X, PLAY_AGAIN_Y * SCALE_Y, PLAY_AGAIN_W * SCALE_X, PLAY_AGAIN_H * SCALE_Y };
            SDL_FRect exitRect = { QUIT_GAME_X * SCALE_X, QUIT_GAME_Y * SCALE_Y, QUIT_GAME_W * SCALE_X, QUIT_GAME_H * SCALE_Y };

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            if (mx >= newGameRect.x && mx <= newGameRect.x + newGameRect.w &&
                my >= newGameRect.y && my <= newGameRect.y + newGameRect.h) {
                drawButtonHoverEffect(renderer, newGameRect, SCALE_X, SCALE_Y, isVictory);
            }
            if (mx >= exitRect.x && mx <= exitRect.x + exitRect.w &&
                my >= exitRect.y && my <= exitRect.y + exitRect.h) {
                drawButtonHoverEffect(renderer, exitRect, SCALE_X, SCALE_Y, isVictory);
            }
        }
    }

    // Composite the shadow mask from LightingSystem over the rendered scene
    if (shadowMask != nullptr) {
        SDL_RenderTexture(renderer, shadowMask, nullptr, nullptr);
    }

    // Render the debug overlay only while active gameplay is running.
    if (!bagel::World::eof(stateQ) && !isGameOver) {
        LightingDebugSystem::renderOverlay(renderer);
    }

    SDL_RenderPresent(renderer);
}

void RenderSystem::drawMaze(SDL_Renderer* renderer) {
    for (int row = 0; row < MAZE_ROWS; ++row) {
        for (int col = 0; col < MAZE_COLS; ++col) {
            char cell = MAZE_LAYOUT[row][col];

            float x = MAZE_START_X + col * MAZE_TILE_SIZE;
            float y = MAZE_START_Y + row * MAZE_TILE_SIZE;

            if (cell == '#') {
                drawGlowRect(renderer, x, y, MAZE_TILE_SIZE, MAZE_TILE_SIZE);
            }
        }
    }
}

void RenderSystem::drawDots(SDL_Renderer* renderer) {
    for (int row = 0; row < MAZE_ROWS; ++row) {
        for (int col = 0; col < MAZE_COLS; ++col) {
            char cell = MAZE_LAYOUT[row][col];

            float centerX = MAZE_START_X + col * MAZE_TILE_SIZE + MAZE_TILE_SIZE / 2.0f;
            float centerY = MAZE_START_Y + row * MAZE_TILE_SIZE + MAZE_TILE_SIZE / 2.0f;

            if (cell == '.') {
                drawFilledCircle(renderer, centerX, centerY, 2.5f, 235, 215, 185, 255);
            }
            else if (cell == 'o') {
                drawFilledCircle(renderer, centerX, centerY, 6.0f, 255, 220, 190, 255);
            }
            else if (cell=='F')
            {
                drawFilledRect(
                    renderer,
                    centerX - 6.0f,
                    centerY - 3.0f,
                    10.0f,
                    6.0f,
                    180, 180, 180, 255
                );

                drawFilledRect(
                    renderer,
                    centerX + 4.0f,
                    centerY - 4.0f,
                    4.0f,
                    8.0f,
                    220, 220, 220, 255
                );

                drawFilledCircle(
                    renderer,
                    centerX - 6.0f,
                    centerY,
                    3.0f,
                    120, 120, 120, 255
                );

                SDL_SetRenderDrawColor(
                    renderer,
                    255, 255, 180, 180
                );

                SDL_RenderLine(
                    renderer,
                    centerX + 8.0f,
                    centerY,
                    centerX + 14.0f,
                    centerY - 4.0f
                );

                SDL_RenderLine(
                    renderer,
                    centerX + 8.0f,
                    centerY,
                    centerX + 14.0f,
                    centerY + 4.0f
                );

                SDL_RenderLine(
                    renderer,
                    centerX + 8.0f,
                    centerY,
                    centerX + 16.0f,
                    centerY
                );
            }
        }
    }
}

void RenderSystem::drawGhost(
    SDL_Renderer* renderer,
    float x,
    float y,
    Uint8 r,
    Uint8 g,
    Uint8 b,
    Uint8 a
) {
    drawFilledCircle(renderer, x + 14, y + 12, 14, r, g, b, a);
    drawFilledRect(renderer, x, y + 12, 28, 18, r, g, b, a);

    drawFilledCircle(renderer, x + 5, y + 30, 5, r, g, b, a);
    drawFilledCircle(renderer, x + 14, y + 30, 5, r, g, b, a);
    drawFilledCircle(renderer, x + 23, y + 30, 5, r, g, b, a);

    drawFilledCircle(renderer, x + 9, y + 12, 4, 255, 255, 255, a);
    drawFilledCircle(renderer, x + 20, y + 12, 4, 255, 255, 255, a);

    drawFilledCircle(renderer, x + 10, y + 12, 2, 0, 0, 40, a);
    drawFilledCircle(renderer, x + 21, y + 12, 2, 0, 0, 40, a);
}

void RenderSystem::drawStatus(SDL_Window *window,VisionMode visionMode) {
    static const bagel::Mask mask = bagel::MaskBuilder()
            .set<BatteryLifeComponent>()
            .build();
    static const bagel::Mask stateMask = bagel::MaskBuilder()
            .set<GameStateComponent>()
            .build();
    static int stateQ = bagel::World::createQuery(stateMask);

    int score = 0;
    if (!bagel::World::eof(stateQ)) {
        bagel::Entity stateEnt = bagel::World::first(stateQ);
        score = stateEnt.get<GameStateComponent>().score;
    }

    for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next()) {
        if (e.test(mask)) {
            std::string modeText;

            if (visionMode == VisionMode::Full) {
                modeText = "Full Vision";
            }
            else if (visionMode == VisionMode::MediumDark) {
                modeText = "Medium Darkness";
            }
            else {
                modeText = "Flashlight Only";
            }

            std::string title =
                "Pacman in the Dark World | Battery: " +
                std::to_string(static_cast<int>(e.get<BatteryLifeComponent>().current)) +
                "% | Score: " +
                std::to_string(score) +
                " | Mode: " +
                modeText;

            SDL_SetWindowTitle(window, title.c_str());
            return;
        }
    }
}

void RenderSystem::drawPacman(
    SDL_Renderer* renderer,
    const PositionComponent& position,
    const DrawingComponent& drawing,
    const DirectionComponent& direction
) {
    float centerX = position.x + static_cast<float>(drawing.width) / 2.0f;
    float centerY = position.y + static_cast<float>(drawing.height) / 2.0f;
    float radius = static_cast<float>(drawing.width) / 2.0f;

    if (ghostState == GhostState::FRIGHTENED)
        radius += 3.0;

    drawFilledCircle(renderer, centerX, centerY, radius, 255, 255, 0, 255);

    SDL_SetRenderDrawColor(renderer, 0, 0, 12, 255);

    if (direction.current == Direction::Right) {
        for (int offset = -9; offset <= 9; ++offset) {
            SDL_RenderLine(renderer, centerX, centerY, centerX + radius, centerY + static_cast<float>(offset));
        }
    }
    else if (direction.current == Direction::Left) {
        for (int offset = -9; offset <= 9; ++offset) {
            SDL_RenderLine(renderer, centerX, centerY, centerX - radius, centerY + static_cast<float>(offset));
        }
    }
    else if (direction.current == Direction::Up) {
        for (int offset = -9; offset <= 9; ++offset) {
            SDL_RenderLine(renderer, centerX, centerY, centerX + static_cast<float>(offset), centerY - radius);
        }
    }
    else if (direction.current == Direction::Down) {
        for (int offset = -9; offset <= 9; ++offset) {
            SDL_RenderLine(renderer, centerX, centerY, centerX + static_cast<float>(offset), centerY + radius);
        }
    }
}

void RenderSystem::drawFlashlight(
    SDL_Renderer* renderer,
    const PositionComponent& position,
    const DrawingComponent& drawing,
    const DirectionComponent& direction
) {
    SDL_FRect lightRect;

    float pacmanCenterX = position.x + static_cast<float>(drawing.width) / 2.0f;
    float pacmanCenterY = position.y + static_cast<float>(drawing.height) / 2.0f;

    float lightLength = 170.0f;
    float lightWidth = 26.0f;

    if (direction.current == Direction::Right) {
        lightRect = {
            position.x + static_cast<float>(drawing.width),
            pacmanCenterY - lightWidth / 2.0f,
            lightLength,
            lightWidth
        };
    }
    else if (direction.current == Direction::Left) {
        lightRect = {
            position.x - lightLength,
            pacmanCenterY - lightWidth / 2.0f,
            lightLength,
            lightWidth
        };
    }
    else if (direction.current == Direction::Up) {
        lightRect = {
            pacmanCenterX - lightWidth / 2.0f,
            position.y - lightLength,
            lightWidth,
            lightLength
        };
    }
    else if (direction.current == Direction::Down) {
        lightRect = {
            pacmanCenterX - lightWidth / 2.0f,
            position.y + static_cast<float>(drawing.height),
            lightWidth,
            lightLength
        };
    }
    else {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 120, 90);
    SDL_RenderFillRect(renderer, &lightRect);
}

void RenderSystem::drawDarkOverlay(SDL_Renderer* renderer, Uint8 alpha) {
    SDL_FRect overlay = {
        0.0f,
        0.0f,
        850.0f,
        850.0f
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
    SDL_RenderFillRect(renderer, &overlay);
}

void RenderSystem::drawLightAroundPacman(
    SDL_Renderer* renderer,
    const PositionComponent& position,
    const DrawingComponent& drawing,
    float radius
) {
    float centerX = position.x + static_cast<float>(drawing.width) / 2.0f;
    float centerY = position.y + static_cast<float>(drawing.height) / 2.0f;

    drawFilledCircle(renderer, centerX, centerY, radius, 255, 255, 170, 55);
    drawFilledCircle(renderer, centerX, centerY, radius * 0.65f, 255, 255, 160, 65);
    drawFilledCircle(renderer, centerX, centerY, radius * 0.35f, 255, 255, 130, 75);
}

void RenderSystem::drawGlowRect(
    SDL_Renderer* renderer,
    float x,
    float y,
    float w,
    float h
) {
    drawFilledRect(renderer, x - 2, y - 2, w + 4, h + 4, 0, 70, 255, 55);
    drawFilledRect(renderer, x, y, w, h, 0, 115, 255, 255);

    if (w > 6 && h > 6) {
        drawFilledRect(renderer, x + 3, y + 3, w - 6, h - 6, 0, 170, 255, 160);
    }
}

void RenderSystem::drawFilledRect(
    SDL_Renderer* renderer,
    float x,
    float y,
    float w,
    float h,
    Uint8 r,
    Uint8 g,
    Uint8 b,
    Uint8 a
) {
    SDL_FRect rect = {x, y, w, h};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
}

void RenderSystem::drawFilledCircle(
    SDL_Renderer* renderer,
    float centerX,
    float centerY,
    float radius,
    Uint8 r,
    Uint8 g,
    Uint8 b,
    Uint8 a
) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    for (int y = static_cast<int>(-radius); y <= static_cast<int>(radius); ++y) {
        for (int x = static_cast<int>(-radius); x <= static_cast<int>(radius); ++x) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderPoint(
                    renderer,
                    centerX + static_cast<float>(x),
                    centerY + static_cast<float>(y)
                );
            }
        }
    }
}

void RenderSystem::drawButtonHoverEffect(
    SDL_Renderer* renderer,
    const SDL_FRect& buttonRect,
    float SCALE_X,
    float SCALE_Y,
    bool isVictory
) {
    // Nudge flashY slightly downward (by 5% of button height) to center perfectly with the button's visual text area
    float flashY = buttonRect.y + buttonRect.h * 0.5f + buttonRect.h * 0.05f;
    float buttonW = buttonRect.w;
    float buttonH = buttonRect.h;

    // Scale flashlight dimensions dynamically based on button height and width
    float headH = buttonH * 0.35f;
    float lensH = headH * 0.85f;
    float bodyH = headH * 0.4f;
    float switchH = bodyH * 0.2f;

    float bodyW = buttonW * 0.08f;
    float headW = buttonW * 0.02f;
    float lensW = buttonW * 0.01f;
    float switchW = bodyW * 0.2f;

    // Position of left flashlight lens (left body starts inset by 2% of button width)
    float leftBodyX = buttonRect.x + buttonW * 0.02f;
    float lensCenterX_left = leftBodyX + bodyW + headW;

    // Position of right flashlight lens (right body ends inset by 2% of button width)
    float rightBodyX = buttonRect.x + buttonW * 0.98f - bodyW;
    float lensCenterX_right = rightBodyX - headW - lensW;

    // 1. Draw Left Flashlight
    drawSingleFlashlight(renderer, lensCenterX_left, flashY, bodyW, bodyH, headW, headH, lensW, lensH, switchW, switchH, true);

    // 2. Draw Right Flashlight
    drawSingleFlashlight(renderer, lensCenterX_right, flashY, bodyW, bodyH, headW, headH, lensW, lensH, switchW, switchH, false);

    // 3. Draw Left Flashlight Light Beam
    float beamStartX_left = lensCenterX_left + lensW;
    float beamLength = (buttonRect.x + buttonW * 0.5f) - beamStartX_left;
    float beamSpread = buttonH * 0.45f;

    SDL_Vertex verticesL[3];
    verticesL[0].position = { beamStartX_left, flashY };
    verticesL[0].color = { 255.0f/255.0f, 255.0f/255.0f, 220.0f/255.0f, 150.0f/255.0f }; // soft yellow glow at source

    verticesL[1].position = { beamStartX_left + beamLength, flashY - beamSpread };
    verticesL[1].color = { 255.0f/255.0f, 255.0f/255.0f, 220.0f/255.0f, 10.0f/255.0f }; // fade out

    verticesL[2].position = { beamStartX_left + beamLength, flashY + beamSpread };
    verticesL[2].color = { 255.0f/255.0f, 255.0f/255.0f, 220.0f/255.0f, 10.0f/255.0f }; // fade out

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderGeometry(renderer, nullptr, verticesL, 3, nullptr, 0);

    // 4. Draw Right Flashlight Light Beam
    float beamStartX_right = lensCenterX_right;
    float beamLength_right = beamStartX_right - (buttonRect.x + buttonW * 0.5f);

    SDL_Vertex verticesR[3];
    verticesR[0].position = { beamStartX_right, flashY };
    verticesR[0].color = { 255.0f/255.0f, 255.0f/255.0f, 220.0f/255.0f, 150.0f/255.0f };

    verticesR[1].position = { beamStartX_right - beamLength_right, flashY - beamSpread };
    verticesR[1].color = { 255.0f/255.0f, 255.0f/255.0f, 220.0f/255.0f, 10.0f/255.0f };

    verticesR[2].position = { beamStartX_right - beamLength_right, flashY + beamSpread };
    verticesR[2].color = { 255.0f/255.0f, 255.0f/255.0f, 220.0f/255.0f, 10.0f/255.0f };

    SDL_RenderGeometry(renderer, nullptr, verticesR, 3, nullptr, 0);
}

void RenderSystem::drawSingleFlashlight(
    SDL_Renderer* renderer,
    float lensX,
    float flashY,
    float bodyW,
    float bodyH,
    float headW,
    float headH,
    float lensW,
    float lensH,
    float switchW,
    float switchH,
    bool pointsRight
) {
    float headX = 0.0f;
    float bodyX = 0.0f;
    float switchX = 0.0f;

    if (pointsRight) {
        headX = lensX - headW;
        bodyX = lensX - headW - bodyW;
        switchX = bodyX + bodyW * 0.4f;
    } else {
        headX = lensX + lensW;
        bodyX = lensX + lensW + headW;
        switchX = bodyX + bodyW * 0.4f;
    }

    // Draw Switch (Red)
    SDL_FRect switchRect = { switchX, flashY - bodyH * 0.5f - switchH, switchW, switchH };
    SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
    SDL_RenderFillRect(renderer, &switchRect);

    // Draw Body (Grey cylinder)
    SDL_FRect bodyRect = { bodyX, flashY - bodyH * 0.5f, bodyW, bodyH };
    SDL_SetRenderDrawColor(renderer, 85, 95, 105, 255);
    SDL_RenderFillRect(renderer, &bodyRect);

    // Draw red accent band on the body
    float accentW = bodyW * 0.1f;
    float accentX = pointsRight ? (bodyX + bodyW - accentW - bodyW * 0.1f) : (bodyX + bodyW * 0.1f);
    SDL_FRect accentRect = { accentX, flashY - bodyH * 0.5f, accentW, bodyH };
    SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
    SDL_RenderFillRect(renderer, &accentRect);

    // Draw Bezel/Head (Dark grey flared)
    float headInnerW = headW * 0.4f;
    float headInnerH = bodyH + (headH - bodyH) * 0.4f;
    float headInnerX = pointsRight ? headX : (headX + headW - headInnerW);
    SDL_FRect headInnerRect = { headInnerX, flashY - headInnerH * 0.5f, headInnerW, headInnerH };
    SDL_SetRenderDrawColor(renderer, 60, 65, 75, 255);
    SDL_RenderFillRect(renderer, &headInnerRect);

    float headOuterW = headW * 0.6f;
    float headOuterH = headH;
    float headOuterX = pointsRight ? (headX + headInnerW) : headX;
    SDL_FRect headOuterRect = { headOuterX, flashY - headOuterH * 0.5f, headOuterW, headOuterH };
    SDL_SetRenderDrawColor(renderer, 50, 55, 65, 255);
    SDL_RenderFillRect(renderer, &headOuterRect);

    // Draw Lens (Bright yellow/white)
    SDL_FRect lensRect = { lensX, flashY - lensH * 0.5f, lensW, lensH };
    SDL_SetRenderDrawColor(renderer, 255, 255, 210, 255);
    SDL_RenderFillRect(renderer, &lensRect);
}