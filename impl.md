# EarthRise.RTS Implementation Plan

## 1. Goal
Convert the recommendations in improve.md into an executable, staged plan that can be delivered incrementally without destabilizing the current three-target build layout:
- NeuronCore
- NeuronClient
- EarthRise

This plan prioritizes build stability, correctness, and testability first, then migration from custom LTE types toward standard C++ types. D3D12 and renderer migration work is deferred unless it directly blocks the current build or preserves an already-existing boundary.

### Current Implementation Status - April 26, 2026
- Phase 0 is complete for the x64-debug baseline. `build-baseline.md` records the current generator, SDK, dependency baseline, and legacy warning distribution. x64-release remains a follow-up validation item.
- Gate A is satisfied for the default x64-debug workflow.
- Phase 1 test foundation is implemented locally. `NeuronCore.Test`, `NeuronClient.Test`, and `EarthRise.Test` build as Microsoft Native Unit Test DLLs, are registered with CTest, and run through Visual Studio `vstest.console.exe`.
- Gate B is satisfied locally: each active module has Unit and Integration test groups, the test targets build with scoped `/W4 /WX`, and the x64-debug CTest run passes all registered Microsoft Native Unit Test projects.
- `DataReader` and `DataWriter` no longer depend on the missing `NetLib.h` include and now have round-trip unit coverage.
- Phase 2 has started. Tuple equality and Array equality are currently in corrected form and now have regression tests that protect against the planned self-compare and byte-count comparison bugs.
- Phase 2 is complete locally for x64-debug. Release-safe assertion semantics are implemented through `ASSERT_RELEASE` and `ASSERT_RELEASE_TEXT`, routed through `Neuron::Fatal`, and covered by Microsoft Native Unit Tests.
- Gate C is satisfied locally for x64-debug. x64-release remains a follow-up validation item before broad Phase 3 work.
- Phase 3 has started with `phase3-safety-inventory.md`, documenting current `Reference<T>` and type metadata registry thread-safety assumptions before atomic or locking changes are attempted.

## 2. Delivery Principles
- Keep each phase build-safe and shippable.
- Prefer small, reviewable pull requests over large refactors.
- Separate behavior changes from cleanup or style-only changes.
- Require measurable exit criteria for every phase.
- Maintain compatibility with current CMake presets and vcpkg workflow.
- Treat legacy NeuronClient code as migration territory, not greenfield code.
- Add tests before risky behavior changes whenever the affected code can be isolated.
- Prefer documented constraints over speculative rewrites when ownership or thread model is unclear.
- Prioritize custom-to-standard type migration over renderer migration.
- Keep renderer work limited to maintenance, build hygiene, and boundary preservation until the type-modernization gates are complete.

## 3. Analysis Summary and Recommendations

### What Is Strong
- The plan correctly starts with build stability before modernization.
- The phase order keeps broad modernization behind build, test, and safety work.
- The plan respects the current repository shape and does not assume server or tool targets that do not exist.

### Recommended Adjustments
1. Make Phase 0 a hard gate
- Do not begin warning escalation, sanitizers, or broad modernization until the current x64-debug build has a recorded baseline.
- Capture any current graphics SDK or PCH macro issue as a named blocker if it affects the default build during Phase 0.

2. Split warning policy into adoption levels
- Start with warnings-as-errors only for new targets and new files.
- Add legacy NeuronClient warning cleanup by subsystem after tests exist.
- Avoid enabling /WX globally until the legacy warning baseline is known and accepted.

3. Use Microsoft Native Unit Test architecture
- Testing should use the standard Microsoft Native Unit Test framework for C++.
- Create three test projects that mirror the active production targets: `NeuronCore.Test`, `NeuronClient.Test`, and `EarthRise.Test`.
- Each test project should contain both Unit and Integration test groups for its respective module.

4. Move correctness fixes immediately after test scaffolding
- Tuple and Array fixes are small, high-confidence changes.
- They should land as the first test-backed behavior changes after the Microsoft Native Unit Test projects exist.

5. Treat RefCounted atomic conversion as a design decision, not a mechanical edit
- First identify whether the reference system is intended to be thread-safe or thread-confined.
- If thread-safe, use atomic refcounting with documented memory ordering.
- If thread-confined, document the invariant and add debug checks around cross-thread ownership transfer points.

6. Include bulk header guard conversion after tests exist
- Converting roughly 165 LTE headers to `#pragma once` is a planned mechanical modernization task.
- Do it in controlled bulk batches only after the Microsoft Native Unit Test projects exist and the build baseline is stable.
- Keep header guard conversion separate from behavior changes and type migrations.

7. Promote standard type modernization ahead of renderer migration
- The next major modernization track should focus on replacing custom LTE types with standard C++ types where safe.
- Begin with non-reflected implementation-local containers and ownership sites, then move toward public APIs and reflected fields only after metadata support exists.
- Treat `String`, `Vector`, `Array`, `Map`, `Reference`, `Pointer`, and `AutoPtr` as separate migrations with separate risk profiles.

8. Defer D3D12 migration
- D3D12 work should remain non-urgent and should not consume sprint capacity meant for type modernization.
- Keep only build hygiene and source-boundary preservation work that prevents current graphics code from becoming harder to maintain.
- Do not remove OpenGL/SFML dependencies until there is a renewed renderer migration goal.

### Decision Gates
- Gate A: CMake configure and x64-debug build baseline are reproducible.
- Gate B: `NeuronCore.Test`, `NeuronClient.Test`, and `EarthRise.Test` exist, build, and run at least one Microsoft Native Unit Test each.
- Gate C: Critical correctness fixes are covered by regression tests.
- Gate D: Thread-safety contracts are documented before atomic or locking changes land.
- Gate E: A standard-type modernization pilot is complete with tests, rollback notes, and no reflected-field regressions.
- Gate F: Renderer migration is explicitly reactivated before any D3D12 replacement work removes legacy behavior.

## 4. Plan Horizon and Cadence
- Sprint length: 2 weeks
- Planning horizon: 8 to 10 sprints
- Checkpoints:
  - Mid-sprint technical checkpoint
  - End-of-sprint demo and build validation
- Recommended phase policy:
  - One active critical phase at a time.
  - Standard type modernization becomes the primary modernization track after Gate C.
  - Renderer migration should not run in parallel unless it is explicitly reactivated and does not compete with type-modernization work.
  - Modernization batches must name the subsystem, files, and rollback path before implementation.

## 5. Pre-Phase 0 Recommendations

Before starting Phase 0 implementation, treat this as a readiness review. Phase 0 should establish a reliable baseline and remove only the blockers that prevent that baseline from being measured.

### Recommended Entry Rules
- Freeze the Phase 0 scope to build, CMake, include hygiene, PCH policy, shader target behavior, and baseline documentation.
- Do not start correctness fixes, warning escalation, sanitizer presets, test framework work, ownership modernization, or renderer replacement inside Phase 0 unless one of those items directly blocks configure or compile.
- Keep experimental renderer work behind explicit feature flags. The default build should remain usable even if the D3D12 path is incomplete.
- Treat recursive include directories as a temporary compatibility bridge. Every broad include path or exclusion should have a documented reason.
- Preserve the current three-target layout. Do not introduce new server, tools, or game-logic targets as part of Phase 0.
- Do not replace reflected `AutoPtr` or generated reflection ownership with `std::unique_ptr` during this phase; reflected types may require generated copy/assignment behavior before ownership changes are safe.

### Recommended Phase 0 Slices
1. Baseline capture
- Record configure/build results for x64-debug first.
- Record x64-release after Debug is either clean or has one named blocker.
- Capture generator, compiler version, Windows SDK version, vcpkg baseline, enabled cache options, and top-level failure signatures.

2. Build graph stabilization
- Remove stale generated targets from the build graph.
- Ensure missing optional shaders produce status output instead of a hard failure.
- Keep asset staging behavior stable until a replacement target is validated.

3. Header and PCH containment
- Document what may be included from each PCH.
- Prefer local include fixes over adding more broad include directories.

4. Phase 0 report
- End Phase 0 with a short build-baseline note that lists clean targets, blocked targets, known warnings, and next recommended phase.

### Recommended Non-Goals
- No broad LTE header guard conversion.
- No global `/WX` or sanitizer enforcement.
- No SFML removal.
- No renderer backend replacement.
- No large ownership modernization.
- No D3D12 feature work unless it is required to keep the current build graph stable.

### Recommended Phase 0 Success Signal
- A developer can run the documented configure/build path twice and get the same result.
- Any remaining failure has a single owner, a reproduction command, and a next action.
- The default build path is not blocked by missing experimental shader assets.

## 6. Phase Plan

### Phase 0: Build Baseline and Unblockers
Duration: 1 sprint
Priority: Critical

#### Objectives
- Ensure current CMake configuration is stable after recent graphics and shader build changes.
- Remove immediate blockers preventing repeatable local builds.

#### Work Items
1. Build baseline validation
- Validate x64-debug and x64-release configure/build behavior.
- Record current known warnings and failures in a build-baseline note.
- Recommendation: make x64-debug the first required baseline; x64-release can be a follow-up gate if Debug still exposes compile blockers.

2. Shader generation hardening
- Keep bootstrap shader generation optional when source files are missing.
- Ensure no stale generated shader project can break build graph.
- Recommendation: shader targets should be generated from discovered sources, and missing experimental shaders should produce a status message, not a failed build.

3. Include-path and header collision controls
- Preserve recursive include needs while avoiding standard header shadowing.
- Keep collision-prone roots excluded from broad include search.
- Recommendation: document every excluded include root with the specific collision it prevents.

4. PCH and Windows SDK compatibility check
- Verify PCH include order and macro hygiene around Windows and graphics SDK headers.
- Document and enforce required include constraints in NeuronClient pch policy notes.
- Recommendation: add a small preprocessor sanity check for `near`, `far`, and local name leakage before Windows or graphics SDK headers are consumed.

#### Exit Criteria
- CMake configure succeeds cleanly on x64-debug.
- x64-debug build has either a clean result or a single documented blocking failure with owner and reproduction command.
- Build reaches compile stage consistently without stale custom target failures.
- Known blockers are documented and reproducible.
- Gate A is satisfied.

### Phase 1: Quality Gates and Test Foundation
Duration: 1 sprint
Priority: Critical

#### Objectives
- Introduce automated checks to prevent regressions.
- Stand up Microsoft Native Unit Test coverage for each active module.
- Provide separate Unit and Integration test groups for NeuronCore, NeuronClient, and EarthRise.

#### Work Items
1. Add Microsoft Native Unit Test project structure
- Create three C++ Microsoft Native Unit Test projects:
  - `NeuronCore.Test`
  - `NeuronClient.Test`
  - `EarthRise.Test`
- Use the Microsoft C++ Unit Test Framework as the standard test architecture.
- Keep test project naming aligned with the production modules they validate.
- Register the projects so they can run from Visual Studio Test Explorer and the command line build/test workflow.

2. Add unit and integration test layout per module
- Each test project should contain separate Unit and Integration groups.
- Recommended initial layout:
  - `Tests/NeuronCore.Test/Unit`
  - `Tests/NeuronCore.Test/Integration`
  - `Tests/NeuronClient.Test/Unit`
  - `Tests/NeuronClient.Test/Integration`
  - `Tests/EarthRise.Test/Unit`
  - `Tests/EarthRise.Test/Integration`
- Unit tests should isolate logic within the module.
- Integration tests should validate module boundaries, startup paths, asset staging assumptions, and cross-target behavior where practical.

3. Seed initial test coverage
- Add initial `NeuronCore.Test` unit tests for pure utilities:
  - FileSys
  - GameMath
  - EventManager
  - DataReader and DataWriter round-trips
- Add initial `NeuronClient.Test` unit tests for low-risk LTE utility behavior once the test project links successfully.
- Add initial `EarthRise.Test` integration coverage for launcher-level assumptions that can be validated without starting the full game loop.
- Recommendation: keep the first tests small and deterministic; do not pull renderer, audio, or script runtime complexity into the first test slice.

4. Add warning policy
- Enable stricter warning level defaults.
- Treat selected warning classes as errors for new code first.
- Keep legacy exceptions documented where needed.
- Recommendation: start with `/permissive-`, override warnings, and return-type safety before global `/WX`.

5. Add CI entry point
- Add a workflow that runs configure, build, and the Microsoft Native Unit Test projects on x64-debug.
- Recommendation: CI should publish build logs and Microsoft Native Unit Test output as artifacts while the baseline is still moving.

#### Exit Criteria
- `NeuronCore.Test`, `NeuronClient.Test`, and `EarthRise.Test` build in x64-debug.
- Each test project has at least one Unit test and one Integration test registered.
- Microsoft Native Unit Tests run locally and through the command line workflow.
- At least 10 total tests pass across the three test projects.
- Warning policy enforced on new changes.
- Gate B is satisfied.

#### Current Status
- Complete locally for x64-debug.
- The three Microsoft Native Unit Test projects build and are registered as CTest entries.
- Initial coverage includes NeuronCore math helpers, EventManager dispatch behavior, DataReader/DataWriter round trips, NeuronClient Tuple equality, NeuronClient Array equality, and launcher/source tree smoke checks.
- CI wiring remains a process-infrastructure follow-up unless it is pulled forward before Phase 4.

### Phase 2: Critical Correctness Fixes
Duration: 1 sprint
Priority: Critical

#### Objectives
- Fix identified logic and safety defects with tests.

#### Work Items
1. Tuple equality bug fixes
- Correct Tuple3 and Tuple4 equality self-compare bug.
- Add regression tests.
- Recommendation: land this as a small standalone change immediately after the test projects exist.
- Current status: Tuple3 and Tuple4 equality are in corrected form, and regression tests now verify that third and fourth elements participate in equality and inequality checks.

2. Array equality correctness
- Replace byte-count-based comparison with element-wise logic.
- Optional trivial-type fast path only when safe.
- Add unit tests for equality edge cases.
- Recommendation: start with correctness-first `std::equal`; add any memcmp fast path only after tests cover size, empty, non-trivial, and unequal-value cases.
- Current status: Array equality is in corrected element-wise form, and regression tests now cover matching arrays and arrays that differ outside the first byte range.

3. Release-safe assertion path
- Introduce ASSERT_RELEASE for production invariants.
- Route failures through logging path suitable for release diagnostics.
- Recommendation: define the release behavior before adoption: log-and-terminate for impossible invariants, return Fatal for recoverable validation.
- Current status: Complete locally. `ASSERT_RELEASE` and `ASSERT_RELEASE_TEXT` are always-active semantic aliases for release invariants, and `Neuron::Fatal` now preserves formatted failure text, writes it to the debugger output channel, breaks only when a debugger is attached, and throws `std::runtime_error`.

4. DataReader and DataWriter regression coverage
- Keep the `NetLib.h` dependency removal covered by compile and behavior tests.
- Add round-trip tests for primitive values, strings, and trivially copyable arrays.
- Current status: Complete locally in `NeuronCore.Test`.

#### Exit Criteria
- All correctness fixes covered by tests.
- No behavior regressions in existing debug smoke run.
- Gate C is satisfied.

#### Current Status
- Complete locally for x64-debug.
- Regression coverage now protects Tuple3/Tuple4 equality, Array equality, DataReader/DataWriter round trips, and release assertion behavior.
- x64-debug build and all registered Microsoft Native Unit Test projects pass after the Phase 2 changes.
- x64-release validation remains pending.

### Phase 3: Thread and Memory Safety Hardening
Duration: 1 to 2 sprints
Priority: High

#### Objectives
- Reduce concurrency risk and unsafe ownership behavior in critical paths.

#### Current Status
- Started. `phase3-safety-inventory.md` records the current intrusive reference counting and lazy type registry risks.
- Current recommendation is to treat `Reference<T>` and type metadata registration as thread-confined/startup-only until cross-thread ownership and runtime registration requirements are proven.
- No atomic or locking behavior has been changed yet.

#### Work Items
1. Reference counting safety
- Migrate RefCounted refCount to atomic or formally document single-threaded contract.
- Add stress tests for concurrent increment and decrement scenarios where applicable.
- Recommendation: begin with a call-site inventory. Do not change memory ordering until ownership transfer points are known.

2. Type registry synchronization
- Add mutex protection in dynamic type registration paths.
- Document thread-safety contract for registration lifecycle.
- Recommendation: prefer startup-only registration if that matches runtime behavior; locking every lookup should be avoided unless registration can happen after startup.

3. Event and async ordering audit
- Verify EventManager and ASyncLoader lock and atomic ordering invariants.
- Remove unclear volatile usage where redundant.
- Recommendation: add comments only for ordering guarantees that are not obvious from the code.

4. Pointer and null-safety cleanup
- Add nodiscard where applicable and tighten access paths.
- Document intended use boundaries for legacy pointer wrappers.
- Recommendation: keep Pointer and reflection-facing wrappers stable; improve new code call sites first.

#### Exit Criteria
- Concurrency-critical files have explicit synchronization contracts.
- New thread-safety tests pass.
- No data races observed in targeted stress runs.
- Gate D is satisfied.

### Phase 4: Build and Process Infrastructure
Duration: 1 sprint
Priority: High

#### Objectives
- Improve build reproducibility, contributor onboarding, and project governance.

#### Work Items
1. CMake and presets
- Add sanitizer presets where practical.
- Keep renderer-related CMake options documented but avoid expanding backend selection work unless it is needed for build stability.
- Recommendation: sanitizer presets should be opt-in and documented; do not make them required for every developer until dependency compatibility is proven.

2. Asset staging workflow
- Move GameData staging from ad hoc post-build behavior to explicit target or install workflow.
- Recommendation: keep the current post-build copy until the explicit asset target is validated, then remove the old path in a separate change.

3. Documentation set
- Add README with quick start and target map.
- Add ARCHITECTURE overview document.
- Refactor coding standards into mandatory rules, modernization guidance, and legacy exceptions.
- Recommendation: document current reality first. Mark D3D12 as deferred roadmap direction, and make custom-to-standard type modernization the active modernization theme.

4. TODO governance
- Standardize TODO format with issue IDs.
- Add optional reporting in CI.

5. vcpkg reproducibility
- Document baseline and expected setup.
- Add vcpkg configuration for dependency pinning policy.
- Recommendation: pin and document the vcpkg baseline before adding optional packages.

#### Exit Criteria
- New contributor can build from README instructions.
- Build and dependency policies are documented and repeatable.

### Phase 5: Standard Type Modernization Track
Duration: 2 to 3 sprints
Priority: High

#### Objectives
- Reduce dependency on custom LTE types in non-reflected, implementation-local code.
- Build a repeatable migration pattern from custom types to standard C++ types without breaking reflection, serialization, scripting, or generated metadata.

#### Work Items
1. Type migration inventory
- Categorize `String`, `Vector`, `Array`, `Map`, `HashMap`, `HashSet`, `Stack`, `Pointer`, `Reference`, `AutoPtr`, `Tuple`, and related LTE wrappers by usage.
- Separate reflected fields, script-visible APIs, serialization paths, and implementation-local usage.
- Recommendation: maintain a migration table with status, owning subsystem, test coverage, and blockers.

2. Compatibility API cleanup
- Prefer standard-like APIs such as `.get()`, `.reset()`, `begin()`, `end()`, `data()`, `empty()`, and explicit range access before changing storage types.
- Replace direct `.t`, `.v`, `.buffer`, `release()`, and `deleteElements()` usage in small batches.
- Recommendation: shrink custom-only call sites before replacing wrappers.

3. Non-reflected standard container migration
- Convert local, non-reflected `Vector`, `HashMap`, `HashSet`, `Map`, and `Array` usage to `std::vector`, `std::unordered_map`, `std::unordered_set`, `std::map`, `std::span`, or `std::unique_ptr<T[]>` where semantics match.
- Keep reflected fields on LTE wrappers until metadata support or adapters exist.
- Recommendation: start with low-risk implementation files that do not cross script, serialization, or generated metadata boundaries.

4. Ownership modernization
- Migrate implementation-local `AutoPtr` ownership to `std::unique_ptr` where move-only semantics are acceptable.
- Keep `Reference<T>` and reflected `AutoPtr` APIs stable until ownership and reflection contracts are redesigned.
- Recommendation: do not replace copyable reflected `AutoPtr` fields with `std::unique_ptr` unless generated copy/assignment behavior is available.

5. API quality improvements
- Add override, const-correctness, nodiscard, noexcept in targeted modules.
- Avoid sweeping signature changes across entire codebase in one pass.
- Recommendation: prioritize `override` and `[[nodiscard]]` before const-signature changes that can ripple across call sites.

6. Bulk header guard conversion
- Convert legacy include guards to `#pragma once` in controlled bulk batches.
- Start with `NeuronClient/LTE` headers after the test projects exist and the build baseline is stable.
- Keep each batch mechanical: header guard removal only, no type migration, formatting churn, or behavior changes.
- Recommendation: batch by folder, verify each batch with the Microsoft Native Unit Test projects, and stop if warning or build trends regress.

#### Exit Criteria
- At least one standard-type migration pilot is complete in a non-reflected subsystem.
- Migration batches compile cleanly and preserve behavior.
- Reflection and serialization boundaries are documented for every custom type touched.
- Build time and warning trend do not regress.
- Gate E is satisfied.

### Phase 6: Deferred Renderer Boundary Maintenance
Duration: Deferred until explicitly reactivated
Priority: Low

#### Objectives
- Keep the current rendering path maintainable while type modernization is the active priority.
- Preserve useful renderer boundaries without committing sprint capacity to D3D12 migration.
- Avoid making OpenGL/SFML coupling worse, but do not treat replacement as urgent.

#### Work Items
1. Boundary preservation
- Keep graphics-specific includes localized.
- Avoid spreading OpenGL, SFML, Windows, or D3D headers into unrelated core and gameplay code.
- Recommendation: make small containment fixes only when they reduce build risk or dependency spread.

2. Current renderer health
- Fix defects in the active renderer path when needed.
- Keep shader and asset build rules resilient to missing experimental files.
- Recommendation: favor maintenance and diagnostics over replacement work.

3. Dependency inventory
- Maintain a lightweight inventory of SFML/OpenGL usage by subsystem.
- Record future replacement candidates without scheduling them as active work.
- Recommendation: this inventory supports future renderer planning without changing current priorities.

4. Reactivation checklist
- Require an explicit roadmap decision before starting D3D12 replacement work.
- Require build baseline, tests, critical correctness fixes, and standard-type migration pilot to be complete first.
- Recommendation: do not remove SFML from dependencies until replacement functionality is implemented and validated.

#### Exit Criteria
- Renderer-related changes remain maintenance-only unless Gate F is explicitly opened.
- Current build and active renderer behavior are not regressed by modernization work.

## 7. Recommendation Backlog

### Before Phase 0 Starts
- Create the Phase 0 issue list and assign owners for build baseline, shader target behavior, include/PCH hygiene, and baseline reporting.
- Confirm that Phase 0 is responsible for the default build path first; D3D12-enabled failures may remain documented optional blockers unless they break the default build.
- Prepare the build-baseline note template so Phase 0 captures consistent data.
- Confirm that missing experimental shader assets are treated as optional, not release blockers.

### Start During Phase 0
- Record the current build baseline and any outstanding graphics SDK/PCH blocker.
- Document PCH and SDK header rules before more graphics SDK includes spread.
- Keep all changes limited to build graph, include hygiene, shader generation, asset staging safety, and baseline documentation.

### Start After Gate A
- Add `NeuronCore.Test`, `NeuronClient.Test`, and `EarthRise.Test` using Microsoft Native Unit Test architecture.
- Add Unit and Integration groups to each test project.
- Fix Tuple equality and Array equality with tests after the test projects land.
- Begin CI wiring for configure, build, and Microsoft Native Unit Test execution.

### Start After Gate B
- Add warning adoption levels.
- Add release assertion semantics.
- Audit RefCounted and Type registry threading contracts.
- Add sanitizer presets as opt-in configurations.

### Start After Gate C or D
- Modernize selected ownership sites.
- Run bulk header guard conversion in controlled mechanical batches.
- Split generated reflection metadata.
- Route script errors through the logger.

### Start After Gate E
- Continue custom-to-standard migration by subsystem.
- Evaluate reflected container metadata support before changing reflected fields.
- Consider renderer maintenance only when it preserves current behavior or reduces dependency spread.

### Defer
- Global `/WX` on all legacy code.
- Removing SFML from dependencies.
- Replacing the current renderer path.

## 8. Workstream Mapping to Recommendations
- Architecture and build: Improve sections 1 and 8
- Memory and safety: Improve sections 2 and 5
- Code quality: Improve section 3
- Testing and validation: Improve section 4
- Rendering and assets: Improve section 6
- Scripting and reflection: Improve section 7
- Documentation and process: Improve section 9
- Immediate bug fixes: Improve section 10

## 9. Suggested Sprint Sequence
- Sprint 1: Phase 0
- Sprint 2: Phase 1
- Sprint 3: Phase 2
- Sprint 4 to 5: Phase 3
- Sprint 6: Phase 4
- Sprint 7 to 9: Phase 5
- Sprint 10 and onward: continue standard-type migration by subsystem; Phase 6 remains deferred until explicitly reactivated
- Recommendation: if Phase 0 does not produce a usable build baseline, extend Phase 0 rather than starting modernization work.

## 10. Risk Register and Mitigations
1. Legacy macro interactions break SDK headers
- Mitigation: isolate Windows and graphics SDK include boundaries, add preprocessor sanity checks in CI.

2. Broad refactors destabilize gameplay code
- Mitigation: refactor in scoped batches with tests and per-batch feature freeze.

3. Warning-as-Fatal blocks progress in legacy modules
- Mitigation: use targeted suppression list with sunset dates and owners.

4. Renderer work distracts from the active type-modernization priority
- Mitigation: keep renderer changes maintenance-only and require Gate F before replacement or D3D12 migration work starts.

5. Test adoption slows due to framework setup overhead
- Mitigation: start with NeuronCore pure utility tests and expand outward.

6. Atomic refcounting changes object lifetime behavior
- Mitigation: inventory ownership transfer paths first, add focused stress tests, and land memory-order changes separately from API cleanup.

7. Sanitizer and warning work consumes the whole sprint
- Mitigation: define a fixed warning budget per sprint and track remaining warnings as backlog, not blockers.

## 11. Definition of Done for This Plan
This implementation plan is considered active when:
- It is tracked in issues or milestones by phase.
- Each phase has owner, estimate, and acceptance checks.
- CI reflects at least configure, build, and tests for x64-debug.
- Progress is reviewed every sprint against phase exit criteria.
- Decision gates A through E are tracked explicitly in milestones.
- Gate F remains closed unless the project explicitly reactivates renderer migration.

## 12. Immediate Next Actions
1. Run x64-release validation for the completed Phase 0 through Phase 2 work.
2. Begin Phase 3 with a RefCounted ownership-transfer inventory before changing atomic or locking behavior.
3. Document whether reference counting is thread-confined or thread-safe before implementation.
4. Identify Type registry registration timing and decide whether synchronization belongs only on startup mutation paths or on runtime lookup as well.
5. Add focused stress tests only after the ownership and registration contracts are documented.
6. Keep renderer and D3D12 work deferred while Phase 3 safety work begins.
