#include "RenderSystem.h"
#include "Constants.h"
#include "Maze.h"
#include <vector>
#include <string>
#include <algorithm>


#include <cmath>

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

void RenderSystem::render(
    SDL_Renderer* renderer,
    VisionMode visionMode
) {
    if (renderer == nullptr) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(renderer, 0, 0, 12, 255);
    SDL_RenderClear(renderer);

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

    static const bagel::Mask stateMask = bagel::MaskBuilder()
        .set<GameStateComponent>()
        .build();
    static int stateQ = bagel::World::createQuery(stateMask);

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

    float pulseTimer = static_cast<float>(SDL_GetTicks()) / 1000.0f;
    float lightRadius = LIGHT_RADIUS * (batteryLevel / 100.0f);
    if (isGameOver) {
        lightRadius = LIGHT_RADIUS * 0.25f;
    } else if (isLowBattery) {
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

            bool lit = isGhostLit(position.x, position.y, px, py, pacmanDir, flashlightOn, visionMode, lightRadius);
            if (!lit) {
                drawGlowingEyes(renderer, position.x, position.y, drawing.r, drawing.g, drawing.b, 255);
            }
        }
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