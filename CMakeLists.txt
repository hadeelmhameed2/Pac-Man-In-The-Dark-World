cmake_minimum_required(VERSION 3.26)
project(PacmanDarkWorld)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)

add_subdirectory(libs/SDL)
add_subdirectory(libs/box2d/box2d-main)

add_executable(PacmanDarkWorld
        main.cpp

        src/Game.h
        src/Game.cpp
        src/Constants.h
        src/GridMovement.h

        ecs/bagel.h
        ecs/Components.h

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
)

target_include_directories(PacmanDarkWorld PRIVATE
        ecs
        src
        Systems
        libs/SDL/include
        libs/box2d/box2d-main/include
)

target_link_libraries(PacmanDarkWorld PRIVATE
        SDL3-static
        box2d
)
