#include "LightingDebugSystem.h"
#include "Constants.h"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdio>

namespace {
struct DebugOverlayState {
    float batteryCurrent = 100.0f;
    float batteryMax = 100.0f;
    PowerMode mode = PowerMode::Normal;
    float boostRemaining = 0.0f;
};

DebugOverlayState gState;

// 5x7 bitmap font. We keep the set intentionally small and only cover the
// characters needed for the debug overlay text.
//
// Each row is encoded as 5 bits (bit 4 = left-most pixel, bit 0 = right-most).
using GlyphRows = unsigned char[7];

constexpr GlyphRows GLYPH_SPACE = {0,0,0,0,0,0,0};
constexpr GlyphRows GLYPH_COLON  = {0,0,0,0b00100,0,0b00100,0};
constexpr GlyphRows GLYPH_DOT    = {0,0,0,0,0,0b00110,0b00110};
constexpr GlyphRows GLYPH_PERCENT= {0b11001,0b11010,0b00100,0b01000,0b10011,0b00000,0b00000};
constexpr GlyphRows GLYPH_0      = {0b01110,0b10001,0b10011,0b10101,0b11001,0b10001,0b01110};
constexpr GlyphRows GLYPH_1      = {0b00100,0b01100,0b00100,0b00100,0b00100,0b00100,0b01110};
constexpr GlyphRows GLYPH_2      = {0b01110,0b10001,0b00001,0b00010,0b00100,0b01000,0b11111};
constexpr GlyphRows GLYPH_3      = {0b11110,0b00001,0b00001,0b01110,0b00001,0b00001,0b11110};
constexpr GlyphRows GLYPH_4      = {0b00010,0b00110,0b01010,0b10010,0b11111,0b00010,0b00010};
constexpr GlyphRows GLYPH_5      = {0b11111,0b10000,0b10000,0b11110,0b00001,0b00001,0b11110};
constexpr GlyphRows GLYPH_6      = {0b01110,0b10000,0b10000,0b11110,0b10001,0b10001,0b01110};
constexpr GlyphRows GLYPH_7      = {0b11111,0b00001,0b00010,0b00100,0b01000,0b01000,0b01000};
constexpr GlyphRows GLYPH_8      = {0b01110,0b10001,0b10001,0b01110,0b10001,0b10001,0b01110};
constexpr GlyphRows GLYPH_9      = {0b01110,0b10001,0b10001,0b01111,0b00001,0b00001,0b01110};

constexpr GlyphRows GLYPH_A = {0b00100,0b01010,0b10001,0b11111,0b10001,0b10001,0b10001};
constexpr GlyphRows GLYPH_B = {0b11110,0b10001,0b10001,0b11110,0b10001,0b10001,0b11110};
constexpr GlyphRows GLYPH_D = {0b11100,0b10010,0b10001,0b10001,0b10001,0b10010,0b11100};
constexpr GlyphRows GLYPH_E = {0b11111,0b10000,0b10000,0b11110,0b10000,0b10000,0b11111};
constexpr GlyphRows GLYPH_G = {0b01110,0b10001,0b10000,0b10000,0b10011,0b10001,0b01110};
constexpr GlyphRows GLYPH_I = {0b11111,0b00100,0b00100,0b00100,0b00100,0b00100,0b11111};
constexpr GlyphRows GLYPH_L = {0b10000,0b10000,0b10000,0b10000,0b10000,0b10000,0b11111};
constexpr GlyphRows GLYPH_M = {0b10001,0b11011,0b10101,0b10101,0b10001,0b10001,0b10001};
constexpr GlyphRows GLYPH_N = {0b10001,0b11001,0b10101,0b10011,0b10001,0b10001,0b10001};
constexpr GlyphRows GLYPH_O = {0b01110,0b10001,0b10001,0b10001,0b10001,0b10001,0b01110};
constexpr GlyphRows GLYPH_P = {0b11110,0b10001,0b10001,0b11110,0b10000,0b10000,0b10000};
constexpr GlyphRows GLYPH_R = {0b11110,0b10001,0b10001,0b11110,0b10100,0b10010,0b10001};
constexpr GlyphRows GLYPH_S = {0b01111,0b10000,0b10000,0b01110,0b00001,0b00001,0b11110};
constexpr GlyphRows GLYPH_T = {0b11111,0b00100,0b00100,0b00100,0b00100,0b00100,0b00100};
constexpr GlyphRows GLYPH_U = {0b10001,0b10001,0b10001,0b10001,0b10001,0b10001,0b01110};
constexpr GlyphRows GLYPH_V = {0b10001,0b10001,0b10001,0b10001,0b01010,0b01010,0b00100};
constexpr GlyphRows GLYPH_W = {0b10001,0b10001,0b10001,0b10101,0b10101,0b11011,0b10001};
constexpr GlyphRows GLYPH_Y = {0b10001,0b01010,0b00100,0b00100,0b00100,0b00100,0b00100};
constexpr GlyphRows GLYPH_F = {0b11111,0b10000,0b10000,0b11110,0b10000,0b10000,0b10000};

const GlyphRows* glyphFor(char ch) {
    switch (ch) {
        case ' ': return &GLYPH_SPACE;
        case ':': return &GLYPH_COLON;
        case '.': return &GLYPH_DOT;
        case '%': return &GLYPH_PERCENT;
        case '0': return &GLYPH_0;
        case '1': return &GLYPH_1;
        case '2': return &GLYPH_2;
        case '3': return &GLYPH_3;
        case '4': return &GLYPH_4;
        case '5': return &GLYPH_5;
        case '6': return &GLYPH_6;
        case '7': return &GLYPH_7;
        case '8': return &GLYPH_8;
        case '9': return &GLYPH_9;
        case 'A': return &GLYPH_A;
        case 'B': return &GLYPH_B;
        case 'D': return &GLYPH_D;
        case 'E': return &GLYPH_E;
        case 'G': return &GLYPH_G;
        case 'I': return &GLYPH_I;
        case 'L': return &GLYPH_L;
        case 'M': return &GLYPH_M;
        case 'N': return &GLYPH_N;
        case 'O': return &GLYPH_O;
        case 'P': return &GLYPH_P;
        case 'R': return &GLYPH_R;
        case 'S': return &GLYPH_S;
        case 'T': return &GLYPH_T;
        case 'U': return &GLYPH_U;
        case 'V': return &GLYPH_V;
        case 'W': return &GLYPH_W;
        case 'Y': return &GLYPH_Y;
        case 'F': return &GLYPH_F;
        default:  return &GLYPH_SPACE;
    }
}

static const char* modeLabel(PowerMode mode) {
    switch (mode) {
        case PowerMode::Normal: return "NORMAL";
        case PowerMode::PowerSaving: return "POWER SAVING";
        case PowerMode::Boost: return "BOOST";
        case PowerMode::OutOfPower: return "OUT OF POWER";
    }
    return "UNKNOWN";
}
} // namespace

std::string LightingDebugSystem::powerModeToString(PowerMode mode) {
    return modeLabel(mode);
}

void LightingDebugSystem::setState(float batteryCurrent, float batteryMax, PowerMode mode, float boostRemaining) {
    gState.batteryCurrent = batteryCurrent;
    gState.batteryMax = batteryMax;
    gState.mode = mode;
    gState.boostRemaining = boostRemaining;
}

void LightingDebugSystem::renderOverlay(SDL_Renderer* renderer) {
    if (renderer == nullptr) {
        return;
    }

    const float fraction = (gState.batteryMax > 0.0f) ? (gState.batteryCurrent / gState.batteryMax) : 0.0f;
    const int batteryPct = static_cast<int>(std::lround(std::clamp(fraction, 0.0f, 1.0f) * 100.0f));
    const bool hasBoost = gState.boostRemaining > 0.0f;

    auto batteryColor = [fraction]() -> SDL_Color {
        if (fraction > 0.5f) return SDL_Color{76, 211, 127, 255};
        if (fraction > 0.3f) return SDL_Color{242, 201, 76, 255};
        return SDL_Color{235, 92, 92, 255};
    };

    auto modeColor = [](PowerMode mode) -> SDL_Color {
        switch (mode) {
            case PowerMode::Normal: return SDL_Color{76, 211, 127, 255};
            case PowerMode::PowerSaving: return SDL_Color{242, 201, 76, 255};
            case PowerMode::Boost: return SDL_Color{90, 200, 255, 255};
            case PowerMode::OutOfPower: return SDL_Color{235, 92, 92, 255};
        }
        return SDL_Color{200, 200, 200, 255};
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Full-width HUD bar at the top of the screen.
    drawFilledRect(renderer, 0, 0, WINDOW_WIDTH, 72, 12, 14, 20, 200);
    drawFilledRect(renderer, 0, 71, WINDOW_WIDTH, 1, 70, 78, 92, 220);
    drawFilledRect(renderer, 0, 0, WINDOW_WIDTH, 4, 0, 0, 0, 60);

    const SDL_Color batt = batteryColor();
    const SDL_Color mode = modeColor(gState.mode);

    // Battery section.
    drawText(renderer, 16, 10, "BATTERY", 225, 230, 240, 255, 2);
    drawFilledRect(renderer, 16, 30, 220, 18, 24, 28, 36, 220);
    drawFilledRect(renderer, 18, 32, 216, 14, 45, 50, 64, 255);
    drawFilledRect(renderer, 18, 32, static_cast<int>(216.0f * std::clamp(fraction, 0.0f, 1.0f)), 14, batt.r, batt.g, batt.b, 255);

    char batteryPctText[16];
    std::snprintf(batteryPctText, sizeof(batteryPctText), "%d%%", batteryPct);
    drawText(renderer, 244, 28, batteryPctText, batt.r, batt.g, batt.b, 255, 2);

    // Mode badge.
    drawText(renderer, 316, 10, "MODE", 225, 230, 240, 255, 2);
    drawFilledRect(renderer, 316, 30, 192, 18, mode.r, mode.g, mode.b, 220);
    const std::string modeText = powerModeToString(gState.mode);
    drawText(renderer, 328, 28, modeText.c_str(), 255, 255, 255, 255, 2);

    // Boost countdown (only when active).
    if (hasBoost) {
        drawText(renderer, 528, 10, "BOOST", 225, 230, 240, 255, 2);
        drawFilledRect(renderer, 528, 30, 168, 18, 22, 70, 95, 220);
        char boostLine[32];
        std::snprintf(boostLine, sizeof(boostLine), "BOOST: %.1fs", std::max(0.0f, gState.boostRemaining));
        drawText(renderer, 536, 28, boostLine, 90, 200, 255, 255, 2);
    }
}

void LightingDebugSystem::drawText(SDL_Renderer* renderer, int x, int y, const char* text, Uint8 r, Uint8 g, Uint8 b, Uint8 a, int scale) {
    int cursorX = x;
    for (const char* p = text; *p; ++p) {
        char ch = static_cast<char>(std::toupper(static_cast<unsigned char>(*p)));
        drawGlyph(renderer, cursorX, y, ch, r, g, b, a, scale);
        cursorX += 6 * scale; // 5 pixels + 1 pixel spacing
    }
}

void LightingDebugSystem::drawGlyph(SDL_Renderer* renderer, int x, int y, char ch, Uint8 r, Uint8 g, Uint8 b, Uint8 a, int scale) {
    const GlyphRows* glyph = glyphFor(ch);
    for (int row = 0; row < 7; ++row) {
        const unsigned char bits = (*glyph)[row];
        for (int col = 0; col < 5; ++col) {
            if (bits & (1 << (4 - col))) {
                drawFilledRect(renderer, x + col * scale, y + row * scale, scale, scale, r, g, b, a);
            }
        }
    }
}

void LightingDebugSystem::drawFilledRect(SDL_Renderer* renderer, int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_FRect rect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)};
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
}


