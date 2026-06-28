# Pacman in the Dark World

**Pacman in the Dark World** is a survival-horror themed adaptation of the classic arcade game built using C++, SDL3, Box2D, and an ECS architecture. Plunged into a dark, shadowy labyrinth, you must navigate using a depleting battery-powered flashlight and your ears to collect dots while avoiding ghosts invisible in the dark.

---

## 1. Core Gameplay Mechanics

### 🔋 Flashlight & Battery System
- **Active Power Drain**: Pacman's flashlight battery constantly drains over time during gameplay.
- **Dynamic Radial Light**:
  - **Full Light (>50% Battery)**: The entire maze is fully illuminated and visible.
  - **Medium Darkness (≤50% Battery)**: The maze falls into deep shadow. Pacman is surrounded by a limited radial light circle that shrinks as the battery level drops.
  - **Out of Power (0% Battery)**: The flashlight goes out, plunging the game into pitch black, leading to an immediate game over.
- **Flashlight Pickups (`F` items)**: Rendered on screen as a detailed grey flashlight icon. Collecting an `F` item instantly restores **+20.0** to the battery level.
- **Overcharge Boost**: Certain flashlight pickups trigger a temporary **Boost Mode** (lasting 5 seconds) which expands the light radius by **1.5x**, though it increases the battery consumption rate by **1.5x** as a trade-off.

### 👻 Smart Ghost House Release & Respawn
- **Staggered Entry**: Pinky, Inky, and Clyde spawn inside the central ghost house, pacing back and forth.
- **Release Timers**: Ghosts are released sequentially:
  - **Pinky**: 2 seconds.
  - **Inky**: 5 seconds.
  - **Clyde**: 8 seconds.
- **Gate Collision**: The gate tile (`-`) is a one-way path passable only by ghosts in the `LeavingHouse` or `EATEN` states. Pacman is physically blocked from crossing it, preventing him from hiding in the spawn pen.
- **Respawn Loop**: Eating an Energizer puts the ghosts into a blue, vulnerable state. Eaten ghosts immediately teleport back to the center of the house, pace inside for 2 seconds, and then exit the house again.

### 🌀 Screen Wrapping
- Tunnels at the left and right edges on **Row 14** allow both Pacman and the ghosts to wrap around horizontally.

### 🧠 Ghost AI Personalities (Chase Phase)
- **Blinky (Red)**: Direct Chaser. Targets Pacman's coordinates directly.
- **Pinky (Pink)**: Interceptor. Targets 4 tiles ahead of Pacman's direction.
- **Inky (Cyan)**: Flanker. Targets the vector from Blinky to 2 tiles ahead of Pacman, doubled.
- **Clyde (Orange)**: Shy Chaser. Chases Pacman directly when far away (>8 tiles), but flees to the bottom-left corner of the map when close (≤8 tiles).

---

## 2. Audio Subsystem (SDL3_mixer)

The game features a dynamic audio system that responds to gameplay changes:

- **BGMs**:
  - `normal_bgm`: Plays during normal gameplay (>50% battery).
  - `ambient_drone`: A low, tense ambient track that replaces the main BGM when the battery drops to 50% or below.
- **SFXs**:
  - `eating`: Retro pop sound triggered when eating standard dots.
  - `light_pellet`: Mechanical click when collecting battery/flashlight (`F`) items.
  - `power_down`: Plays when transitioning into low-power mode (≤50% battery).
  - `heartbeat`: Alarm sound playing below 30% battery. Beats faster as the battery nears 0%.
  - `ghost_vulnerable`: Siren sound playing when ghosts are frightened.
  - `turbine_surge`: Plays when entering Boost Mode.
  - `death`: Play-again theme triggered upon game over.
  - `victory`: Chime sequence played upon eating all dots.

---

## 3. Controls

- **`Arrow Keys`**: Move Pacman (Up, Down, Left, Right).
- **`Spacebar` / `Enter`**: Starts the game from the Main Menu, or restarts it from the Game Over/Victory screens.
- **`Escape`**: Quits the game.

---

## 4. Compilation & Running

### Prerequisites
- CMake 3.20+
- A C++20 compiler

### Build Commands
```powershell
cmake -B build
cmake --build build --config Release
```

### Running the Game
Run the executable from the build folder:
```powershell
.\build\Release\PacmanDarkWorld.exe
```
*(Note: Visual asset folders will be copied into the output directory automatically post-build.)*
