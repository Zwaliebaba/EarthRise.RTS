# EarthRise.RTS Build Baseline

## Baseline Date
April 26, 2026

## Scope
This is the Phase 0 build baseline for the current three-target workspace:
- NeuronCore
- NeuronClient
- EarthRise

The baseline records the current build state before test framework work, warning escalation, sanitizer presets, ownership modernization, or renderer replacement work begins.

## Environment
- Host OS: Windows
- Generator: Visual Studio 18 2026
- Build preset checked: x64-debug
- Configuration: Debug
- C++ mode: MSVC `/std:c++latest` through the checked-in CMake configuration
- Windows SDK observed in compiler invocation: 10.0.26100.0
- vcpkg builtin baseline: 89dd0f4d241136b843fb55813b2f0fa6448c204d

## Build Result
- x64-debug: Passed
- x64-release: Not checked in this baseline pass
- Configure/build graph status: No stale generated shader target failure observed
- Experimental D3D12 shader status: `EARTHRISE_D3D12_GENERATED_SHADERS=0` is expected while `BootstrapClear.hlsl` is absent
- Verification pass: x64-debug passed again after PCH policy comments were added

## Warning Baseline
The verification x64-debug build completed with the following warning-code distribution. This is a broad rebuild because touching the PCH invalidated many translation units.

| Count | Warning |
|---:|---|
| 3687 | C4100 unreferenced parameter |
| 469 | C4267 size conversion, possible loss of data |
| 182 | C4458 declaration hides class member |
| 167 | C4244 conversion, possible loss of data |
| 14 | C4702 unreachable code |
| 11 | C4456 declaration hides previous local declaration |
| 3 | C4305 truncation from double to float |
| 2 | C4457 declaration hides function parameter |
| 2 | C4834 discarded nodiscard return value |
| 1 | C4189 local variable initialized but not referenced |
| 1 | C4245 signed/unsigned conversion |
| 1 | C4459 declaration hides global declaration |
| 1 | C4701 potentially uninitialized local variable |

These warnings are legacy baseline warnings. Phase 1 warning policy should not enable global warnings-as-errors until this baseline is intentionally reduced or scoped exceptions are documented.

## Current Phase 0 Notes
- The default build path is not blocked by the removed experimental shader file.
- Recursive NeuronClient include directories remain a compatibility bridge and must be curated to avoid standard/SDK header shadowing.
- NeuronClient excludes `Audio` and `LTE` from recursive include-directory expansion to reduce case-insensitive header collision risk.
- Renderer migration and D3D12 feature work remain deferred unless they are required to keep the default build graph stable.

## Next Checks
1. Re-run x64-debug after Phase 0 documentation changes.
2. Run x64-release once Debug remains reproducible.
3. Keep this file updated when warning counts, presets, compiler versions, or build-blocking failures change.
