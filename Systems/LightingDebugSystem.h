#pragma once

#include <SDL3/SDL.h>
#include <string>
#include "LightingComponents.h"

// LightingDebugSystem owns the tiny on-screen debug overlay for the battery
// / power-mode feature set. It stays isolated from the team-owned systems and
// only needs a one-line hook from RenderSystem to draw after the scene.
class LightingDebugSystem {
public:
    // Called by BatterySystem once per update to publish the current state.
    static void setState(
        float batteryCurrent,
        float batteryMax,
        PowerMode mode,
        float boostRemaining
    );

    // Called from the end of the main render pass so the overlay is always on
    // top of the scene.
    static void renderOverlay(SDL_Renderer* renderer);

private:
    static void drawText(
        SDL_Renderer* renderer,
        int x,
        int y,
        const char* text,
        Uint8 r,
        Uint8 g,
        Uint8 b,
        Uint8 a,
        int scale = 2
    );

    static void drawGlyph(
        SDL_Renderer* renderer,
        int x,
        int y,
        char ch,
        Uint8 r,
        Uint8 g,
        Uint8 b,
        Uint8 a,
        int scale
    );

    static void drawFilledRect(
        SDL_Renderer* renderer,
        int x,
        int y,
        int w,
        int h,
        Uint8 r,
        Uint8 g,
        Uint8 b,
        Uint8 a
    );

    static std::string powerModeToString(PowerMode mode);
};
