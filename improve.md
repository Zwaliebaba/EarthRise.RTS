# EarthRise.RTS — Recommended Improvements

A prioritized review of the EarthRise.RTS codebase covering architecture, safety, build, testing, documentation, and modernization. References use `path:area` for navigation; line numbers are approximate where files are large.

---

## Priority Summary

**Critical**
- No unit/integration tests exist anywhere in the project.
- `Tuple3`/`Tuple4` equality compares `a.z == a.z` (self-compare bug).
- `RefCounted::refCount` is a plain `uint`, used across threads — risk of corruption.
- `Array<T>` equality compares raw byte counts rather than element counts.

**High**
- Heavy raw `new`/`delete` in LTE containers (`Array`, `AutoPtr`, `Vector`).
- No strict warning level / `-Werror`; sanitizers absent from presets.
- Dynamic type registry (`Type.cpp`) has no synchronization.
- Const-correctness, `[[nodiscard]]`, and `override` largely missing on virtuals.

**Medium**
- SFML/OpenGL coupling blocks the D3D12 migration documented in `directx12.md`.
- Header guard style is inconsistent across `NeuronClient/LTE/*.h`.
- CMake shader-compile rules hardcoded to a single shader.
- Event/async subsystems use mutexes/atomics with unclear invariants.
- Script compile errors print to `std::cout` instead of `Neuron::DebugTrace`.

**Low**
- No top-level `README.md` or `ARCHITECTURE.md`.
- `AutoClass_Generated.h` is 164 KB in a single file → slow incremental builds.
- 269 unscoped `TODO/FIXME/HACK` comments not tracked.
- vcpkg manifest baseline not pinned in docs.

---

## 1. Architecture & Build System

### 1.1 Standardize header guards on `#pragma once`
- **Where:** `NeuronClient/LTE/*.h` (≈165 headers, mixed styles).
- **Why:** `.github/coding-standards.md:27` already specifies `#pragma once`; legacy `#ifndef` guards are inconsistent and error-prone.
- **Action:** Bulk-replace `#ifndef`/`#define`/`#endif` triplets with `#pragma once` across `NeuronClient/LTE`.

### 1.2 Document PCH inclusion policy
- **Where:** `NeuronClient/pch.h`, `NeuronCore/pch.h`.
- **Issue:** PCHs deliberately exclude `<windows.h>` to avoid macro pollution but the rationale is undocumented.
- **Action:** Add a short header comment listing what belongs in the PCH and what is forbidden (e.g., `<windows.h>`, `<d3d12.h>`).

### 1.3 Make shader compilation scale
- **Where:** `NeuronClient/CMakeLists.txt:37-82`.
- **Issue:** Compilation is hardcoded for `BootstrapClear.hlsl`.
- **Action:** Extract a CMake function `earthrise_compile_shader(SOURCE ENTRY OUTPUT)` and invoke it once per shader; emit a single index header for the loader.

### 1.4 Stage GameData outside POST_BUILD
- **Where:** `EarthRise/CMakeLists.txt:18-24`.
- **Issue:** Asset copy as a POST_BUILD step leaves stale data on partial builds.
- **Action:** Move asset staging to a dedicated `earthrise_assets` target or a CMake `install` step that can be invoked and verified independently.

### 1.5 Add a backend selection flag
- **Where:** `NeuronClient/CMakeLists.txt`.
- **Issue:** `EARTHRISE_ENABLE_D3D12` enables D3D12 linkage but OpenGL headers are still included unconditionally.
- **Action:** Introduce `set(EARTHRISE_BACKEND "OpenGL" CACHE STRING "OpenGL or D3D12")` and gate includes/sources behind it.

---

## 2. Memory Management & Pointer Safety

### 2.1 Replace LTE owning containers with std equivalents in non-reflected code
- **Where:** `NeuronClient/LTE/Array.h`, `AutoPtr.h:20-21`, `BaseVector.h`, `Vector.h` (`deleteElements`).
- **Issue:** Raw `new[]/delete[]`, manual placement-new, and implicit-ownership patterns. `Array::resize` destroys elements rather than preserving them.
- **Action:** Per `types.md` Phase 2, migrate non-reflected locals to `std::vector<T>` / `std::vector<std::unique_ptr<T>>` and `std::unique_ptr<T>`; delete `deleteElements` callers as you go.

### 2.2 Make `RefCounted::refCount` atomic (or document single-threaded use)
- **Where:** `NeuronClient/LTE/Reference.h:18-21`.
- **Issue:** `uint refCount` is incremented/decremented without synchronization. Used by Thread, XAudio2, scripting subsystems.
- **Action:** Replace with `std::atomic<uint32_t>`; use `memory_order_relaxed` for inc, `acq_rel` for dec, `acquire` after a zero-check before delete. If single-threaded by contract, add a `static_assert` and document.

### 2.3 Audit `Pointer<T>` null safety
- **Where:** `NeuronClient/LTE/Pointer.h:6, 72, 77-90`.
- **Issue:** Null-deref check is gated on `DEBUG_POINTERS`; `operator T*()` bypasses checks entirely.
- **Action:** Mark accessors `[[nodiscard]]`; in new code prefer `T&` parameters or raw `T*` with explicit invariant; consider keeping `Pointer` only inside reflected code.

### 2.4 Eliminate unsafe C string functions at compile time
- **Where:** Top-level `CMakeLists.txt`.
- **Action:** Add `/sdl /W4` (MSVC) and `-D_FORTIFY_SOURCE=2` (Clang/GCC). Treat use of `strcpy`/`sprintf`/`gets` as errors.

### 2.5 Fix `Array<T>::operator==` byte-count bug
- **Where:** `NeuronClient/LTE/Array.h` (see `types.md:186`).
- **Action:** Use `std::equal(begin(), end(), other.begin(), other.end())`; specialize on `std::is_trivially_copyable_v<T>` for `memcmp` only when sizes match in elements *and* bytes.

---

## 3. Code Quality & Modern C++

### 3.1 Add `const`, `noexcept`, `override`, `[[nodiscard]]`
- **Where:** Virtual APIs in `NeuronClient/LTE/RenderPass.h:11`, plus `Object`, `Item`, `Component`, `Module`.
- **Action:** Enable `-Wsuggest-override` (Clang) and `/w14263` (MSVC); audit `OnRender(DrawState*)` and similar to take `const DrawState*` where state is read-only; mark non-throwing destructors and swap operations `noexcept`.

### 3.2 Quarantine reflection macros
- **Where:** `NeuronClient/LTE/Common.h` (`FIELDS`, `METADATA`, `AUTOMATIC_REFLECTION_*`).
- **Issue:** Macros defeat IDE refactoring/completion.
- **Action:** Keep macros, but add `#ifdef __INTELLISENSE__` stubs that expose plain C++ structure to tooling; document policy in `coding-standards.md`.

### 3.3 Introduce a small set of concepts
- **Where:** `Vector.h`, `Map.h`, `Pointer.h`, `Reference.h`.
- **Action:** Define `Comparable`, `Hashable`, `Drawable`; constrain new template APIs to surface readable errors. CMake already targets C++23/26, so this is free.

### 3.4 Standardize enums
- **Action:** New enums must be `enum class` inside a namespace; legacy `#define X(type)` lists in `NeuronClient/Game/Common.h` may stay but are explicitly legacy. Add a clang-tidy rule to flag new bare enums.

---

## 4. Testing & Validation

### 4.1 Stand up a test framework (Critical)
- **Issue:** ~31K LOC of C++ with zero tests. The only "Test" artifact is a script (`GameData/Script/Task/Test.lts`).
- **Action:**
  1. Add a `Tests/` directory with its own CMake target using GoogleTest or Catch2 (both available via vcpkg).
  2. Seed with unit tests for pure utilities first: `NeuronCore/FileSys`, `GameMath`, `EventManager`, `DataReader`/`DataWriter`.
  3. Add round-trip tests for serialization and reflection.
  4. Wire CTest into `.github/workflows/`.

### 4.2 Enable strict warnings
- **Where:** Top-level `CMakeLists.txt`.
- **Action:**
  ```cmake
  if (MSVC)
    add_compile_options(/W4 /WX /permissive- /w14263 /w14265)
  else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror=return-type
                        -Werror=strict-aliasing -Wsuggest-override)
  endif()
  ```

### 4.3 Add sanitizer presets
- **Where:** `CMakePresets.json` (currently only `x64-debug`/`x64-release`).
- **Action:** Add `x64-debug-asan` (`/fsanitize=address`) and `x64-debug-ubsan` presets; run them in CI nightly.

---

## 5. Threading & Concurrency

### 5.1 Audit `EventManager` locking
- **Where:** `NeuronCore/EventManager.h:24` (`inline static` mutex).
- **Action:** Verify every handler-list mutation in `EventManager.cpp` is wrapped in `std::lock_guard`; consider `std::shared_mutex` if dispatch dwarfs registration.

### 5.2 Document `ASyncLoader` memory ordering
- **Where:** `NeuronCore/ASyncLoader.h:22-23`.
- **Action:** Either drop the `volatile` (atomic already provides ordering), or document why both are present. Use explicit `memory_order_acquire`/`release` on the `m_isValid` flag.

### 5.3 Lock the dynamic type registry
- **Where:** `NeuronClient/LTE/Type.cpp`.
- **Action:** Add a `std::mutex` (or `std::shared_mutex`) around the registry; expose `Type::Register` with locked semantics; document that registration after startup must use it.

---

## 6. Rendering & Assets

### 6.1 Wrap renderer state stack in RAII
- **Where:** `NeuronClient/LTE/Renderer.h` (`Push/Pop` blend/scissor/etc.).
- **Action:** Provide `RendererStateGuard{BlendMode::Alpha}` that pops on destruction; add a debug sentinel/string label to detect mismatched pushes.

### 6.2 Replace texture upload heuristic with a budget
- **Where:** `NeuronClient/LTE/Texture2D.cpp:38-56` (`IncrementalGenerateFromShader`).
- **Action:** Replace the "scale jobSize from elapsed time" heuristic with a fixed initial tile size + exponential backoff on timeout + a configurable per-frame ms budget; log decisions for profiling.

### 6.3 Document the JSL shader preprocessor
- **Where:** Custom `#include`/`#output` directives in `Shader.cpp` (per `directx12.md:59`).
- **Action:** Write a short spec for JSL syntax, add a syntax/include validator before compile, and align names to ease the eventual port to HLSL/DXC.

---

## 7. Scripting & Reflection

### 7.1 Split generated reflection metadata
- **Where:** `NeuronClient/LTE/AutoClass_Generated.h` (164 KB).
- **Action:** Generate per-subsystem files (`AutoClass_Generated_LTE.h`, `_Game.h`, `_UI.h`) so a single reflected-type change does not retrigger the world.

### 7.2 Route script errors through the logger
- **Where:** `NeuronClient/LTE/Expression.cpp:7-10`.
- **Action:** Replace `std::cout` with `Neuron::DebugTrace`; return an error result instead of best-effort continuing; persist errors to a file alongside the source `.lts` path.

---

## 8. Migration: SFML / OpenGL → Win32 / D3D12

The plan in `directx12.md` is solid; the key blocker is SFML coupling. Suggested ordering:

1. Implement Win32 windowing (HWND, message pump, resize, fullscreen toggle).
2. Stand up D3D12 device/queue/swapchain (already scaffolded per `directx12.md:26`).
3. Port shaders from GLSL/JSL to HLSL behind the new `EARTHRISE_BACKEND` flag.
4. Replace SFML image load/save with WIC or DirectXTex.
5. Only then drop SFML from `vcpkg.json` and `CMakeLists.txt`.

Track these as discrete milestones in the repo so progress is visible.

---

## 9. Documentation & Process

### 9.1 Add `README.md`
- One-paragraph pitch, quick start (`vcpkg`, `cmake --preset x64-debug`, `cmake --build`), folder map, links to `coding-standards.md`, `types.md`, `directx12.md`.

### 9.2 Add `ARCHITECTURE.md`
- High-level diagram of NeuronCore / NeuronClient / EarthRise; subsystem responsibilities (LTE, Game, UI, Module, Script, Component); data flow for load → tick → render → script → save; allowed dependency directions.

### 9.3 Restructure `coding-standards.md`
- **Where:** `.github/coding-standards.md`.
- **Action:** Split into:
  1. Mandatory rules (naming, PCH, header guards, RAII, COM smart pointers).
  2. Modern C++ rules for new code (`std::unique_ptr`, `std::string_view`, `constexpr`, `noexcept`).
  3. Legacy exceptions in `NeuronClient` (C-strings, `AutoPtr`, raw `new` where already entrenched).
  4. Modernization roadmap with links to `types.md` and `directx12.md`.

### 9.4 Track TODO/FIXME comments
- 269 occurrences found across the tree.
- **Action:** Adopt `// TODO(#issue): description` format; convert outstanding ones to GitHub issues; have CI extract a TODO report per build.

### 9.5 Pin vcpkg
- **Where:** `vcpkg.json:9`.
- **Action:** Document the required vcpkg commit in `CONTRIBUTING.md`; add a `vcpkg-configuration.json` to pin transitives; cache the vcpkg binary cache in CI.

---

## 10. Specific Bugs to Fix Now

### 10.1 `Tuple3` / `Tuple4` self-compare
- **Where:** `NeuronClient/LTE/Tuple3` (per `types.md:267`).
- **Fix:**
  ```cpp
  friend bool operator==(Tuple3 const& a, Tuple3 const& b) {
      return a.x == b.x && a.y == b.y && a.z == b.z;  // not a.z == a.z
  }
  ```
  Add a regression test.

### 10.2 `Array<T>` equality (see 2.5).

### 10.3 Add a release-mode assertion macro
- **Where:** `NeuronCore/Debug.h:50-54`.
- **Action:** `DEBUG_ASSERT` no-ops in Release. Add `ASSERT_RELEASE(expr)` for invariants in refcounting, geometry, and state machines; log to file rather than break.

---

## Suggested Roadmap

| Sprint | Theme | Items |
|---|---|---|
| 1 | Stop the bleeding | 4.1 (test framework), 10.1, 10.2, 2.2 |
| 2 | Compiler-enforced quality | 4.2, 4.3, 3.1 |
| 3 | Modernize ownership | 2.1, 2.5, 3.4 |
| 4 | Docs & scaling | 9.1–9.5, 7.1, 1.3 |
| 5+ | D3D12 migration | Section 8, 6.x |
