cmake_minimum_required(VERSION 3.26)
project(PacmanDarkWorld)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)

add_subdirectory(libs/SDL)

add_executable(PacmanDarkWorld
        main.cpp

        src/Game.h
        src/Game.cpp
        src/Constants.h

        ecs/Entity.h
        ecs/Components.h
)

target_include_directories(PacmanDarkWorld PRIVATE
        src
        ecs
        libs/SDL/include
)

target_link_libraries(PacmanDarkWorld PRIVATE
        SDL3-static
)