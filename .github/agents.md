# AGENTS.md

## Project Overview

EarthRise is currently a Windows-native C++ codebase built as a Visual Studio/MSBuild solution, not a CMake-based multi-executable workspace. The checked-in solution file is `EarthRise.slnx` and currently contains three projects.

| Project | Type | Role |
|---|---|---|
| **EarthRise** | Windows application | App project with WinUI/Windows App SDK package references, packaging resources, and launcher entry point |
| **NeuronClient** | Static library | Large legacy gameplay/client library with many subsystems including AI, Audio, Component, Game, LTE, Network, UI, and Volume |
| **NeuronCore** | Static library | Smaller shared/core library with a simpler project layout |

Do not assume planned projects such as `GameLogic`, `GameRender`, `NeuronServer`, `EarthRiseServer`, or test projects exist unless they are present in the workspace.

## Current Build and Tooling

### Solution and projects

- Solution file: `EarthRise.slnx`
- Project format: Visual Studio `.vcxproj`
- Platform: x64
- Build system source of truth: Visual Studio/MSBuild

### Language and toolset

- `NeuronClient`: `stdcpp20`, toolset `v145`
- `NeuronCore`: `stdcpp20`, toolset `v145`
- `EarthRise`: `stdcpp20` on Visual Studio 18+, `stdcpp17` on older IDE versions
- `EarthRise`: `v145` on Visual Studio 18+, `v143` otherwise

### Dependencies

- NuGet packages are referenced via `packages.config` and imported from the repository `packages/` folder.
- `NeuronClient` also uses a local `vcpkg.json` manifest for native dependencies.
- `EarthRise` and `NeuronCore` currently have vcpkg manifest mode disabled in their project files.

## Repository Shape

### EarthRise

- Contains `EarthRise.vcxproj`, `launch.cpp`, `Package.appxmanifest`, `app.manifest`, `resources.rc`, `resource.h`, and `Assets/`.
- Uses Windows App SDK / WinUI package imports in the project file.
- Current project settings show `PrecompiledHeader` as `NotUsing`.

### NeuronClient

- Contains many legacy subsystems such as:
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
- Style is mixed but many files are distinctly legacy C++.
- Common legacy patterns include:
  - include guards like `#ifndef Name_h__`
  - typedefs like `uint`, `uint32`, `uchar`
  - raw pointers and manual ownership
  - macros such as `LTE_ASSERT`, `error`, `debugbreak`, and `debugprint`
- OpenGL-related code uses GLEW-style headers and constants in places such as `LTE/GLEnum.h`.

### NeuronCore

- Smaller and more conventional than NeuronClient.
- Uses MSBuild with `stdcpp20` and `v145`.
- Follow existing file-local style rather than assuming a full modernization pass.

## Agent Guidance

### When changing code

1. Prefer the current checked-in solution and project files over older planning documents.
2. Keep changes within the project that already owns the code.
3. Match the local style of the file being edited.
4. Make the smallest change that solves the requested problem.
5. Avoid repo-wide cleanup unless explicitly asked.

### When investigating structure

- Start with `EarthRise.slnx` and the relevant `.vcxproj` file.
- Use nearby files in the same subsystem as the primary style reference.
- Treat `.github/copilot-instructions.md` and `.github/coding-standards.md` as supporting guidance, but prefer the checked-in code when they differ.

### When editing legacy NeuronClient code

- Preserve include guards if the file already uses them.
- Preserve local typedefs, macros, and ownership style unless the task requires a wider refactor.
- Reuse existing assertion and diagnostic helpers.
- Do not convert legacy code to a different architectural style as part of an unrelated fix.

### When editing project files or dependencies

- Verify the affected `.vcxproj`, `packages.config`, and `vcpkg.json` together.
- Keep package changes scoped to the owning project.
- Validate with the Visual Studio/MSBuild workspace build.

## Development Workflow

- Open and build the checked-in Visual Studio solution.
- Use workspace build diagnostics as the primary validation path.
- Use targeted file/error inspection for project-file and compile issues.
- Do not document or instruct CMake workflows unless the repository actually adds them.

## Testing and Validation

- No active test project is present in the checked-in solution.
- Validate changes with builds and focused error inspection.
- Do not claim broader test coverage than has actually been run.

## Documentation Expectations

- Keep docs aligned with the repository as it exists today.
- Remove or clearly label references to planned or absent projects.
- Prefer concise, factual guidance over speculative architecture notes.

## Code Style Summary

- Match the edited file's indentation and brace style.
- Match the file's header style: some files use `#pragma once`, others use legacy include guards.
- Do not add unnecessary includes.
- Preserve public interfaces unless the task requires a breaking change.
- Respect the repository's mixed legacy/modern C++ character.
