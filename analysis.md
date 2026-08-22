# Retro Bomberman 3D — Improvement Roadmap

A comprehensive review of the codebase, organized from **quick fixes → code quality → gameplay polish → major features**.

---

## 🟢 Small / Quick Fixes (< 1 hour each)

- **Remove dead code & commented-out blocks** — [main.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/main.cpp) has an entire commented-out `Camera2D` example (lines 55–117). [map.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/map.cpp) has a large commented-out block in `RedrawExplodeArea` (lines 1169–1228). Scattered `std::cout` debug prints in [enemy.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/enemy.cpp#L181), [bomb.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/bomb.cpp), and [map.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/map.cpp). Clean all of these up.

- **Remove `.o` files from source tree** — All compiled object files (`*.o`) live inside [src/](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src) alongside the source. Your `.gitignore` excludes them from git, but they clutter the directory. Move object output to `obj/` (the `OBJ_DIR` you already define in the Makefile but don't use for the C++ targets).

- **Remove `game.exe` from the repo root** — The `.gitignore` catches `*.exe`, but the file is still sitting in the working tree. Delete it so it doesn't accidentally get committed.

- **Fix the typo `SetExploadeTimer`** → `SetExplodeTimer` in [block.h](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/block.h#L44) and [block.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/block.cpp#L260). Also `m_EMEMY` → `m_ENEMY` in [asset.h](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/asset.h#L37).

- **Fix double `break` statement** in [block.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/block.cpp#L100-L101) inside the `CLAY_BLOCK` case — there are two consecutive `break;` statements.

- **Remove the 19 MB `grass.jpg`** from [resources/](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/resources) — it doesn't appear to be loaded anywhere in the code and it's enormous. If it was used historically, consider adding it to `.gitignore` or replacing with a much smaller version.

- **Use `constexpr` / `const` for magic numbers** — values like `3.5f` speed and hardcoded animation indices `36`, `49`, `55` in [player.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/player.cpp#L174-L184) and [enemy.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/enemy.cpp#L163) should be named constants (e.g., `ANIM_IDLE = 36`, `ANIM_RUN = 49`).

---

## 🟡 Code Quality / Architecture (1–3 hours each)

- **Replace `#define` constants with `constexpr` or `enum`** — [macros.h](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/macros.h) and [asset.h](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/asset.h) use C-style `#define` for everything (`ROW`, `COL`, `BLOCK_SIZE`, `t_GRASS1`, `m_PLAYER`, etc.). Use `constexpr float`, `constexpr int`, or `enum class` for type safety and debuggability.

- **Replace global `SHOW_2D` compile-time toggle with a runtime flag** — Currently `#define SHOW_2D 0` in [macros.h](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/macros.h#L19) means you need to recompile to switch between 2D/3D. Make it a runtime setting (e.g., a `bool` on the `Game` class or a settings struct) toggled from a menu or a key press.

- **Fix encapsulation on `Human`** — The `// private:` is commented out and replaced with `public:` in [human.h](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/human.h#L19-L20), exposing all internals. Restore proper access control and add getters/setters where needed.

- **Reduce code duplication in `Player::HandleInput()` vs `Player::HandleInput2()`** — [player.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/player.cpp#L69-L258) has two nearly identical input handlers differing only in key bindings. Refactor into a single method that takes a key-mapping config or struct.

- **Reduce code duplication in collision checking** — The `adjacentRows[8]`/`adjacentColumns[8]` arrays and their iteration loops appear in at least **5 different places** across [map.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/map.cpp). Extract into a shared utility (e.g., `forEachNeighbor(row, col, callback)`).

- **Reduce code duplication in camera logic** — `Map1vs1::MoveCamera()` in [map1vs1.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/map1vs1.cpp#L85-L185) duplicates the entire camera clamping logic from `Map::MoveCamera()`. Extract a `ClampCamera(Camera2D&, Rectangle playerRect)` helper.

- **Thread safety issue in bomb countdown** — The bomb countdown thread in [map.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/map.cpp#L58-L76) calls `GetTime()` from a non-main thread, but Raylib's `GetTime()` is not guaranteed thread-safe. The bomb also stores `bombTimer` as a non-atomic `float` that's read/written from multiple threads. Consider using `std::chrono` for timing in the background thread.

- **Potential ID collision** — Player/enemy IDs are generated via `GetRandomValue(0, INT_MAX)` in [map.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/map.cpp#L138) and [map1vs1.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/map1vs1.cpp#L43). With multiple entities, there's a chance of collision. Use an incrementing counter or UUID instead.

- **Memory management inconsistency with models** — `Enemy` stores both a `Model model` (unused, line 29) and a `std::shared_ptr<Model> model1` (actually used) in [enemy.h](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/enemy.h#L29-L30). Remove the unused `model` field. Unify naming (`model` everywhere).

- **Asset loading thread is detached** — In [asset.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/asset.cpp#L164), `thr_loadAnimations` is detached. If the game exits before it finishes, it could crash. Use `join()` before window close, or guard with a flag.

- **`level.h` is missing include guards** — [level.h](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/level.h) has no `#pragma once` or include guard, risking multiple inclusion issues.

- **Improve the Makefile** — The Makefile defines `OBJ_DIR = obj` but the actual C++ compile target (`%.o: %.cpp`) writes `.o` files next to the source. Fix the pattern rule to output to `$(OBJ_DIR)`. Also add a `run` target for convenience.

---

## 🟠 Gameplay Polish (1–5 hours each)

- **Add sound effects & music** — Raylib has built-in audio support (`InitAudioDevice`, `LoadSound`, `PlaySound`). Add sounds for: bomb placement, explosion, power-up pickup, player death, level clear, and background music.

- **Add a pause menu** — There's currently no way to pause the game. Add `KEY_ESCAPE` to toggle a pause state with Resume / Restart / Quit options.

- **Add death/explosion animation** — Currently [enemy.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/enemy.cpp#L178-L182) `Dies()` just sets `alive = false`. Player death teleports to (0,0). Add a death animation (model dissolve, particle effect) and a brief invincibility window.

- **Smarter enemy AI** — Enemies in [enemy.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/enemy.cpp#L65-L169) just walk in a direction and randomly turn on collision. Add difficulty tiers:
  - **Easy**: Current random-walk behavior
  - **Medium**: Chase player when in line-of-sight
  - **Hard**: Pathfind toward player, avoid bombs

- **Add a scoring system** — Track score per level based on enemies killed, time taken, and power-ups collected. Display on the end-level UI in [map.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/map.cpp#L357-L450).

- **Level timer countdown** — Add an optional countdown timer for levels. If time runs out, the player loses. Display it prominently in the HUD.

- **Screen shake on explosions** — A quick camera shake when bombs explode adds juice. Easy to implement by briefly offsetting `camera.offset` or `camera3d.position`.

- **Transition animations between pages** — Currently page switches are instant (`page = 1`, `page = 2`). Add fade-in/fade-out or slide transitions.

---

## 🔴 Major Features (Days to weeks each)

> These align with your own feature ideas — ordered by dependency/complexity:

- **10 predefined levels + level timer** — You only have 2 handcrafted levels currently in [level.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/level.cpp). Design 8 more with increasing difficulty, varying grid sizes, enemy counts, and power-up distributions. Add a `timeLimit` field to the `Level` struct and enforce it in gameplay.

- **Level select UI overhaul** — Replace the current horizontal row of `GuiButton` calls in [game.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/game.cpp#L192-L217) with a proper grid/carousel showing level preview, difficulty rating, best time, and lock/unlock state.

- **Random level generation improvements** — The current generator in [level.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/level.cpp#L86-L178) can create unsolvable maps (no guaranteed path from player to all enemies). Add pathfinding validation (BFS/DFS) to ensure the map is solvable before presenting it.

- **Scrolling/rolling camera view** — You already have camera following logic in [map.cpp](file:///c:/Users/karan/Desktop/Code/CPP/bomberman3d/src/map.cpp#L487-L547). Extend it to support maps much larger than the screen with smooth follow, edge clamping, and minimap overlay.

- **Zoom controls** — You have mouse-wheel zoom partially implemented. Add keyboard shortcuts (`+` / `-`), a zoom slider in the UI, and pinch-to-zoom for potential touch support. Make zoom work properly in 3D mode by adjusting `camera3d.fovy`.

- **Endless mode with scoring** — Create a mode where completing a level auto-generates the next (harder) one. Track cumulative score, maintain a local high-score table, and persist it to a file.

- **Level editor / sharing** — Build a page where the user can paint tiles (grass, brick, clay) on a grid, place enemies and power-ups, and export the level to a JSON/binary file. Add an import function to play shared levels.

- **Online leaderboard** — Stand up a simple REST backend (e.g., a serverless function or a lightweight HTTP server). Submit scores with player name + time + level. Display a scrollable leaderboard in the game using Raylib's HTTP request or `libcurl`.

- **Online multiplayer — versus mode** — This is the largest undertaking. Options:
  - **Peer-to-peer with UDP**: Use a library like `ENet` or raw sockets. Implement a lobby system, input synchronization (lockstep or client-side prediction), and a matchmaking flow.
  - **Client-server**: More robust but needs a dedicated server. Consider using WebSockets for the transport.
  - Build the netcode layer as a separate module that sits between `HandleInput` and `Map` so it can serialize/deserialize game state.

- **Online multiplayer — team mode** — Extends the versus mode with team assignment, allied bomb immunity, shared score, and team-based win conditions.

---

## 📋 Summary Priority Matrix

| Priority | Category | Items |
|----------|----------|-------|
| **P0 — Do now** | Cleanup | Remove dead code, fix typos, delete `.o` files from `src/`, remove 19MB grass.jpg |
| **P1 — Quick wins** | Quality | Replace `#define` with `constexpr`/`enum`, fix encapsulation, add include guards |
| **P2 — Important** | Architecture | DRY up duplicated code (input handlers, collision loops, camera logic), fix thread safety |
| **P3 — Polish** | Gameplay | Sound, pause menu, death animations, smarter AI, scoring |
| **P4 — Features** | Content | 10 levels, level select UI, endless mode, level timer |
| **P5 — Ambitious** | Features | Level editor, online leaderboard, multiplayer |

> [!TIP]
> I'd recommend tackling **P0 → P1 → P2** first before adding any new features. A clean, well-structured codebase will make every subsequent feature 2–3× faster to implement.
