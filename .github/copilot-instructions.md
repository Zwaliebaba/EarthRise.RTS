# GitHub Copilot Instructions

## Priority Guidelines

When generating code for this repository:

1. **Match the current codebase**: Prefer checked-in CMake targets, source layout, and local subsystem style over older planning notes.
2. **Preserve project boundaries**: Keep changes aligned with the current three-target layout: `NeuronCore`, `NeuronClient`, and `EarthRise`.
3. **Distinguish present from planned**: DirectX 12 and MMO/server architecture are strategic goals, not current implemented systems unless the workspace contains them.
4. **Respect local style**: This repository mixes legacy C++ and newer C++; match the file and subsystem you are editing.
5. **Keep changes minimal**: Do not modernize unrelated code while fixing a specific issue.
6. **Validate with the active build system**: Use the CMake/vcpkg workflow and the checked-in presets as the source of truth.

## Current Repository Shape

The current workspace is a CMake-based Windows C++ game codebase with three active targets:

- **NeuronCore** - static library for shared engine/core facilities such as file system, timing, math, debug helpers, events, and task utilities.
- **NeuronClient** - large legacy static library containing gameplay/client systems, LTE runtime code, rendering abstractions, UI, audio, components, networking code, scripts, and content-facing systems.
- **EarthRise** - Windows executable launcher that links `NeuronClient`, initializes the legacy runtime, loads script entry points, and copies `GameData` beside the built executable.

Do not assume absent projects such as `GameLogic`, `GameRender`, `NeuronServer`, `EarthRiseServer`, dedicated tools, or test projects exist until they are actually added to the workspace.

## Technology Detection

Before generating code, verify the current toolchain from the checked-in files:

- **Build system**: CMake, rooted at `CMakeLists.txt`.
- **Presets**: `CMakePresets.json` currently defines `x64-debug` and `x64-release` using the Visual Studio 18 2026 generator.
- **Package manager**: vcpkg manifest mode through the top-level `vcpkg.json`.
- **Language standard**:
  - MSVC builds use `/std:c++latest` from the top-level CMake options.
  - Non-MSVC builds request C++23 through CMake.
- **Platform**: Windows x64 is the active target.
- **Active dependencies**: GLEW, Freetype, and SFML 3 components (`System`, `Window`, `Graphics`, `Network`).
- **Precompiled headers**: All three active targets use `pch.h`/`pch.cpp` through `target_precompile_headers`.

Never assume packages, registries, generated projects, or frameworks beyond what is present in the CMake and manifest files.

## Strategic Direction

The intended long-term direction is to migrate EarthRise toward a native DirectX 12 renderer and MMO-capable architecture.

### DirectX 12 migration goals

- Treat the current GLEW/OpenGL/SFML rendering path as legacy until a native DirectX 12 renderer exists.
- Avoid adding new OpenGL-specific abstractions unless needed to keep existing code working.
- Prefer migration-friendly render boundaries: explicit device ownership, resource lifetime, frame resources, descriptor management, command submission, synchronization, and resize/device-loss handling.
- Use Windows-native and Direct3D-friendly types where new renderer-facing code is introduced, while preserving legacy call sites until they are deliberately migrated.
- Keep shader, texture, mesh, and material changes data-driven where possible so content can move to a D3D12 pipeline without gameplay rewrites.
- For COM ownership in new Windows/DirectX code, follow `.github/coding-standards.md` and prefer `winrt::com_ptr<T>` unless local code already establishes a different convention.

### MMO architecture goals

- Design new gameplay/networked systems with authoritative server simulation in mind.
- Separate simulation state from presentation state so future server code does not depend on rendering, audio, UI, or local input.
- Prefer stable IDs, handles, explicit serialization, deterministic tick boundaries, and clear ownership over raw cross-system pointers for network-relevant state.
- Keep client-only systems (`Renderer`, UI, audio, window/input, presentation effects) out of shared simulation logic.
- Plan for replication, interest management, persistence, matchmaking/session flow, and server observability when adding MMO-facing systems.
- Do not add server projects or network protocols speculatively; create them when a concrete task adds that architecture.

## Context Files

Prioritize these files when they are relevant:

- `.github/copilot-instructions.md`
- `.github/agents.md`
- `.github/coding-standards.md`
- `CMakeLists.txt`
- `CMakePresets.json`
- `vcpkg.json`
- nearby files in the subsystem being edited

If guidance conflicts, prefer the actual code and build files in the target subsystem, then update stale documentation as part of the change when appropriate.

## Codebase Scanning Instructions

When context files do not fully answer a question:

1. Identify similar files in the same target and folder.
2. Match existing patterns for:
   - naming
   - ownership and memory management
   - Fatal handling
   - header guards or `#pragma once`
   - logging and assertions
   - include ordering
   - PCH usage
3. Prefer consistency with the edited file over repo-wide cleanup.
4. Do not replace legacy patterns in unrelated areas just to make them modern.
5. Check CMake target membership when adding or moving source files.

## Project-Specific Guidance

### EarthRise

- Current Windows executable target built from `EarthRise/EarthRise.cpp`, `EarthRise.h`, `pch.h`, and `pch.cpp`.
- Links `NeuronClient` through whole-archive and links the discovered `sfml-main` library.
- Copies the top-level `GameData` folder beside the executable after build.
- Acts as a launcher for script-driven application entry points; preserve this flow unless a task explicitly replaces it.
- Keep app-shell code thin. New engine, renderer, simulation, and networking code should live in the owning library/target rather than growing the launcher.

### NeuronClient

- Large legacy static library with many subsystems, including `AI`, `Audio`, `CodeGen`, `Component`, `Game`, `LTE`, `Module`, `Network`, `Strukt`, `UI`, and `Volume`.
- Currently owns much of the client/gameplay/runtime surface and links GLEW, Freetype, SFML, `ole32`, and `xaudio2`.
- Contains older C++ conventions such as:
  - include guards like `#ifndef File_h__`
  - typedef aliases such as `uint`, `uint32_t`, and `uchar`
  - raw pointers and C-style strings in many areas
  - macro-based assertions and diagnostics such as `DEBUG_ASSERT`, `Fatal(...)`, `__debugbreak();`, and `DebugTrace`
- Preserve surrounding legacy style unless the user explicitly asks for modernization.
- OpenGL/GLEW-related code is legacy renderer infrastructure. Keep fixes scoped, and prefer boundaries that make a later D3D12 replacement easier.
- Existing `Network` code is not automatically an MMO server architecture. Treat it as current client/library code until a server target is introduced.

### NeuronCore

- Smaller static library for shared core code.
- Uses CMake target `NeuronCore` with public include directory set to the target root.
- Prefer keeping it renderer-agnostic and client/server-friendly where practical.
- Follow existing style in the files you touch rather than assuming it is fully modernized.

## Code Quality Standards

- Make the smallest change that solves the problem.
- Preserve binary compatibility and public interfaces unless the task requires otherwise.
- Reuse existing helper macros, typedefs, containers, and utility functions where that is already the local pattern.
- Avoid introducing new third-party libraries unless the task explicitly requires them.
- Do not add `#include` directives that are already provided by the local `pch.h` unless the file clearly needs a direct include.
- New `.cpp` files should include `pch.h` first when that matches the target convention.
- For new shared systems, prefer explicit ownership, stable identifiers, and serialization-ready data over hidden global coupling.

## Assertions and Diagnostics

Follow the local subsystem:

- In legacy NeuronClient/LTE code, prefer existing macros such as `DEBUG_ASSERT`, `Fatal`, `__debugbreak();`, and `DebugTrace`.
- In newer/core code, follow `.github/coding-standards.md` and nearby examples for `ASSERT_TEXT`, `DEBUG_ASSERT`, `Neuron::DebugTrace`, and `Neuron::Fatal` where available.
- Do not force one assertion style across the whole repo.

## Documentation Requirements

- Keep documentation aligned with the checked-in CMake build flow.
- Clearly label DirectX 12 and MMO/server architecture as planned direction until implemented.
- Remove references to absent Visual Studio solution/project files unless they reappear in the workspace.
- Match the existing markdown structure used in `.github` docs.

## Build and Validation

- Prefer the checked-in CMake presets for validation:
  - `cmake --build --preset x64-debug`
  - `cmake --build --preset x64-release`
- Treat CMake and vcpkg manifests as the source of truth for compilation status and dependencies.
- When changing package configuration, verify `CMakeLists.txt`, `CMakePresets.json`, and `vcpkg.json` together.
- No active test project is present unless one is added later. Do not claim test coverage that was not run.
- Do not claim Debug and Release are both clean unless they have actually been validated.

## General Best Practices

- Follow naming conventions exactly as they appear in existing files.
- Match code organization patterns from similar files.
- Respect the mixed legacy/modern nature of the repository.
- Keep answers and code changes grounded in the current workspace, while shaping new architecture toward DirectX 12 and MMO readiness when relevant.
