#include "RenderSystem.h"
#include <vector>
#include <string>


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
    drawGhosts(renderer);


    static const bagel::Mask drawMask =
    bagel::MaskBuilder()
        .set<PositionComponent>()
        .set<DrawingComponent>()
        .build();

    static const bagel::Mask pacmanMask =
    bagel::MaskBuilder()
        .set<InputComponent>()
        .set<PositionComponent>()
        .set<DrawingComponent>()
        .set<DirectionComponent>()
        .build();

// Draw entities
for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
{
    if (!e.test(drawMask))
        continue;

    auto& position = e.get<PositionComponent>();
    auto& drawing  = e.get<DrawingComponent>();

    if (e.has<InputComponent>()) //player
    {
        auto& direction = e.get<DirectionComponent>();
        drawPacman(renderer,position,drawing,direction);

        if (e.has<FlashlightComponent>())
        {
            auto& flashlight = e.get<FlashlightComponent>();

            if (flashlight.isOn &&
                visionMode != VisionMode::Full)
            {
                drawFlashlight(
                    renderer,
                    position,
                    drawing,
                    direction
                );
            }
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

        SDL_SetRenderDrawColor(
            renderer,
            drawing.r,
            drawing.g,
            drawing.b,
            drawing.a
        );

        SDL_RenderFillRect(renderer, &rect);
    }
}

// Darkness / flashlight effects
for (bagel::Entity e = bagel::Entity::first(); !e.eof(); e.next())
{
    if (!e.test(pacmanMask))
        continue;

    auto& position  = e.get<PositionComponent>();
    auto& drawing   = e.get<DrawingComponent>();
    auto& direction = e.get<DirectionComponent>();

    if (visionMode == VisionMode::MediumDark)
    {
        drawDarkOverlay(renderer, 120);

        drawLightAroundPacman(
            renderer,
            position,
            drawing,
            120.0f
        );
    }
    else if (visionMode == VisionMode::FlashlightOnly)
    {
        drawDarkOverlay(renderer, 225);

        drawLightAroundPacman(
            renderer,
            position,
            drawing,
            35.0f
        );

        if (e.has<FlashlightComponent>() &&
            e.get<FlashlightComponent>().isOn)
        {
            drawFlashlight(
                renderer,
                position,
                drawing,
                direction
            );
        }
    }

    break; // only one Pacman
}

SDL_RenderPresent(renderer);
}

void RenderSystem::drawMaze(SDL_Renderer* renderer) {
    const std::vector<std::string> maze = {
        "############################",
        "#............##............#",
        "#.####.#####.##.#####.####.#",
        "#o####.#####.##.#####.####o#",
        "#.####.#####.##.#####.####.#",
        "#..........................#",
        "#.####.##.########.##.####.#",
        "#.####.##.########.##.####.#",
        "#......##....##....##......#",
        "######.##### ## #####.######",
        "     #.##### ## #####.#     ",
        "     #.##          ##.#     ",
        "     #.## ###--### ##.#     ",
        "######.## #      # ##.######",
        "      .   #      #   .      ",
        "######.## #      # ##.######",
        "     #.## ######## ##.#     ",
        "     #.##          ##.#     ",
        "     #.## ######## ##.#     ",
        "######.## ######## ##.######",
        "#............##............#",
        "#.####.#####.##.#####.####.#",
        "#o..##................##..o#",
        "###.##.##.########.##.##.###",
        "###.##.##.########.##.##.###",
        "#......##....##....##......#",
        "#.##########.##.##########.#",
        "#.##########.##.##########.#",
        "#..........................#",
        "############################"
    };

    const float tile = 24.0f;
    const float startX = 70.0f;
    const float startY = 55.0f;

    for (int row = 0; row < static_cast<int>(maze.size()); ++row) {
        for (int col = 0; col < static_cast<int>(maze[row].size()); ++col) {
            char cell = maze[row][col];

            float x = startX + col * tile;
            float y = startY + row * tile;

            if (cell == '#') {
                drawGlowRect(renderer, x, y, tile, tile);
            }
        }
    }
}

void RenderSystem::drawDots(SDL_Renderer* renderer) {
    const std::vector<std::string> maze = {
        "############################",
        "#............##............#",
        "#.####.#####.##.#####.####.#",
        "#o####.#####.##.#####.####o#",
        "#.####.#####.##.#####.####.#",
        "#..........................#",
        "#.####.##.########.##.####.#",
        "#.####.##.########.##.####.#",
        "#......##....##....##......#",
        "######.##### ## #####.######",
        "     #.##### ## #####.#     ",
        "     #.##          ##.#     ",
        "     #.## ###--### ##.#     ",
        "######.## #      # ##.######",
        "      .   #      #   .      ",
        "######.## #      # ##.######",
        "     #.## ######## ##.#     ",
        "     #.##          ##.#     ",
        "     #.## ######## ##.#     ",
        "######.## ######## ##.######",
        "#............##............#",
        "#.####.#####.##.#####.####.#",
        "#o..##................##..o#",
        "###.##.##.########.##.##.###",
        "###.##.##.########.##.##.###",
        "#......##....##....##......#",
        "#.##########.##.##########.#",
        "#.##########.##.##########.#",
        "#..........................#",
        "############################"
    };

    const float tile = 24.0f;
    const float startX = 70.0f;
    const float startY = 55.0f;

    for (int row = 0; row < static_cast<int>(maze.size()); ++row) {
        for (int col = 0; col < static_cast<int>(maze[row].size()); ++col) {
            char cell = maze[row][col];

            float centerX = startX + col * tile + tile / 2.0f;
            float centerY = startY + row * tile + tile / 2.0f;

            if (cell == '.') {
                drawFilledCircle(renderer, centerX, centerY, 2.5f, 235, 215, 185, 255);
            }
            else if (cell == 'o') {
                drawFilledCircle(renderer, centerX, centerY, 6.0f, 255, 220, 190, 255);
            }
        }
    }
}

void RenderSystem::drawGhosts(SDL_Renderer* renderer) {
    drawGhost(renderer, 396, 370, 255, 0, 0);
    drawGhost(renderer, 432, 370, 255, 120, 200);
    drawGhost(renderer, 468, 370, 0, 180, 255);
    drawGhost(renderer, 360, 370, 255, 150, 0);
}

void RenderSystem::drawGhost(
    SDL_Renderer* renderer,
    float x,
    float y,
    Uint8 r,
    Uint8 g,
    Uint8 b
) {
    drawFilledCircle(renderer, x + 14, y + 12, 14, r, g, b, 255);
    drawFilledRect(renderer, x, y + 12, 28, 18, r, g, b, 255);

    drawFilledCircle(renderer, x + 5, y + 30, 5, r, g, b, 255);
    drawFilledCircle(renderer, x + 14, y + 30, 5, r, g, b, 255);
    drawFilledCircle(renderer, x + 23, y + 30, 5, r, g, b, 255);

    drawFilledCircle(renderer, x + 9, y + 12, 4, 255, 255, 255, 255);
    drawFilledCircle(renderer, x + 20, y + 12, 4, 255, 255, 255, 255);

    drawFilledCircle(renderer, x + 10, y + 12, 2, 0, 0, 40, 255);
    drawFilledCircle(renderer, x + 21, y + 12, 2, 0, 0, 40, 255);
}

void RenderSystem::drawStatus(SDL_Window *window,VisionMode visionMode) {

    static const bagel::Mask mask = bagel::MaskBuilder()
            .set<BatteryLifeComponent>()
            .build();

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
                "% | Mode: " +
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