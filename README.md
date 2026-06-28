# Pacman in the Dark World - Game Guide

Welcome to **Pacman in the Dark World**, a survival-horror spin on the classic arcade game. Locked in a pitch-black labyrinth, you must rely on a depleting battery-powered flashlight and your ears to navigate the corridors, collect pellets, and escape the monsters lurking in the shadows.

---

## 1. Game Overview

Unlike traditional Pacman, where the entire maze is visible at all times, **Pacman in the Dark World** plunges you into total darkness. Your field of view is limited to a small circle around you or the direct beam of your directional flashlight. The ghosts are invisible in the dark and only reveal their glowing eyes when they step into your light or approach your immediate vicinity.

Your goal remains to eat all the dots/pellets in the maze to win, but you must constantly manage your flashlight battery to avoid going completely blind and being consumed by the darkness.

---

## 2. Core Mechanics & Features

### 🔦 Battery & Light System
- **Power Drain**: Pacman's flashlight runs on a battery that continuously drains over time during active gameplay.
- **Dynamic Visibility**: As the battery level decreases, your field of view dynamically shrinks. The radius of your light circle matches the current battery percentage.
- **Visual Modes**:
  - **Full Light**: Visual mode is bright and fully illuminated at start, or when you trigger specific power-ups.
  - **Medium Darkness**: The battery level drops, casting the maze into deep shadows. You must rely on a close-range light radius to see details.
  - **Flashlight Only**: The ultimate power-saving mode. The ambient circle shrinks to a tiny sliver, and you must use your movement direction to point a focused flashlight beam down corridors.

### 🔋 Battery Replenishment
- **Light Pellets (`F` items)**: Flashlight batteries can be recharged by collecting battery icons (rendered as `F` letters on the map).
- **Green Chargers**: Static solar/green charging tiles are located in corners of the map. Stepping onto these chargers rapidly recharges your battery level back to 100%.

### ⚡ Boost Pads
- **Boost Mode**: Stepping onto a blue **Boost Pad** activates Pacman's emergency backup turbine.
- **Power Immunity**: Grants **5 seconds of complete immunity** from battery drain.
- **Super Flare**: The visibility light radius is boosted to **1.5x** its normal capacity, illuminating large sections of the maze.

### 👻 Smart Ghost House Exit Logic
- **Staggered Entry**: Pinky, Inky, and Clyde start the game locked inside the central Ghost House (spawn pen).
- **Exit Release Timers**: The ghosts pace back and forth inside the pen and are released sequentially:
  - **Pinky**: Leaves after 2 seconds.
  - **Inky**: Leaves after 5 seconds.
  - **Clyde**: Leaves after 8 seconds.
- **Gate Collision Exemption**: The top gate tile (`-`) is passable for ghosts only when they are exiting (`LeavingHouse`) or returning after being eaten (`EATEN`). It is permanently solid for Pacman, preventing him from hiding inside the spawn area.
- **Post-Consumption Reset**: Eating an Energizer allows Pacman to consume ghosts. Eaten ghosts instantly teleport back to the house, pace inside for 2 seconds, and then exit the house again.

### 🌀 Screen Wrapping Tunnels
- The horizontal tunnel on **Row 14** allows both Pacman and the ghosts to wrap around from the left edge to the right edge (and vice-versa) to escape tight corners and reset pathfinding chases.

### 🧠 Ghost Personalities & Pathfinding AI
Each ghost in the Dark World calculates its target tiles uniquely during the active **Chase** phase, based on the coordinates of Pacman and other entities:
- **Blinky (Red)**: The Direct Aggressor. Targets Pacman's current coordinates directly, tracking his movements continuously.
- **Pinky (Pink)**: The Ambusher. Targets 4 tiles ahead of Pacman in his current moving direction, attempting to cut him off.
- **Inky (Cyan)**: The Flanker. Calculates its target by finding the vector from Blinky's current position to 2 tiles ahead of Pacman and doubling it. This results in a flanking or pincer movement designed to trap Pacman between Blinky and itself.
- **Clyde (Orange)**: The Coward/Shy Chaser. Checks if it is far from Pacman (greater than 8 tiles/`CLYDE_SHY_DISTANCE`). If so, it chases Pacman directly; however, if it gets too close (within 8 tiles), it panics and retreats toward its designated scatter target in the bottom-left corner of the map.

During the **Scatter** phase, the ghosts retreat to the four corners of the maze:
- **Blinky**: Top-Right corner.
- **Pinky**: Top-Left corner.
- **Inky**: Bottom-Right corner.
- **Clyde**: Bottom-Left corner.

---

## 3. Audio Subsystem (Powered by SDL3_mixer)

The game features a fully dynamic, adaptive audio engine that changes based on your survival status and battery level:

| Audio Asset | Type | Trigger / Behavior |
| :--- | :--- | :--- |
| `normal_bgm` | Music (BGM) | Plays during normal gameplay when battery levels are safe. |
| `ambient_drone` | Music (BGM) | A tense, quiet drone that automatically replaces the main BGM when the battery falls into critical low-power states. |
| `heartbeat` | Sound Effect | A warning heartbeat that plays on low battery. The beat **dynamically speeds up** as your battery level drops closer to 0%. |
| `eating` | Sound Effect | A light retro pop sound triggered when eating standard dots. |
| `light_pellet` | Sound Effect | A satisfying mechanical thud when collecting battery (`F`) items. |
| `turbine_surge` | Sound Effect | A mechanical turbine surge effect when activating a blue Boost Pad. |
| `power_down` | Sound Effect | A heavy power-switch sound triggered when battery levels transition into low-power states. |
| `death` | Sound Effect | Classic retro game-over theme played when Pacman loses all battery or is caught by a ghost. |
| `victory` | Sound Effect | An upbeat chime sequence played upon successfully clearing the maze. |

---

## 4. Controls & Shortcuts

The game is played entirely using the keyboard:

### 🎮 Gameplay Navigation
- **`Up Arrow`**: Move Pacman Up
- **`Down Arrow`**: Move Pacman Down
- **`Left Arrow`**: Move Pacman Left
- **`Right Arrow`**: Move Pacman Right

### 🖥️ Menu Controls
- **`Spacebar` / `Enter`**: 
  - Starts the game from the Main Menu.
  - Resets and restarts the game from the Game Over or Victory screens.
- **`Escape`**: Quits the game immediately.
