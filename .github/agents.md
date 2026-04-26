# AGENTS.md

## Project Overview

EarthRise is currently a Windows-native C++ game codebase built with CMake and vcpkg. The active workspace has three CMake targets.

| Target | Type | Role |
|---|---|---|
| **NeuronCore** | Static library | Shared core facilities: file system, timing, math, debugging, events, task utilities, and low-level helpers |
| **NeuronClient** | Static library | Large legacy gameplay/client/runtime library containing AI, Audio, Component, Game, LTE, Module, Network, UI, Volume, and rendering-adjacent systems |
| **EarthRise** | Executable | Thin launcher executable that links `NeuronClient`, initializes the legacy runtime, loads script application entry points, and copies `GameData` to the output directory |

Do not assume absent projects such as `GameLogic`, `GameRender`, `NeuronServer`, `EarthRiseServer`, tools, or test projects exist unless they are present in the workspace.

## Current Build and Tooling

### Build files

- Root build file: `CMakeLists.txt`
- Presets: `CMakePresets.json`
- Package manifest: `vcpkg.json`
- Active presets: `x64-debug`, `x64-release`
- Current Windows generator: Visual Studio 18 2026
- Platform: Windows x64

### Language and compiler settings

- MSVC builds use `/std:c++latest` from the top-level CMake configuration.
- Non-MSVC builds request C++23.
- C++ extensions are disabled through CMake.
- MSVC builds enable `/MP`, `/Zc:__cplusplus`, `/Zc:preprocessor`, and `/permissive-`.
- All active targets use precompiled headers through `target_precompile_headers(... pch.h)`.

### Dependencies

- vcpkg dependencies: `glew`, `freetype`, `sfml`.
- CMake finds GLEW, Freetype, and SFML 3 components: `System`, `Window`, `Graphics`, and `Network`.
- `NeuronClient` links those dependencies publicly so they propagate to `EarthRise`.
- Windows builds also link `ole32` and `xaudio2` through `NeuronClient`.
- `EarthRise` links the discovered `sfml-main` library directly.

## Strategic Direction

The intended future direction is a native DirectX 12 renderer and MMO-capable game architecture. Agents should actively account for that direction when designing new systems, while staying honest about what is implemented today.

### DirectX 12 migration

- Current rendering dependencies include GLEW/OpenGL-adjacent code and SFML. Treat these as legacy infrastructure unless the task is explicitly maintaining them.
- Do not deepen OpenGL coupling for new renderer-facing work when a migration-friendly boundary is reasonable.
- Prefer D3D12-ready concepts for new rendering architecture: device/context ownership, command queues, command lists, descriptor heaps, frame resources, upload paths, fences, resource state transitions, swap-chain lifecycle, and resize/device-loss handling.
- Keep rendering interfaces separated from gameplay simulation so a DirectX 12 backend can replace the legacy path incrementally.
- Use PIX/graphics-debugger-friendly naming and markers when adding native DirectX code.
- Prefer `winrt::com_ptr<T>` for new COM ownership when consistent with `.github/coding-standards.md`.

### MMO architecture

- Favor authoritative server simulation for future networked gameplay.
- Keep simulation, persistence, and replication concerns independent from local presentation systems.
- New network-relevant gameplay state should be serialization-ready and should prefer stable IDs or handles over raw cross-system pointers.
- Client-only code includes rendering, audio, UI, local input, camera, presentation effects, and launcher concerns.
- Server-capable code should avoid dependencies on SFML windows, graphics, audio, local input, or DirectX.
- When planning MMO work, consider tick rate, replication frequency, interest management, world partitioning, persistence, latency tolerance, security, observability, and operational tooling.
- Do not create server targets, protocols, databases, or services speculatively. Add them when a concrete task requires them.

## Repository Shape

### EarthRise

- Contains the current executable entry point in `EarthRise/EarthRise.cpp`.
- Creates the legacy window/runtime path, initializes the renderer, loads script entry points, and runs the update loop.
- Handles launcher-level behavior such as screenshots, script reload, and shutdown shortcuts.
- Should remain thin as the project evolves; avoid moving renderer, simulation, server, or content systems into this target.

### NeuronClient

- Large legacy library with many subsystems such as:
  - `AI`
  - `Audio`
  - `CodeGen`
  - `Component`
  - `Game`
  - `LTE`
  - `Module`
  - `Network`
  - `Strukt`
  - `UI`
  - `Volume`
- Common legacy patterns include:
  - include guards like `#ifndef Name_h__`
  - typedefs like `uint`, `uint32_t`, and `uchar`
  - raw pointers and manual ownership
  - C-style strings in older systems
  - macros such as `DEBUG_ASSERT`, `Fatal`, `__debugbreak();`, and `DebugTrace`
- OpenGL-related code uses GLEW-style headers and constants in places such as `LTE/GLEnum.h`.
- Existing `Network` code should be inspected before assuming it is suitable for MMO server authority, replication, or persistence.

### NeuronCore

- Smaller shared library than `NeuronClient`.
- Best home for renderer-neutral and client/server-neutral primitives when they are genuinely shared.
- Keep new shared utilities conservative, low-dependency, and friendly to both client and future server builds.

### GameData

- Runtime content copied beside the executable after build.
- Contains scripts, shaders, fonts, textures, music, sound, item/technology data, and other content folders.
- Treat scripts and data files as part of the behavior surface; search them when changing script-bound functions, reflected types, assets, or gameplay data names.

## Agent Guidance

### When changing code

1. Prefer checked-in CMake targets and source files over older planning documents.
2. Keep changes within the library or executable that already owns the code.
3. Match the local style of the file being edited.
4. Make the smallest change that solves the requested problem.
5. Avoid repo-wide cleanup unless explicitly asked.
6. For new systems, prefer boundaries that support the DirectX 12 and MMO direction without forcing premature rewrites.

### When investigating structure

- Start with `CMakeLists.txt`, the relevant target `CMakeLists.txt`, `CMakePresets.json`, and `vcpkg.json`.
- Use nearby files in the same subsystem as the primary style reference.
- Treat `.github/copilot-instructions.md` and `.github/coding-standards.md` as supporting guidance, but prefer the checked-in code when they differ.
- Search `GameData` when changing script-bound, reflected, asset-driven, or gameplay-data-driven code.

### When editing legacy NeuronClient code

- Preserve include guards if the file already uses them.
- Preserve local typedefs, macros, containers, and ownership style unless the task requires a wider refactor.
- Reuse existing assertion and diagnostic helpers.
- Do not convert legacy code to a different architectural style as part of an unrelated fix.
- Keep OpenGL maintenance scoped, but avoid expanding OpenGL-only abstractions for new renderer work.

### When designing DirectX 12 work

- Define ownership and lifetime for devices, queues, swap chains, descriptors, resources, command allocators, command lists, fences, and frame contexts.
- Separate render graph/submission concepts from gameplay object ownership.
- Account for CPU/GPU synchronization, resource state transitions, upload/readback paths, resize, fullscreen/windowed behavior, and debug-layer validation.
- Keep shader and asset pipeline decisions explicit.
- Add CMake and vcpkg changes only when the code actually needs new dependencies.

### When designing MMO work

- Start from client/server ownership and authority boundaries.
- Keep server-side simulation independent from rendering, audio, UI, local input, and launcher code.
- Design data structures for serialization, replication, persistence, and version tolerance.
- Plan for interest management and world partitioning before broad gameplay replication.
- Treat security, validation, rate limiting, and observability as first-class server concerns.

### When editing build files or dependencies

- Verify the affected target `CMakeLists.txt`, top-level `CMakeLists.txt`, `CMakePresets.json`, and `vcpkg.json` together.
- Keep dependency changes scoped to the owning target.
- Prefer target-based CMake (`target_link_libraries`, `target_include_directories`, `target_compile_definitions`) over global changes unless the setting is intentionally repo-wide.

## Specialized Agents

- **Architect**: Use for high-level design, DirectX 12 renderer boundaries, MMO/server authority models, dependency boundaries, and long-term migration strategy.
- **Planner**: Use for incremental, build-safe implementation plans, especially migration work that must preserve a runnable game after each phase.
- **SoftwareEngineer**: Use for C++ implementation, Windows/DirectX work, CMake integration, performance-sensitive engine/game systems, and focused fixes.
- **RefactorCleaner**: Use for dead code, duplicate code, stale preprocessor paths, unused includes, and cautious cleanup of legacy systems.

## Development Workflow

- Configure and build through the checked-in CMake presets.
- Use workspace diagnostics and focused compiler errors as the primary validation path.
- Build `x64-debug` and `x64-release` when changes affect build files, shared headers, dependencies, renderer code, or cross-target behavior.
- Do not document Visual Studio `.slnx` or `.vcxproj` workflows unless those files are reintroduced.

## Testing and Validation

- No active test project is present in the checked-in workspace.
- Validate changes with builds, targeted runtime checks, and focused Fatal inspection.
- Do not claim broader test coverage than has actually been run.

## Documentation Expectations

- Keep docs aligned with the repository as it exists today.
- Clearly mark planned DirectX 12/MMO work as future direction until implementation lands.
- Prefer concise, factual guidance over speculative architecture notes.
- Update `.github/agents.md`, `.github/copilot-instructions.md`, `.github/coding-standards.md`, and future architecture docs when build flow or agent behavior changes.

## Code Style Summary

- Match the edited file's indentation and brace style.
- Match the file's header style: many legacy files use include guards, newer files may use `#pragma once`.
- Do not add unnecessary includes.
- Include `pch.h` first in new `.cpp` files when adding code to targets that use PCH.
- Preserve public interfaces unless the task requires a breaking change.
- Respect the repository's mixed legacy/modern C++ character.
