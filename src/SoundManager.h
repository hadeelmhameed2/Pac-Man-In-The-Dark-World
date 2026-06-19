#pragma once

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <string>
#include <unordered_map>

class SoundManager {
public:
    static SoundManager& getInstance();

    // Prevent copying or assignment
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    bool init();
    void clean();

    // Loading assets
    bool loadWAV(const std::string& name, const std::string& filepath);
    
    // Play sound effects (fire-and-forget)
    void playSFX(const std::string& name);

    // Play loopable BGM/dones using a track
    void playBGM(const std::string& name);
    void stopBGM();
    void setBGMVolume(float volume); // 0.0f to 1.0f

private:
    SoundManager() = default;
    ~SoundManager() = default;

    MIX_Mixer* mixer = nullptr;
    MIX_Track* bgmTrack = nullptr;
    std::unordered_map<std::string, MIX_Audio*> audioMap;
    bool initialized = false;
};
