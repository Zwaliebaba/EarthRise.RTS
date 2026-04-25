# DirectX 12 Migration Plan

## Purpose

EarthRise currently renders through SFML-created windows and OpenGL contexts, GLEW, the LTE `GL*` wrappers, and GLSL/JSL shader files under `GameData/Shader`. The target is a native Win32 application shell with a DirectX 12 renderer, no OpenGL/GLEW backend, and no SFML dependency.

The best approach is an incremental replacement, not a one-shot rewrite and not a long-lived dual-backend architecture. The renderer is deeply OpenGL-shaped today: public interfaces expose `GL_TextureFormat`, `GL_Texture`, `GL_Buffer`, framebuffer stacks, shader uniform locations, fixed-function fallbacks, OpenGL draw modes, and GLSL shader conventions. A direct replacement would touch gameplay, UI, render passes, texture generation, mesh uploads, shader instances, and window/input code all at once. Instead, first carve out D3D12-friendly boundaries, then replace each OpenGL/SFML subsystem with native Win32/D3D12 code until the legacy backend and dependencies are deleted completely.

## Resolved Decisions

- The final renderer is D3D12-only. Do not keep an OpenGL fallback backend.
- SFML should be removed completely, including windowing, input, image utilities, timing/sleep, joystick, and network/web usage.
- Minimum GPU support is Direct3D feature level 12_1.
- Shaders should be compiled into generated C++ header files, not loaded as loose runtime bytecode files in the final path.
- Asset format changes are allowed; prefer D3D-friendly assets such as DDS/BC textures and precomputed mipmaps where useful.
- The renderer may redesign the deferred pipeline, render-target layout, material bindings, and pass structure where it reduces technical debt.
- Borderless fullscreen is enough; no fullscreen-exclusive or multi-monitor-specific milestone is required initially.
- PIX markers and GPU timing queries are part of the first D3D12 implementation, not a later polish item.

## Implementation Status

- First implementation slice is build-validated and the next bootstrap slice has started.
- Added `EARTHRISE_ENABLE_D3D12`, D3D12 WinSDK library linkage (`d3d12`, `dxgi`, `dxguid`), Win32 window linkage (`user32`), and DXC discovery.
- Added conditional shader-header generation for a bootstrap fullscreen-triangle HLSL shader. In the active CMake Tools Debug build, DXC was found and generated `BootstrapClearVS.h` and `BootstrapClearPS.h` under the build tree.
- Added a non-active D3D12 adapter probe that uses `winrt::com_ptr`, enumerates DXGI adapters by high-performance preference, and validates hardware Direct3D feature level 12_1 support.
- Added a non-active Win32/D3D12 clear-present smoke path in `NeuronClient/LTE/Render/D3D12/D3D12ClearPresent.*`. It creates a native Win32 window, selects a feature-level 12_1 hardware adapter, creates a D3D12 device/queue/swap chain/RTVs/fence, clears and presents one frame, emits a command-list marker, resolves GPU timestamp queries, and consumes generated shader blobs when they are available.
- The active SFML/OpenGL boot path is intentionally unchanged while the Win32/D3D12 clear-present path remains a non-active smoke path.
- Current validation: active Debug CMake Tools build completed successfully with D3D12 scaffolding and the new clear-present smoke path enabled.
- Current diagnostics: no errors in the new D3D12 scaffolding files or migration document. The latest CMake Tools diagnostics report one unrelated legacy warning in `Texture2D.cpp` for ignoring an `sf::Image` `[[nodiscard]]` result.
- Current working-tree state: D3D12 scaffolding and this plan are uncommitted; the earlier `types.md` ownership-cleanup changes are also still present in the same working tree.
- Next implementation step: add an explicit developer/runtime entry point for the smoke path, run it with the D3D12 debug layer, then harden resize/device-removal/shutdown behavior before making Win32/D3D12 the active bootstrap.

## Current State

### Build and Dependencies

- `vcpkg.json` currently depends on `glew`, `freetype`, and `sfml`.
- Top-level `CMakeLists.txt` finds GLEW, Freetype, and SFML 3 components `System`, `Window`, `Graphics`, and `Network`.
- `NeuronClient` links GLEW, Freetype, SFML, `ole32`, and `xaudio2` on the active legacy path.
- When `EARTHRISE_ENABLE_D3D12` is on, `NeuronClient` also links Windows SDK libraries `d3d12`, `dxgi`, `dxguid`, and `user32` for the D3D12/Win32 scaffold.
- Shader compilation should use DXC at build time. The generated output should be C++ header files containing compiled DXIL blobs and small metadata tables consumed by the renderer.

### Rendering Surface

Important current files:

- `NeuronClient/LTE/Window.cpp` owns the SFML `sf::RenderWindow`, event polling, resize handling, mouse/key translation, fullscreen, icon, and vsync.
- `NeuronClient/LTE/Renderer.cpp` owns global OpenGL render state, framebuffer cache, buffer binding, viewport/scissor/depth/blend/cull stacks, draw call counters, matrix state, and mesh draw preparation.
- `NeuronClient/LTE/Shader.cpp` owns JSL preprocessing, GLSL compile/link, shader caches, uniform lookup, texture unit assignment, and matrix injection.
- `NeuronClient/LTE/Texture2D.cpp`, `Texture3D.cpp`, and `CubeMap.cpp` own GL texture creation, bind/unbind, render-target behavior, mip generation, readback, and image load/save.
- `NeuronClient/LTE/Mesh.h` stores mutable GL VBO/IBO handles directly on `MeshT`.
- `NeuronClient/LTE/GLEnum.h`, `GLType.h`, and `GL.h` expose many OpenGL concepts into higher-level code.
- `NeuronClient/Game/RenderPass/*.cpp` and `NeuronClient/Game/RenderPasses.h` define the current frame graph-like pass sequence using `Texture2D`, `DrawState`, `Shader`, and `Renderer_*` calls.

### Shader Content

- Shaders live under `GameData/Shader/Common`, `GameData/Shader/Vertex`, and `GameData/Shader/Fragment`.
- There are roughly 169 `.jsl` shader files.
- `Shader.cpp` prepends `#version 120` and performs a custom preprocessor for `#include` and `#output`.
- Shader code uses GLSL-era conventions such as `gl_Position`, `gl_FragData`, `sampler2D`, `texture2D`, `VERT_OUT`, `FRAG_IN`, `RETURN`, and engine globals such as `WORLD`, `VIEW`, `PROJ`, `worldPos`, `linearDepth`, and `farPlane`.
- The renderer uses logarithmic depth and deferred render passes, so clip-space and depth behavior must be validated carefully.

### SFML Usage Beyond Rendering

SFML is not only the window/context layer today:

- `Window.cpp`, `Mouse.cpp`, `Keyboard.cpp`, and `Joystick.cpp` use SFML window/input APIs.
- `Texture2D.cpp` and `CubeMap.cpp` use `sf::Image` for image load/save and icon/cubemap exports.
- `Timer.cpp` and `Thread.cpp` use `sf::Clock` and `sf::sleep`.
- `Location.cpp` uses `sf::Http` for web locations.

The DirectX 12 migration should remove SFML completely. Rendering/window/input removal is the first priority, but the plan must also replace SFML image load/save, timing/sleep, joystick, and HTTP usage before the migration is considered complete.

### SFML Removal Workstreams

| SFML Area | Current Files | Replacement |
|---|---|---|
| Window and events | `Window.cpp` | Win32 `HWND`, WndProc, DXGI-friendly resize/focus handling |
| Keyboard and mouse | `Keyboard.cpp`, `Mouse.cpp` | Win32 messages plus Raw Input for high-precision mouse paths |
| Joystick/gamepad | `Joystick.cpp` | XInput or Windows Gaming Input |
| Image load/save | `Texture2D.cpp`, `CubeMap.cpp` | WIC for common image formats plus DirectXTex for DDS/BC assets |
| Time and sleep | `Timer.cpp`, `Thread.cpp` | `std::chrono`, waitable timers, or Win32 sleep/wait APIs |
| HTTP/web locations | `Location.cpp` | WinHTTP or an approved non-SFML HTTP path |
| SFML entry/linking | `CMakeLists.txt`, `EarthRise/CMakeLists.txt` | Native Windows subsystem/link behavior without `sfml-main` |

## Target Architecture

### Recommended Module Shape

Keep the current three-target layout for now:

- `EarthRise`: thin launcher and Win32 entry integration where needed.
- `NeuronClient`: client runtime, rendering, UI, input, audio, gameplay, and scripts.
- `NeuronCore`: renderer-agnostic utilities.

Add new implementation folders under `NeuronClient/LTE` rather than new projects at first:

- `Platform/Win32`: HWND creation, message pump, raw input, cursor/fullscreen, DPI, timing helpers.
- `Render`: API-neutral renderer-facing types and resource handles.
- `Render/D3D12`: device, swap chain, command queues, descriptors, frame resources, upload/readback, pipeline cache, generated shader library, PIX markers, timing queries, and debug tooling.

Only split a dedicated render library later if compile times or ownership boundaries justify it.

### Core Design

Introduce a D3D12-first `RenderCore` layer, not a general-purpose multi-backend RHI. The layer should hide D3D12 COM objects from gameplay, UI, reflection, and scripts, but its descriptors and lifetime rules should map naturally to D3D12 rather than abstracting for hypothetical future APIs:

- `RenderDevice`: owns adapter selection, D3D12 device, queues, allocator pools, descriptor heaps, debug layer, and lifetime fences.
- `RenderSwapChain`: owns DXGI swap chain, back buffers, present mode, resize, fullscreen policy, and frame pacing.
- `RenderCommandContext`: records draw/copy/resource barriers for one frame or pass.
- `RenderTexture`, `RenderBuffer`, `RenderSampler`, `RenderPipeline`, `RenderShader`, `RenderRenderTargetSet`: API-neutral handles with D3D12 resources behind them.
- `RenderFrame`: per-frame command allocator/list, upload arena, transient descriptor ranges, retire fence, and deferred releases.
- `RenderPassGraph` or lightweight pass builder: first model the existing `DrawState` primary/secondary/tertiary/depth buffers, then evolve toward explicit inputs/outputs.

Use `winrt::com_ptr<T>` for COM ownership in new Windows/DirectX code, matching the repository guidance for new DirectX work.

### Migration Principle

Do not expose D3D12 handles to gameplay, UI, or object systems. New code should move in this direction:

```cpp
Texture2D/Shader/Mesh/RenderPass call sites
  -> API-neutral Render interfaces
  -> D3D12 backend implementation
  -> ID3D12Device/ID3D12GraphicsCommandList resources
```

The current OpenGL names can remain only as temporary migration scaffolding. New abstractions should not be named `GL_*`, and compatibility shims must have a clear deletion path rather than becoming a second backend.

### Transitional Boot Strategy

Native Win32 windowing and D3D12 device bring-up should be treated as one tightly-coupled milestone. Replacing `WindowT` with a Win32 implementation before the D3D12 swap chain exists would strand the current OpenGL renderer without its SFML-created OpenGL context. The safe cutover is:

1. Add Win32 window creation and D3D12 device/swap-chain code together.
2. Keep the old SFML/OpenGL startup path only until the new Win32/D3D12 clear-present path is active.
3. Make the Win32/D3D12 path the active boot path once it can create a window, clear, present, resize, and shut down cleanly.
4. Delete OpenGL/SFML responsibilities incrementally after their D3D12/native replacements are active.

This is still not a permanent dual-backend strategy; it is a temporary bootstrapping path to keep the tree buildable and runnable during the first cutover.

## Best Migration Approach

### 1. Replace OpenGL Behind Temporary Compatibility Adapters

First, isolate OpenGL without changing behavior:

- Move GL implementation details out of public headers where possible.
- Replace `GL_Texture`, `GL_Buffer`, `GL_Program`, and `GL_Framebuffer` exposure in higher-level interfaces with renderer-owned handles.
- Add neutral enums for texture format, pixel format, index format, blend mode, cull mode, filter mode, wrap mode, primitive topology, and depth state.
- Keep conversion helpers from existing `GL_*` enums only during transition.
- Keep existing `Renderer_*`, `Texture_Create`, `Shader_Create`, and render pass call sites compiling while their internals forward through the new interface.
- Remove the OpenGL implementation as soon as each equivalent D3D12 path exists; do not preserve backend selection as a product feature.

This creates a temporary seam where D3D12 can replace OpenGL without rewriting every render pass first.

### 2. Build Native Win32 Windowing Before Swap Chain Work

Create a native Win32 `WindowT` implementation that replaces the SFML implementation:

- `HWND`, `HINSTANCE`, WndProc, message pump, close/focus/resize events.
- DPI awareness and correct client-area sizing.
- Borderless fullscreen policy using DXGI-friendly modes; fullscreen-exclusive support is not required for the initial migration.
- Cursor visibility/capture using Win32 APIs.
- Keyboard/mouse translation into existing `Keyboard_*` and `Mouse_*` functions.
- Raw mouse input as a later improvement for camera controls.
- Joystick/gamepad replacement through XInput or Windows Gaming Input.
- Replacement of `sf::Clock`, `sf::sleep`, `sf::Image`, and `sf::Http` with Win32/standard-library/WIC/WinHTTP or project-local equivalents.

Do this before D3D12 swap chain creation so resize, focus, and presentation behavior is owned by the new platform layer.

### 3. Add the Minimal D3D12 Backend

The first D3D12 milestone should render a clear color and present through the Win32 window:

- Enable D3D12 debug layer in Debug builds.
- Require and validate Direct3D feature level 12_1.
- Select adapter through DXGI, prefer high-performance hardware adapter, and log adapter name/feature level.
- Create device, direct command queue, swap chain, RTV heap, back buffers, command allocators, command list, fence, and frame index handling.
- Implement resize by waiting for GPU idle, releasing back buffers, resizing swap chain, and recreating RTVs.
- Add GPU crash/debug naming for resources and command lists.
- Add PIX markers and GPU timestamp queries from the beginning so frame captures and timing are useful during every migration milestone.
- Add a temporary build flag only if it is needed to keep the tree compiling during the replacement. The final build should not expose OpenGL as a selectable backend.

### 4. Port Resources and Draw Submission

After a presenting D3D12 device exists, port resource types in this order:

1. Vertex/index buffers for `MeshT`.
2. Immutable 2D textures for loaded images and render constants.
3. Render-target textures matching current `Texture2D::Bind`/`Unbind` behavior.
4. Depth buffers and readback paths.
5. Cube maps and 3D textures.
6. Dynamic/upload buffers for UI, particles, generated meshes, and per-frame constants.

Avoid storing backend resource handles directly on reflected objects. `MeshT` currently stores GL VBO/IBO handles, but the D3D12 version should use a renderer resource cache keyed by mesh version or a small non-reflected resource payload owned outside serialized fields.

### 5. Replace GL State Stacks With Explicit Pipeline State

D3D12 rewards explicit immutable pipeline state. Map the current state system gradually:

| Current Concept | D3D12 Target |
|---|---|
| `Renderer_PushBlendMode` | Blend state in PSO key |
| `Renderer_PushCullMode` | Rasterizer state in PSO key |
| `Renderer_PushZBuffer` / `PushZWritable` | Depth/stencil state in PSO key |
| `Renderer_SetShader` | Bound graphics pipeline plus root signature |
| `Renderer_PushColorBuffer` / depth attachment | Render target/depth target set with barriers |
| `Renderer_SetViewport` / scissor | Command-list viewport/scissor commands |
| `ShaderT::SetTexture*` | Descriptor table binding |
| `ShaderT::SetFloat*` / matrix uniforms | Constant buffer or root constants |
| `GL_Framebuffer` cache | Render target set/cache and explicit resource states |
| `glGenerateMipmap` | compute or graphics mip generation pass |

Keep the old push/pop convenience APIs initially, but make them build a `RenderStateKey` and pass-local binding description rather than issuing immediate GL calls.

### 6. Convert Shaders Deliberately, Not By Blind Translation

Do not rely on runtime GLSL translation as the long-term solution. The current shader dialect is old GLSL plus custom JSL macros, and D3D12 needs stable input layouts, root signatures, descriptor spaces, and compiled bytecode.

Recommended shader path:

1. Preserve the existing JSL preprocessor behavior for includes as an inventory aid.
2. Add `GameData/ShaderDX12` or `GameData/Shader/HLSL` for HLSL sources instead of overwriting GLSL immediately.
3. Add a shader manifest that maps logical shader names to vertex/pixel/compute entry points, defines, root-signature layout, render target formats, depth format, blend/cull/depth states, and input layout.
4. Port common includes first: math, color, noise, deferred, lighting, scattering, SMAA, soft particles, and log-depth helpers.
5. Convert foundational vertex shaders and a minimal unlit/fullscreen pixel shader.
6. Bring up render passes one by one: clear, fullscreen quad, default mesh, GBuffer, lighting, post-process, UI, particles, special effects.
7. Compile HLSL with DXC at build time.
8. Generate C++ header files containing compiled DXIL byte arrays plus metadata tables for entry point, shader stage, root signature, render-target formats, and input layout.
9. Include those generated headers in the renderer so the final runtime does not load loose shader bytecode files.

Use automation only for mechanical help:

- `#include` path rewrite.
- `VERT_OUT`/`FRAG_IN` to HLSL struct fields.
- `texture2D` to `Texture2D.Sample` placeholders.
- `gl_FragData[n]` to named multiple render target outputs.
- Matrix/global uniform names to constant-buffer fields.

Manual review remains necessary for coordinate systems, texture sampling, depth, MRT formats, and blend expectations.

### Shader Header Generation Rules

- Generate one C++ header per shader or logical shader group, plus one small registry header.
- Include generated shader blobs from a narrow renderer `.cpp` file or generated shader library unit; do not include large generated byte arrays from widely included headers.
- Generate deterministic output with stable ordering and avoid rewriting generated files when content is unchanged.
- Track HLSL `#include` dependencies so CMake rebuilds only affected generated headers.
- Store source path, entry point, shader stage, root signature ID, input layout ID, render target formats, and debug name in generated metadata.
- Compile Debug shaders with usable symbols and names for PIX. Compile Release shaders optimized.
- Treat generated headers as build artifacts unless there is a deliberate decision to check them in.

### Render Pipeline Redesign Targets

Redesign is allowed where it makes the D3D12 renderer simpler, faster, or easier to reason about. High-value redesign targets are:

- Replace implicit `DrawState` buffer swapping with explicit pass inputs, outputs, formats, and resource states.
- Define GBuffer/material targets around D3D12 bandwidth and format needs rather than copying the OpenGL layout blindly.
- Prefer fullscreen triangles over GL-era fullscreen quads.
- Replace string-based uniform setting with typed pass/material/object binding records.
- Move generated/procedural texture jobs to compute shaders where that simplifies scheduling or barriers.
- Make root signatures, descriptor tables, and sampler usage explicit in shader metadata.
- Preserve visual behavior where it matters, but do not preserve OpenGL implementation structure for its own sake.

### 7. Port Render Passes By Visual Milestone

A good order is:

1. Window + swap chain clear.
2. Fullscreen triangle/quad and a solid-color test shader.
3. Static mesh draw with default camera matrices.
4. Loaded texture sampling.
5. Depth buffer and culling parity.
6. `DrawState` primary/secondary/tertiary/depth render targets.
7. GBuffer pass.
8. Global lighting and local lighting.
9. SSAO/SMAA/post processing.
10. UI/widget/text rendering.
11. Particles, trails, imposters, cubemaps, generated textures, and procedural shader jobs.
12. Screenshot/readback/save paths.

At each milestone, use captured OpenGL screenshots and behavior logs as comparison data, but do not keep OpenGL as a runtime fallback once the D3D12 replacement for that area is accepted.

## Detailed Phases

### Phase 0: Baseline and Inventory

Deliverables:

- Add `directx12.md` and a shader/rendering inventory.
- Add temporary migration flags only where needed; avoid a permanent backend-selection system.
- List all `GL_`, `GLEW`, `sf::RenderWindow`, `sf::Image`, `sf::Keyboard`, `sf::Mouse`, `sf::Joystick`, and `sf::Clock` usage.
- Capture screenshots and frame behavior from the current OpenGL renderer for comparison.

Validation:

- Existing build still passes until the D3D12 replacement begins deleting OpenGL paths.
- Startup smoke test still works.

### Phase 1: Native Win32 Platform Layer

Deliverables:

- Add `Platform/Win32/WindowWin32.cpp` implementation for `WindowT` and make it the active window implementation.
- Replace render-facing `GetImplData()` expectations with either `HWND` or a typed platform handle helper.
- Translate Win32 keyboard/mouse events into existing LTE input state.
- Add Win32 cursor capture, cursor visibility, close, resize, focus, position, borderless fullscreen, and vsync policy placeholders.
- Replace SFML input polling and begin removing SFML `Window`/`Graphics` dependencies.

Validation:

- Open a native Win32 window without SFML.
- Correct resize/focus/input event logging.
- Existing scripts can create and close the window.
- No `sf::RenderWindow`, `sf::Keyboard`, `sf::Mouse`, or `sf::Joystick` usage remains on the active path.

### Phase 2: D3D12 Device Bring-Up

Deliverables:

- `Render/D3D12/D3D12Device.*` with debug layer, adapter selection, device, queues, swap chain, command lists, fences, RTV heap, and frame resources.
- Feature level 12_1 validation and a clear fatal error for unsupported adapters.
- Present clear color to the Win32 window.
- Robust resize and device-removal handling.
- GPU object naming, debug logging, PIX markers, and timestamp query readback.

Validation:

- Debug layer reports no live objects on shutdown.
- Resize repeatedly without leaks or crashes.
- Present at vsync on/off modes.
- PIX capture shows named passes/resources and usable GPU timing ranges.

### Phase 3: API-Neutral Render Types

Deliverables:

- New neutral enums and descriptors for texture formats, buffer usage, index format, sampler state, blend/cull/depth state, topology, and render target layout.
- Compatibility conversion from current `GL_TextureFormat`, `GL_IndexFormat`, and related enums.
- `Texture2D`, `Texture3D`, `CubeMap`, `Mesh`, and `Shader` internals forward to backend-neutral resources.
- Remove new dependencies on `GLType.h` from high-level headers.

Validation:

- Temporary compatibility adapters compile until their D3D12 replacements are complete and the GL code is deleted.
- D3D12 backend can create equivalent resources for simple test assets.

### Phase 4: Meshes, Textures, and Uploads

Deliverables:

- D3D12 vertex/index buffer upload path for `MeshT`.
- D3D12 immutable texture upload and render-target texture creation.
- Resource state tracking and barriers for render target, shader resource, copy source/dest, depth write/read, and present.
- Upload ring/arena to avoid per-resource command allocator churn.
- WIC-based image load/save replacement for `sf::Image` plus DirectXTex or equivalent tooling for DDS/BC textures and precomputed mipmaps.

Validation:

- Draw a textured mesh.
- Texture load/readback/save parity for common asset formats.
- DDS/BC texture path works for at least one representative texture.
- No per-frame resource leaks under debug layer.

### Phase 5: Shader Pipeline

Deliverables:

- DXC integration in CMake or asset tooling.
- HLSL common include library and shader manifest.
- Generated C++ shader headers containing DXIL blobs and metadata.
- Root signature strategy: start with one common graphics root signature, split only when profiling or descriptor pressure requires it.
- Constant-buffer layout for frame, view, object, material, and draw constants.
- Descriptor heap strategy for SRV/UAV/CBV and samplers.
- Developer shader rebuild command equivalent to `Shader_RecompileAll` that regenerates headers and relinks/rebuilds as needed.

Validation:

- Compile all migrated HLSL at build time.
- Generated shader headers are deterministic and included by the renderer.
- D3D12 debug layer sees valid root signature and descriptor usage.

### Phase 6: Render Pass Migration

Deliverables:

- D3D12 implementations for existing visual features in migration order: clear, depth, GBuffer or redesigned visibility/material passes, global lighting, local lighting, SSAO, SMAA or a replacement anti-aliasing pass, blended, particles, lens flares, UI/widgets.
- `DrawState` replacement or adapter that explicitly declares pass inputs/outputs.
- Render target format decisions for albedo, normals, material properties, depth, HDR color, and post-process buffers; redesign is allowed where it simplifies D3D12 resource state, bandwidth, or shader binding.
- Fullscreen pass helper using a fullscreen triangle instead of GL-era quad state where possible.

Validation:

- Side-by-side screenshots against OpenGL milestones.
- GPU capture with PIX for the main frame.
- Pass timing and draw call/poly counters restored or replaced.

### Phase 7: Remove Render SFML, GLEW, and OpenGL

Deliverables:

- Remove GLEW and OpenGL includes from renderer-facing headers.
- Remove `GL.h`, `GLEnum.h`, and `GLType.h` from public render APIs, then delete the legacy GL implementation.
- Replace all SFML usage, including window/input/image/timer/sleep/joystick/network helpers.
- Update `vcpkg.json` and CMake dependencies:
  - remove `glew`,
  - remove all SFML components,
  - replace `Location_Web` with WinHTTP, cpp-httplib, or another approved non-SFML path,
  - keep Freetype unless text rendering is also replaced.
- Remove `sfml-main` lookup/linking if no longer needed.

Validation:

- Clean configure from an empty build directory.
- Debug and Release builds pass.
- No `GL/glew.h`, `opengl32`, `sf::` references, or SFML/GLEW package dependencies remain in the D3D12-only build.

## Build System Plan

Initial CMake shape:

```cmake
option(EARTHRISE_ENABLE_D3D12 "Build the DirectX 12 renderer" ON)

if (WIN32 AND EARTHRISE_ENABLE_D3D12)
  target_link_libraries(NeuronClient PUBLIC d3d12 dxgi dxguid user32)
endif()
```

Shader compilation is now partially integrated for the bootstrap path:

- CMake discovers DXC when `EARTHRISE_ENABLE_D3D12` is enabled.
- `BootstrapClear.hlsl` generates `BootstrapClearVS.h` and `BootstrapClearPS.h` under the build tree when DXC is available.
- The clear-present smoke path includes the generated shader headers only from its narrow implementation `.cpp` when `EARTHRISE_D3D12_GENERATED_SHADERS=1`.
- Command-list markers and GPU timestamp query readback are compiled into the smoke path.
- Build-time shader compilation is required. Runtime loose shader loading is not part of the final D3D12 path.

Dependency decisions:

- D3D12, DXGI, DXGUID, WIC, WinHTTP, and XInput can come from the Windows SDK.
- DXC should be provided either through vcpkg `directx-dxc` or a checked tool path with version logging.
- DirectXTex should be considered for DDS/BC texture tooling and mip generation.
- PIX markers should use WinPixEventRuntime or a small compile-time gated wrapper around the available PIX event API.
- SFML and GLEW should be removed from `vcpkg.json` as soon as no active code path requires them.

## D3D12 Technical Decisions

### Descriptor Management

- Use one shader-visible CBV/SRV/UAV heap per frame initially.
- Use CPU-only heaps for persistent RTV/DSV descriptors.
- Allocate transient descriptor ranges linearly per frame.
- Add a persistent bindless-style table only after the migrated renderer needs it.

Initial budgets should be explicit and easy to tune:

- Frames in flight: start with `3` unless latency profiling argues otherwise.
- Shader-visible CBV/SRV/UAV descriptors: start with a generous per-frame transient heap and log high-water usage.
- Sampler descriptors: use a small static sampler table first, then add descriptor heap allocation only if dynamic samplers are needed.
- RTV/DSV descriptors: allocate from CPU-only persistent heaps with names and leak tracking.
- Upload memory: use page-based upload arenas with per-frame reset after fence completion.
- Deferred releases: queue resources by fence value and release only after GPU completion.

### Frame Resources

- Use 2 or 3 frames in flight.
- Each frame owns command allocator, upload pages, transient descriptors, and deferred-release lists.
- Retire resources only after the frame fence completes.

### Constants and Materials

- Replace ad hoc uniform setting with typed constant buffers.
- Suggested buffers: `FrameConstants`, `ViewConstants`, `ObjectConstants`, `MaterialConstants`, `PostProcessConstants`.
- Keep high-level `shader("name", value)` calls temporarily by writing into a compatibility constant store, but phase them out in favor of generated/typed binding layouts.

### Resource Lifetime

- Avoid immediate destruction of GPU resources.
- Queue deleted resources for deferred release by fence value.
- Never let reflected gameplay objects own raw D3D12 COM objects directly.

### Feature Level Failure Behavior

- Enumerate and log all adapters considered during startup.
- Require Direct3D feature level 12_1 and fail before game startup if no supported adapter exists.
- Log adapter name, vendor ID, device ID, dedicated memory, and chosen feature level.
- WARP can be allowed for developer diagnostics only, not as a supported player runtime path.
- Device-removal errors should log `GetDeviceRemovedReason()` and enough context to make PIX captures actionable.

### Coordinate and Format Risks

- D3D clip-space depth is `0..1`; OpenGL is historically `-1..1` before projection conventions. Audit all projection, log-depth, and depth reconstruction code.
- Texture coordinate origin and framebuffer readback orientation differ from current GL assumptions.
- Front-face winding and cull mode must match existing content, currently CCW front faces.
- sRGB formats and gamma handling should be made explicit; do not rely on implicit GL defaults.
- MRT layout in deferred passes must be explicitly defined in the shader manifest and PSO key.

## Validation Strategy

For every phase:

- Build with the active CMake preset.
- Run the D3D12 path with debug layer enabled as soon as it exists.
- Capture at least one screenshot for visual comparison.
- Track GPU live objects on shutdown.
- Use PIX after the first non-trivial D3D12 frame to verify barriers, descriptors, render targets, and draw calls.
- Run repeated resize/minimize/restore and focus/alt-tab checks.
- Verify borderless fullscreen enter/exit behavior.
- Verify debug names appear for resources, command lists, queues, and passes in PIX.
- Verify GPU timestamp query results are visible in logs or debug UI.
- Verify unsupported hardware fails with a clear feature-level message.

Suggested milestone checks:

- Clear-only frame.
- Fullscreen shader frame.
- One textured mesh.
- Depth-tested scene.
- Deferred GBuffer visualization.
- Lighting pass parity.
- UI/text parity.
- Full startup smoke test.

## Do Not Do

- Do not build a permanent OpenGL compatibility backend.
- Do not expose `ID3D12Resource*`, descriptor handles, command lists, or fences through gameplay, UI, script, or reflected data structures.
- Do not keep runtime GLSL/JSL compilation in the final D3D12 path.
- Do not replace SFML windowing while still relying on an SFML-created OpenGL context for the active renderer.
- Do not make generated shader byte arrays widely included.
- Do not port all shaders before the pass graph, binding model, and generated shader registry exist.
- Do not preserve OpenGL framebuffer/state-stack behavior where an explicit D3D12 pass/resource model would be clearer.

## Recommended First Implementation Slice

The first code slice should be small but pointed at the final D3D12-only architecture:

1. Add Win32 and D3D12 source groups and link `d3d12`, `dxgi`, `dxguid`, and `user32` without adding a permanent renderer-backend selector. Status: build-validated.
2. Add shader tool discovery for DXC and generate one trivial HLSL shader into a C++ header with deterministic output. Status: build-validated for the bootstrap shader.
3. Create a native Win32 window and D3D12 device/swap chain together. Status: implemented as a non-active smoke path and build-validated.
4. Require feature level 12_1 and log adapter details before creating game/runtime state. Status: adapter selection requires feature level 12_1; startup logging is still pending because the smoke path is not yet wired into runtime flow.
5. Clear/present with debug layer, PIX marker, GPU timestamp query, resize handling, and clean shutdown. Status: clear/present, marker, timestamp query readback, fence wait, and object naming are implemented in the smoke path; runtime execution, repeated resize, debug-layer live-object validation, and device-removal hardening are pending.
6. Keep SFML/OpenGL compiling only until this path becomes active, then start deletion by responsibility.

That slice proves the platform, generated shader pipeline, and D3D12 lifecycle without forcing mesh, texture, render pass, or UI migration prematurely. Once the smoke path is runtime-validated and ready to become the active bootstrap, the next slice should port fullscreen triangle rendering, then a textured mesh.