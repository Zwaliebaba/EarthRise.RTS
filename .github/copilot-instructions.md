# GitHub Copilot Instructions

## Priority Guidelines

When generating code for this repository:

1. **Match the current codebase**: Prefer the structure and conventions of the checked-in Visual Studio solution over older planning docs.
2. **Preserve project boundaries**: Keep changes aligned with the current three-project layout in `EarthRise.slnx`.
3. **Respect local style**: This repository mixes legacy and modern C++; match the file and subsystem you are editing.
4. **Keep changes minimal**: Do not modernize unrelated code while fixing a specific issue.
5. **Validate with the active build system**: Use the Visual Studio/MSBuild workspace build, not assumed CMake workflows.

## Current Repository Shape

The current solution is a Visual Studio solution file (`EarthRise.slnx`) containing:

- **EarthRise** — Windows application project (`EarthRise/EarthRise.vcxproj`)
- **NeuronClient** — static library (`NeuronClient/NeuronClient.vcxproj`)
- **NeuronCore** — static library (`NeuronCore/NeuronCore.vcxproj`)

Do not assume planned projects such as `GameLogic`, `GameRender`, `NeuronServer`, `EarthRiseServer`, or test projects exist unless they are actually added to the workspace.

## Technology Detection

Before generating code, verify the current toolchain from the checked-in project files:

- **Build system**: Visual Studio MSBuild `.vcxproj` projects, not CMake.
- **Language standard**:
  - `NeuronClient` and `NeuronCore` use `stdcpp20`.
  - `EarthRise` uses `stdcpp20` on Visual Studio 18+ and `stdcpp17` on older IDE versions.
- **Platform**: x64.
- **Toolset**:
  - `NeuronClient` and `NeuronCore` use `v145`.
  - `EarthRise` uses `v145` on Visual Studio 18+ and `v143` otherwise.
- **Dependencies**:
  - NuGet packages are referenced from `packages.config` and imported in each `.vcxproj`.
  - `NeuronClient` also uses a local `vcpkg.json` manifest for native packages such as GLEW, FreeType, and SFML.

Never assume packages, registries, or frameworks beyond what is present in the project files.

## Context Files

Prioritize these files when they are relevant:

- `.github/copilot-instructions.md`
- `.github/agents.md`
- `.github/coding-standards.md`
- similar files near the code being edited

If guidance conflicts, prefer the actual code in the target subsystem.

## Codebase Scanning Instructions

When context files do not fully answer a question:

1. Identify similar files in the same project and folder.
2. Match existing patterns for:
   - naming
   - ownership and memory management
   - error handling
   - header guards or `#pragma once`
   - logging and assertions
   - include ordering
3. Prefer consistency with the edited file over repo-wide cleanup.
4. Do not replace legacy patterns in unrelated areas just to make them modern.

## Project-Specific Guidance

### EarthRise

- Windows application project with WinUI/Windows App SDK package references.
- Uses app packaging resources such as `Package.appxmanifest`, `app.manifest`, `resources.rc`, and `Assets/`.
- Current project file has `PrecompiledHeader` set to `NotUsing`; do not assume `pch.h` is active here.

### NeuronClient

- Large legacy static library with many subsystems, including `AI`, `Audio`, `CodeGen`, `Component`, `Game`, `LTE`, `Module`, `Network`, `Strukt`, `UI`, and `Volume`.
- Contains older C++ conventions such as:
  - include guards like `#ifndef File_h__`
  - typedef aliases such as `uint`, `uint32`, `uchar`
  - raw pointers and C-style strings in many areas
  - macro-based assertions like `LTE_ASSERT` and `error(...)`
- When editing legacy NeuronClient code, preserve surrounding style unless the user explicitly asks for modernization.
- `NeuronClient/LTE/GLEnum.h` and related files depend on GLEW-style OpenGL constants and headers.

### NeuronCore

- Smaller static library with more conventional MSBuild settings.
- Manifest mode is currently disabled in the project file.
- Follow existing style in the files you touch rather than assuming it is fully modernized.

## Code Quality Standards

- Make the smallest change that solves the problem.
- Preserve binary compatibility and public interfaces unless the task requires otherwise.
- Reuse existing helper macros, typedefs, and utility functions where that is already the local pattern.
- Avoid introducing new third-party libraries unless the task explicitly requires them.
- Do not add `#include` directives that are already provided indirectly unless the file clearly needs a direct include.

## Assertions and Diagnostics

Follow the local subsystem:

- In legacy NeuronClient/LTE code, prefer existing macros such as `LTE_ASSERT`, `error`, `debugbreak`, and `debugprint`.
- In other areas, match the assertions and logging already used in nearby files.
- Do not force one assertion style across the whole repo.

## Documentation Requirements

- Keep documentation aligned with the checked-in solution and build flow.
- Remove references to planned or absent projects when updating docs unless they are clearly marked as future work.
- Match the existing markdown structure used in `.github` docs.

## Build and Validation

- Prefer the workspace build and file-error tools to validate changes.
- Treat Visual Studio/MSBuild as the source of truth for compilation status.
- When changing package configuration, verify the affected project files and manifest settings together.
- Do not claim Debug and Release are both clean unless they have actually been validated.

## General Best Practices

- Follow naming conventions exactly as they appear in existing files.
- Match code organization patterns from similar files.
- Respect the mixed legacy/modern nature of the repository.
- Keep answers and code changes grounded in the current workspace, not a planned future architecture.
