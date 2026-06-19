cmake_minimum_required(VERSION 3.26)
project(PacmanDarkWorld)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)

add_subdirectory(libs/SDL)

set(SDLIMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/SDL_image)
add_subdirectory(libs/box2d/box2d-main)

set(SDLMIXER_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDLMIXER_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/SDL_mixer)

add_executable(PacmanDarkWorld
        main.cpp

        src/Game.h
        src/Game.cpp
        src/Constants.h
        src/GridMovement.h
        src/SoundManager.h
        src/SoundManager.cpp

        ecs/bagel.h
        ecs/Components.h
        ecs/LightingComponents.h

        Systems/InputSystem.h
        Systems/InputSystem.cpp
        Systems/MovementSystem.h
        Systems/MovementSystem.cpp
        Systems/RenderSystem.h
        Systems/RenderSystem.cpp
        Systems/BatterySystem.h
        Systems/BatterySystem.cpp
        Systems/GameStateSystem.h
        Systems/GameStateSystem.cpp
        Systems/GhostAISystem.h
        Systems/GhostAISystem.cpp
        Systems/VisibilitySystem.h
        Systems/VisibilitySystem.cpp
        Systems/LightingSystem.h
        Systems/LightingSystem.cpp
        Systems/LightingDebugSystem.h
        Systems/LightingDebugSystem.cpp
        Systems/ChargerSystem.h
        Systems/ChargerSystem.cpp
)

target_include_directories(PacmanDarkWorld PRIVATE
        ecs
        src
        Systems
        libs/SDL/include
        libs/box2d/box2d-main/include
        libs/SDL_image/include
        libs/SDL_mixer/include
)

target_link_libraries(PacmanDarkWorld PRIVATE
        SDL3-static
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        box2d
)

add_custom_command(TARGET PacmanDarkWorld POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/images"
        "$<TARGET_FILE_DIR:PacmanDarkWorld>/images"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:PacmanDarkWorld>/assets"
)
