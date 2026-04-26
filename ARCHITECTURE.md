# EarthRise.RTS Architecture

This document describes the current checked-in architecture. It intentionally separates implemented systems from planned DirectX 12 and MMO/server direction.

## Active Target Map

| Target | Type | Responsibility |
|---|---|---|
| `NeuronCore` | Static library | Shared core facilities: file system helpers, timing, math, diagnostics, events, task helpers, and serialization helpers. |
| `NeuronClient` | Static library | Legacy client/gameplay/runtime surface: LTE systems, components, game logic, rendering abstractions, UI, audio, script-facing code, and networking code. |
| `EarthRise` | Windows executable | Thin launcher that links `NeuronClient`, initializes the legacy runtime, and stages `GameData`. |

Test coverage mirrors the production target map with `NeuronCore.Test`, `NeuronClient.Test`, and `EarthRise.Test` Microsoft Native Unit Test DLLs.

## Build Shape

- CMake is rooted at `CMakeLists.txt`.
- Presets are defined in `CMakePresets.json` for `x64-debug` and `x64-release`.
- vcpkg manifest mode is rooted at `vcpkg.json`.
- All active production targets use `pch.h` and `pch.cpp` through CMake precompiled headers.
- `GameData` is copied beside the `EarthRise` executable by the current post-build step.

## Dependency Boundaries

`NeuronCore` should remain renderer-agnostic and suitable for shared client/server-facing utilities where practical.

`NeuronClient` owns the legacy client runtime and currently links GLEW, Freetype, SFML 3 components, `ole32`, and `xaudio2`. OpenGL/SFML rendering remains the active legacy path.

`EarthRise` should stay thin. New engine, renderer, simulation, networking, or tooling systems should live in the owning library or a deliberate new target rather than growing the launcher.

## Safety And Modernization Contracts

Phase 3 currently documents these safety contracts:

- `Reference<T>` and `RefCounted` are thread-confined unless a specific object provides its own synchronization.
- Type metadata registration is startup/single-threaded unless runtime registration is deliberately synchronized later.
- LTE `ThreadImpl` owns its `Job` reference for the worker lifetime; callers must not concurrently mutate shared `Reference`-managed objects without external synchronization.
- `ThreadImpl::finished` is atomic and has NeuronClient unit smoke coverage.

The active modernization track is custom-to-standard type migration after safety contracts are documented. Reflected fields, script-visible APIs, serialization paths, and generated metadata are higher-risk boundaries and should not be converted mechanically.

## Planned Direction

DirectX 12 renderer work is deferred. Future renderer work should preserve migration-friendly boundaries such as explicit device ownership, resource lifetime, descriptor management, command submission, synchronization, and resize/device-loss handling.

MMO/server architecture is also planned direction, not an implemented target. Future networked systems should keep simulation state separate from presentation state and prefer stable IDs, serialization-ready data, deterministic tick boundaries, and explicit ownership.