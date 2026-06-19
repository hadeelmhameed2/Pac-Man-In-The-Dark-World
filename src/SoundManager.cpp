#include "SoundManager.h"

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

bool SoundManager::init() {
    if (initialized) {
        return true;
    }

    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        SDL_Log("Failed to initialize SDL audio subsystem: %s", SDL_GetError());
        return false;
    }

    if (!MIX_Init()) {
        SDL_Log("Failed to initialize MIX library: %s", SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return false;
    }

    // Create mixer device with default output and format
    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!mixer) {
        SDL_Log("Failed to create MIX mixer device: %s", SDL_GetError());
        MIX_Quit();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return false;
    }

    // Create a dedicated track for loopable background music
    bgmTrack = MIX_CreateTrack(mixer);
    if (!bgmTrack) {
        SDL_Log("Failed to create BGM track: %s", SDL_GetError());
        MIX_DestroyMixer(mixer);
        mixer = nullptr;
        MIX_Quit();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return false;
    }

    initialized = true;
    SDL_Log("SoundManager initialized successfully.");
    return true;
}

void SoundManager::clean() {
    if (!initialized) {
        return;
    }

    // Clean up all loaded audio assets
    for (auto& pair : audioMap) {
        if (pair.second) {
            MIX_DestroyAudio(pair.second);
        }
    }
    audioMap.clear();

    // Destroy the BGM track
    if (bgmTrack) {
        MIX_DestroyTrack(bgmTrack);
        bgmTrack = nullptr;
    }

    // Destroy the mixer
    if (mixer) {
        MIX_DestroyMixer(mixer);
        mixer = nullptr;
    }

    MIX_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    initialized = false;
    SDL_Log("SoundManager cleaned up successfully.");
}

bool SoundManager::loadWAV(const std::string& name, const std::string& filepath) {
    if (!initialized || !mixer) {
        SDL_Log("Cannot load audio; SoundManager is not initialized.");
        return false;
    }

    // If already loaded, do nothing
    if (audioMap.find(name) != audioMap.end()) {
        return true;
    }

    // Load with predecode=true for efficient memory-based playback
    MIX_Audio* audio = MIX_LoadAudio(mixer, filepath.c_str(), true);
    if (!audio) {
        SDL_Log("Failed to load audio file '%s': %s", filepath.c_str(), SDL_GetError());
        return false;
    }

    audioMap[name] = audio;
    SDL_Log("Loaded audio asset '%s' from '%s'.", name.c_str(), filepath.c_str());
    return true;
}

void SoundManager::playSFX(const std::string& name) {
    if (!initialized || !mixer) {
        return;
    }

    auto it = audioMap.find(name);
    if (it == audioMap.end() || !it->second) {
        SDL_Log("Sound effect '%s' is not loaded.", name.c_str());
        return;
    }

    // Fire-and-forget: SDL3_mixer automatically allocates a track for this
    MIX_PlayAudio(mixer, it->second);
}

void SoundManager::playBGM(const std::string& name) {
    if (!initialized || !mixer || !bgmTrack) {
        return;
    }

    auto it = audioMap.find(name);
    if (it == audioMap.end() || !it->second) {
        SDL_Log("BGM track '%s' is not loaded.", name.c_str());
        return;
    }

    // Stop current track playback first
    MIX_StopTrack(bgmTrack, 0);

    // Set audio source for BGM track
    MIX_SetTrackAudio(bgmTrack, it->second);

    // Configure loop options
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1); // infinite loop

    if (!MIX_PlayTrack(bgmTrack, props)) {
        SDL_Log("Failed to play BGM track '%s': %s", name.c_str(), SDL_GetError());
    }

    SDL_DestroyProperties(props);
}

void SoundManager::stopBGM() {
    if (!initialized || !bgmTrack) {
        return;
    }
    MIX_StopTrack(bgmTrack, 0);
}

void SoundManager::setBGMVolume(float volume) {
    if (!initialized || !bgmTrack) {
        return;
    }
    MIX_SetTrackGain(bgmTrack, volume);
}
