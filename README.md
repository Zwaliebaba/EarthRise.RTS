# EarthRise.RTS

EarthRise.RTS is a Windows C++ game codebase built with CMake and vcpkg. The current workspace has three active production targets:

- `NeuronCore` - shared core utilities such as file system helpers, timing, math, diagnostics, events, and task helpers.
- `NeuronClient` - legacy client/gameplay/runtime library with LTE systems, rendering abstractions, UI, audio, components, scripts, and networking code.
- `EarthRise` - Windows executable launcher that links `NeuronClient`, initializes the legacy runtime, and stages `GameData` beside the built executable.

The active modernization track is build stability, test coverage, thread/memory safety contracts, and custom-to-standard type migration. DirectX 12 and MMO/server architecture remain planned direction until concrete targets and systems are added.

## Prerequisites

- Windows x64
- Visual Studio 18 2026 with MSVC and the Microsoft C++ Unit Test Framework
- CMake 3.28 or newer
- vcpkg with `VCPKG_ROOT` set

Dependencies are resolved through vcpkg manifest mode. The current manifest pins builtin baseline `89dd0f4d241136b843fb55813b2f0fa6448c204d` and declares:

- `cppwinrt`
- `glew`
- `freetype`
- `sfml`

## Configure And Build

Use the checked-in presets as the source of truth:

```powershell
cmake --preset x64-debug
cmake --build --preset x64-debug
```

Release is available as a preset, but the current implementation notes still track x64-release validation as pending:

```powershell
cmake --preset x64-release
cmake --build --preset x64-release
```

Build outputs are written under `out/build/<presetName>`. The `EarthRise` target copies the top-level `GameData` directory beside the executable after build.

## Tests

The test projects use the Microsoft Native Unit Test framework and are registered with CTest when `vstest.console.exe` is available from the Visual Studio installation:

```powershell
ctest --test-dir out/build/x64-debug -C Debug
```

Focused test targets can also be built from CMake Tools or the command line:

```powershell
cmake --build --preset x64-debug --target NeuronCore.Test
cmake --build --preset x64-debug --target NeuronClient.Test
cmake --build --preset x64-debug --target EarthRise.Test
```

## Current Status

- x64-debug is the active validated workflow.
- `NeuronCore.Test`, `NeuronClient.Test`, and `EarthRise.Test` are present and registered with CTest Unit/Integration category filters.
- Phase 3 safety work has started. `Reference<T>` remains documented as thread-confined, type metadata registration remains startup/single-threaded by contract, `Type_Find` no longer mutates the registry on misses, and NeuronClient unit coverage now checks LTE thread wait/cancel behavior, single-threaded reference copies, and stable startup metadata lookup.
- Phase 4 documentation work has started with this README and the architecture overview.

See `impl.md` for the staged implementation plan and `build-baseline.md` for the current build baseline.