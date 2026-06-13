#pragma once

#include <SDL3/SDL.h>
#include "bagel.h"
#include "Components.h"

class RenderSystem {
public:
    void render(
        SDL_Renderer* renderer,
        VisionMode visionMode,
        SDL_Texture* gameOverTexture,
        SDL_Texture* victoryTexture
    );
    void drawStatus(SDL_Window* window,VisionMode visionMode);
    static GhostState ghostState;

private:
    void drawMaze(SDL_Renderer* renderer);
    void drawDots(SDL_Renderer* renderer);
    void drawPacman(
        SDL_Renderer* renderer,
        const PositionComponent& position,
        const DrawingComponent& drawing,
        const DirectionComponent& direction
    );

    void drawFlashlight(
        SDL_Renderer* renderer,
        const PositionComponent& position,
        const DrawingComponent& drawing,
        const DirectionComponent& direction
    );

    void drawGhost(
        SDL_Renderer* renderer,
        float x,
        float y,
        Uint8 r,
        Uint8 g,
        Uint8 b,
        Uint8 a = 255
    );

    void drawDarkOverlay(SDL_Renderer* renderer, Uint8 alpha);

    void drawLightAroundPacman(
        SDL_Renderer* renderer,
        const PositionComponent& position,
        const DrawingComponent& drawing,
        float radius
    );

    void drawFilledRect(
        SDL_Renderer* renderer,
        float x,
        float y,
        float w,
        float h,
        Uint8 r,
        Uint8 g,
        Uint8 b,
        Uint8 a
    );

    void drawGlowRect(
        SDL_Renderer* renderer,
        float x,
        float y,
        float w,
        float h
    );

    void drawFilledCircle(
        SDL_Renderer* renderer,
        float centerX,
        float centerY,
        float radius,
        Uint8 r,
        Uint8 g,
        Uint8 b,
        Uint8 a
    );
};