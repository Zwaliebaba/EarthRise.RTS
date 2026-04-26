# LTE Type Modernization Plan

## Purpose

`NeuronClient/LTE` contains many custom foundational types that overlap with modern C++20/23 standard library facilities: `String`, `Array`, `Vector`, `Map`, `HashMap`, `HashSet`, `Pointer`, `Reference`, `AutoPtr`, `Tuple`, `Stack`, `RingBuffer`, `StringList`, `StringTree`, and the reflection-owned `Data`/`Type` layer.

Modernizing these types can reduce custom maintenance, improve tool support, make ownership easier to reason about, and expose more code to standard algorithms and containers. The migration should not be a broad mechanical replacement. Several LTE types are part of the scripting, reflection, serialization, pooling, and intrusive ownership model, so direct replacement would break behavior even when the surface looks STL-like.

This plan is intentionally staged. It starts with compatibility work and low-risk wrappers, then moves toward standard library usage at subsystem boundaries, and only later tackles the reflection and ownership core.

## Implementation Status

Initial low-risk recommendations implemented:

- Added standard-like `.get()` and `.reset()` helpers to `Pointer<T>`, `Reference<T>`, and `AutoPtr<T>`; `AutoPtr<T>` also now has `release()` for explicit ownership transfer.
- Added matching `.get()` and `.reset()` helpers to `Type` and used `.get()` in pointer-like streaming helpers.
- Updated LTE comparison helpers for `AutoPtr` / `Pointer` / `Reference` to use `.get()` internally instead of direct `.t` access.
- Added `Vector<T>::AsStdVector()` accessors while keeping the existing wrapper and reflected behavior intact.
- Replaced active direct `Vector::v` sorting call sites with public `begin()` / `end()` usage in `Graph` and `Market` code.
- Removed obsolete `std::tr1` fallback branches from `HashMap` and `HashSet`; both now use `<unordered_map>` / `<unordered_set>` directly.
- Converted non-reflected `HashMap` / `HashSet` use sites to `std::unordered_map` / `std::unordered_set` in physics, economy, font kerning, profiling, shader instance state, and type traversal code.
- Kept the reflected `WidgetDynamic::childrenMap` field on `HashMap` until reflected `std::unordered_map` support is designed.
- Replaced straightforward implementation-level `.t` pointer reads with `.get()` in function/font creation, collision mesh casts, string-list rewriting/printing, expression list handling, particle systems, scripted thread results, texture generation, and window viewport code.
- Extended the `.get()` cleanup into gameplay, component, UI script, and render-pass call sites, including generated object component accessors, while leaving non-pointer data fields and wrapper internals unchanged.
- Replaced the lone active `RingBuffer<T>` use in `ObjectType_Trail` with local `std::vector<SegmentData>` storage plus an explicit current index; `RingBuffer.h` remains for compatibility/reflection until removal is safe.
- Fixed `Array<T>` equality to compare elements instead of `size()` raw bytes, added `Array<T>::empty()`, and added standard-style range/`AsStdVector()` accessors to `Stack<T>`.
- Fixed `Tuple3` and `Tuple4` comparison bugs where `z` was compared against itself instead of the other tuple.
- Started Phase 2 local ownership cleanup by replacing implementation-local `AutoPtr` ownership with `std::unique_ptr` in region graph generation, XAudio2 wave loading, SMAA texture data loading, texture file loading, location ASCII reads, serializer staging buffers, and collision mesh acceleration structures.
- Replaced `DiffImpl`'s manually deleted `Vector<DiffBlock*>` storage with `std::vector<std::unique_ptr<DiffBlock>>`, removing the matching `deleteElements()` call site.
- Continued Phase 2 by replacing private `AutoPtr` storage with `std::unique_ptr` in patch application state, query spatial partitions, and thread ownership; active `deleteElements()` usage is now gone outside the compatibility wrappers themselves.

Still intentionally deferred:

- Broad replacement of `String`, `Vector`, `Array`, `Map`, `Reference`, or `Pointer` with standard types.
- Reflected field migration to standard containers.
- Broad ownership rewrites from `AutoPtr`/`Reference` to `std::unique_ptr` or `std::shared_ptr`.
- Serialization and script metadata changes.
- Remaining `.t` access inside wrapper internals, `Type`, list-link plumbing, and raw tuple vertex fields where `.get()` would not be equivalent or the type has no accessor yet.
- Removing `RingBuffer.h`, rewriting reflected `Stack<T>`/`Array<T>` fields, migrating public `AutoPtr` APIs such as `Location::Read` and `Diff_Create`, or changing `Array<T>` buffer transfer semantics.

## Phase 5 Current Inventory - April 26, 2026

Phase 5 has moved from inventory into implementation-local pilot migrations. Reflected fields, script-visible APIs, serialization shapes, and intrusive ownership semantics remain unchanged.

| Type family | Current boundary | Migration status | Primary blockers | Recommended next slice |
|---|---|---|---|---|
| `String` | Reflection, script APIs, utilities, broad gameplay/runtime use | Deferred for broad replacement | Metadata, implicit `char const*`, utility surface, very high usage | Add `std::string_view` overloads only in pure helper code. |
| `Vector<T>` | Reflection, script member functions, local containers | First private UI stack pilot complete | Reflected fields, helper semantics, direct `.v` history | Continue with non-reflected local temporaries after confirming no `Type_Get` or script path. |
| `Array<T>` | Reflected buffers, raw owned storage, file/audio/graphics buffers | Correctness fixed; migration deferred | `release()`/`replace()`, raw `buffer`, serialized shape | Audit byte-buffer users before selecting a `std::vector` or `std::span` pilot. |
| `Map<K, V>` | Reflected ordered maps and local maps | Deferred for broad replacement | Deterministic order, `_ToStream`, metadata | Add/standardize `FindPtr` helper before local map conversions. |
| `HashMap<K, V>` / `HashSet<T>` | Mostly STL-backed wrappers | Partially migrated in non-reflected sites | Reflected `HashMap` fields such as UI widget maps | Continue converting implementation-local uses only. |
| `Stack<T>` | Small STL-backed wrapper with implicit top conversion | First simple local pilot complete in profiler code | Implicit conversion and arithmetic push behavior in broader use | Continue converting only simple local stacks using `push`, `pop`, `back`, and `size`. |
| `Tuple2/3/4` | AutoClass-reflected named values and local tuple-like values | Equality bugs fixed | Reflected field names and generated metadata | Convert non-reflected local tuple-like temporaries to named structs or standard tuples. |
| `AutoPtr<T>` | Legacy ownership wrapper and reflected ownership fields | Local ownership cleanup partially done | Transfer-on-copy, reflected fields, public APIs | Continue implementation-local `std::unique_ptr` conversions; avoid reflected fields. |
| `Reference<T>` | Intrusive shared ownership across engine objects/resources/scripts | Contract documented as thread-confined | Intrusive allocation, public `.t`, implicit raw conversion, thread policy | Do not replace; reduce external `.t` and resolve Phase 3 ownership contracts first. |
| `Pointer<T>` | Non-owning reflected pointer wrapper | Compatibility APIs partially in place | Lifetime is external, reflection metadata, comparison with `Reference` | Keep wrapper stable; convert only obvious local observer variables later. |
| `RingBuffer<T>` | Compatibility/reflection type; no active production use identified in prior pass | Active use migrated; header retained | Possible metadata/type-name dependency | Leave for a later dead-code/removal verification pass. |

Recommended Phase 5 pilot: continue with implementation-local `HashMap`/`HashSet`, `AutoPtr`, simple `Stack<T>`, or local `Vector<T>` sites before touching `String`, `Array`, `Reference`, public APIs, or reflected fields. A pilot must name the subsystem, files, test coverage, and rollback path before code changes land.

### Pilot 1: Profiler Local Stack

- Subsystem: `NeuronClient/LTE` profiler internals.
- File: `NeuronClient/LTE/Profiler.cpp`.
- Change: replaced the private `Stack<char const*> segments` member with `std::vector<char const*>` and updated the local `StackFrame` snapshot constructor plus push/pop calls.
- Boundary check: this stack is private implementation state, not a reflected field, public API, script-visible type, or serialized shape.
- Rollback path: restore the `Stack.h` include, `Stack<char const*>` member, `StackFrame(Stack<char const*> const&)`, and `push`/`pop` calls.
- Validation: built `NeuronClient.Test` and ran the registered `NeuronClient.Test` CTest entry successfully after the change.

### Pilot 2: UI Local Vector Stacks

- Subsystem: `NeuronClient/UI` cursor and clip-region internals.
- Files: `NeuronClient/UI/Cursor.cpp` and `NeuronClient/UI/ClipRegion.cpp`.
- Change: replaced private static `Vector<Cursor>` and `Vector<ClipRegion>` stack storage with `std::vector` and updated local push/pop calls to `push_back`/`pop_back`.
- Boundary check: the `Cursor` and `ClipRegion` value types remain `AutoClass` reflected types; only `.cpp`-local stack containers changed, with no public API, script-visible signature, serialization shape, or reflected field change.
- Rollback path: restore the `LTE/Vector.h` includes, `Vector<Cursor>`/`Vector<ClipRegion>` stack return types and statics, and `push`/`pop` calls.
- Validation: built `NeuronClient.Test` and ran the registered `NeuronClient.Test` CTest entry successfully after the change.

## Search Summary

Audit scope:

- `NeuronClient/LTE` type definitions.
- Usage across `NeuronClient`, excluding `ThirdParty` and `UTF8`.
- Direct internal access such as `.t`, `.v`, `.buffer`, `.release()`, and reflection macros.

`rg` was not available in the current PowerShell environment, so the audit used VS Code search plus PowerShell `Select-String`.

Approximate usage counts across `NeuronClient`:

| Type | Approximate matches | Notes |
|---|---:|---|
| `String` | 975 | Mostly LTE, Game, Module; inherits `std::string` but adds metadata and implicit `char const*` conversion. |
| `Vector<T>` | 540 | Wrapper around `std::vector<T>` with reflection, script functions, extra helpers, and direct `.v` use. |
| `Array<T>` | 201 | Owning fixed-size dynamic array with reflection and transfer helpers; not equivalent to `std::array`. |
| `Reference<T>` | 201 | Intrusive reference-counted owner. Not directly equivalent to `std::shared_ptr`. |
| `StringList` | 141 | Ref-counted parse tree/list abstraction, not just a string container. |
| `Pointer<T>` | 107 | Non-owning wrapper with public `.t` and debug null checks. |
| `Map<K, V>` | 92 | Wrapper around `std::map` with reflection and `get()` pointer-return helper. |
| `AutoPtr<T>` | 59 | Legacy transfer-on-copy owner, closest to pre-C++11 `std::auto_ptr`; migrate carefully to `std::unique_ptr`. |
| `StringTree` | 26 | Ref-counted tree for parsed config/script-ish data. |
| `Stack<T>` | 16 | Small stack wrapper over `std::vector` with implicit top conversion and max-size asserts. |
| `HashSet<T>` | 10 | Thin wrapper over `std::unordered_set` on Windows/macOS. |
| `VectorMap<K, V>` | 8 | Small linear map preserving vector-like storage behavior. |
| `Tuple2`, `Tuple3` | 12 | AutoClass-reflected structs with named fields. Some uses can become `std::pair`/`std::tuple`; reflected uses cannot. |
| `HashMap<K, V>` | 5 | Thin wrapper over `std::unordered_map` on Windows/macOS. |
| `RingBuffer<T>` | 1 | `Array<T>` subclass with current-index semantics. |

Important migration blockers found by search:

| Pattern | Approximate matches | Why it matters |
|---|---:|---|
| `.t` pointer field access | 135 | Many callers reach through `Pointer`, `Reference`, or `AutoPtr` internals directly. A drop-in smart pointer replacement would not compile. |
| `.v` vector backing access | 17 | Some code sorts or manipulates the backing `std::vector` directly. A replacement must expose equivalent range APIs first. |
| `.buffer` access | 15 | `Array` users sometimes expect raw mutable contiguous storage. |
| `.release()` / `.replace()` | 7 | `Array` transfer semantics differ from `std::vector`. |
| `deleteElements()` | 3 | Some containers own raw pointers by convention rather than by type. |
| Reflection macros and metadata hooks | 139 | `FIELDS`, `MapFields`, `DeclareMetadata`, `DefineMetadata`, and `AUTOMATIC_REFLECTION` make several wrappers visible to the runtime type system. |

Subsystem concentration:

- `NeuronClient/LTE` dominates definitions and internal usage.
- `Game` is the largest non-LTE user of `String`, `Vector`, `Pointer`, and `Reference`.
- `Component`, `UI`, and `Module` have smaller but meaningful usage.
- `StringList` and `StringTree` usage is concentrated in LTE and should be treated as parser/runtime infrastructure, not ordinary container code.

## Key Design Constraints

### Reflection and serialization are first-class

`Type.h`, `Common.h`, `AutoClass.h`, and generated metadata code use `FIELDS`, `MapFields`, `Type_Get<T>()`, `_ToStream`, and `AUTOMATIC_REFLECTION_PARAMETRIC*`. Containers such as `Array`, `Vector`, `Map`, `VectorMap`, `RingBuffer`, `AutoPtr`, and `Data` participate in that system.

Replacing a reflected LTE type with a standard type requires one of these first:

- metadata support for the relevant standard type,
- an adapter wrapper that preserves the old reflected type name and field layout semantics, or
- a deliberate decision that the migrated field is no longer serialized or script-visible.

Without that groundwork, direct use of `std::vector`, `std::map`, `std::string`, `std::unique_ptr`, or `std::shared_ptr` can break save/load, script APIs, debug UI, generated function metadata, or runtime `Data` conversion.

### Ownership is not standard smart-pointer ownership yet

`Reference<T>` is intrusive reference counting over `RefCounted`. The pointee stores `uint refCount`, callers often pass references as raw pointers through implicit conversions, and `.t` is public. This is different from `std::shared_ptr<T>` in allocation model, control block layout, aliasing, weak references, thread safety, and object lifetime assumptions.

`Pointer<T>` is a non-owning wrapper. Modern C++23 has no standard `observer_ptr`; raw `T*` or a local observer wrapper are the realistic choices.

`AutoPtr<T>` has transfer-on-copy behavior. It should be treated as technical debt and replaced with `std::unique_ptr<T>` only in local scopes or APIs where move-only semantics can be introduced safely.

### Many wrappers are already STL-backed

Several LTE wrappers are already thin layers over standard containers:

- `String` inherits `std::string`.
- `Vector<T>` contains `std::vector<T> v`.
- `Map<K, V>` contains `std::map<K, V> m`.
- `HashMap` and `HashSet` derive from `std::unordered_map` / `std::unordered_set` on Windows and macOS.
- `Stack<T>` contains `std::vector<T> elements`.

This makes a compatibility-first strategy attractive: add standard-compatible APIs and reduce custom-only call sites before changing storage types.

## Type-by-Type Recommendations

### `String`

Current behavior:

- Inherits from `std::string`.
- Adds `contains`, `containsOnly`, `pop`, `front`, `back`, `ToString`, `FromString`, hashing, and many string utility functions.
- Provides implicit `operator char const*()`.
- Has runtime metadata via `MapFields` and `DeclareMetadata`.

Modern replacement target:

- Prefer `std::string` for new non-reflected code.
- Prefer `std::string_view` for read-only parameters that do not require ownership or null termination.
- Keep `LTE::String` at reflected/script boundaries until metadata and conversion support is explicit.

Recommended migration:

1. Add helper overloads that accept `std::string_view` for pure algorithms such as path normalization, case conversion, and contains checks.
2. Introduce free functions over `std::string_view` rather than adding more methods to `LTE::String`.
3. Replace local variables in non-reflected implementation files where no `Type_Get<String>()`, `Data`, script call, or `Location` API is involved.
4. Avoid changing public APIs that currently expose `String` until call sites are categorized.
5. Remove or quarantine implicit `char const*` dependencies last. That conversion hides lifetime and overload issues.

Do not start by replacing `String` aliases globally. It is the most widely used custom type and is tied into reflection.

### `Vector<T>`

Current behavior:

- Wraps `std::vector<T>` in public member `v`.
- Adds `append`, `push`, `operator<<`, `operator>>`, `contains`, `erase`, `remove`, `removeIndex` with swap-pop behavior, `random`, `shuffle`, `getModulo`, `deleteElements`, and reflection/script metadata.
- Some code uses `.v` directly for `std::sort` or lower-level manipulation.

Modern replacement target:

- Use `std::vector<T>` in new non-reflected code.
- Keep `Vector<T>` for reflected fields, script-visible collections, and APIs that rely on old helper names.

Recommended migration:

1. Add compatibility helpers around standard algorithms: `Contains`, `EraseValue`, `EraseSwap`, `RandomElement`, and `Shuffle` that work with `std::vector` and `Vector`.
2. Replace `.v` direct access with `begin()`, `end()`, `data()`, or explicit `AsStdVector()` accessors. This shrinks the future blast radius.
3. Replace `deleteElements()` call sites with `std::vector<std::unique_ptr<T>>` where ownership is real and local.
4. Convert local, non-reflected temporaries from `Vector<T>` to `std::vector<T>` after the helper APIs exist.
5. Delay reflected member fields until `Type_Get<std::vector<T>>()` or a serialization adapter is implemented.

High-value first slices:

- Files that only create a local `Vector<T>` and iterate/push it.
- Sorting call sites currently reaching into `.v`.
- Raw-pointer vectors with `deleteElements()`.

### `Array<T>`

Current behavior:

- Owns a raw `T* buffer` plus `_size`.
- Represents a heap array with runtime-sized fixed length.
- `resize()` destroys and reallocates instead of preserving existing elements.
- `release()` and `replace()` transfer the raw buffer.
- Reflection maps `size` plus repeated `data` entries.
- Equality uses `memcmp(one.data(), two.data(), one.size())`, which appears byte-count incorrect for non-byte element types and unsafe for non-trivial types.

Modern replacement target:

- `std::vector<T>` for resizable owned buffers.
- `std::unique_ptr<T[]>` plus size for fixed-size ownership where preserving the old reallocate-on-resize semantics matters.
- `std::span<T>` for non-owning buffer views.
- `std::array<T, N>` only for compile-time fixed size, not as an `Array<T>` replacement.

Recommended migration:

1. Audit `Array<T>` instantiations by element type. Separate byte/sample buffers from reflected structured arrays.
2. Replace non-reflected byte buffers with `std::vector<std::byte>` or `std::vector<unsigned char>`.
3. Replace API parameters that only read buffers with `std::span<T const>`.
4. Replace `release()` / `replace()` use with move semantics in local code.
5. Fix or quarantine `operator==` before relying on it during migration.
6. Keep reflected `Array<T>` fields until serialization support exists for the replacement.

### `Map<K, V>`

Current behavior:

- Wraps `std::map<K, V>` in public member `m`.
- Adds `contains`, `get()` returning pointer or null, `operator()` as lookup, random iterator selection, `_ToStream`, and reflection.

Modern replacement target:

- `std::map<K, V>` when ordering is required.
- `std::unordered_map<K, V>` for hash lookup when ordering is not required and hash support exists.

Recommended migration:

1. Add generic helpers: `MapContains`, `FindPtr`, and `FindPtrConst` for standard map/unordered_map.
2. Replace local maps that are not reflected and do not need `_ToStream` with `std::map` or `std::unordered_map`.
3. Preserve `Map<K, V>` for reflected fields until `Type_Get<std::map<K, V>>()` exists or those fields are explicitly excluded from reflection.
4. Avoid changing ordered map behavior to unordered behavior unless deterministic iteration is not observable.

### `HashMap<K, V>` and `HashSet<K>`

Current behavior:

- Already alias/derive from `std::unordered_map` and `std::unordered_set` on Windows and macOS.
- Retains old `std::tr1` fallback for non-Windows/non-macOS.
- Adds `contains`, `get`, and `_ToStream` for `HashMap`.

Modern replacement target:

- `std::unordered_map` and `std::unordered_set` directly.

Recommended migration:

1. Remove the obsolete `std::tr1` fallback once supported platforms are confirmed as modern Windows/CMake only.
2. Add free `FindPtr` helpers for `std::unordered_map`.
3. Convert the small number of `HashMap`/`HashSet` users first. These are among the lowest-risk container migrations.
4. Keep `_ToStream` behavior only where diagnostics depend on it.

### `VectorMap<K, V>`

Current behavior:

- Stores entries in `Vector<VectorMapEntry<K, V>>`.
- Provides linear lookup, index-based key/value access, and swap-pop removal.
- Is reflected.

Modern replacement target:

- `std::vector<std::pair<K, V>>` for small ordered/linear maps.
- `std::unordered_map<K, V>` for true lookup-heavy maps.

Recommended migration:

1. Treat each use by semantics, not by name.
2. If index order or compact storage matters, migrate to `std::vector<std::pair<K, V>>` plus helper functions.
3. If lookup dominates and order does not matter, migrate to `std::unordered_map`.
4. Preserve reflected instances until metadata support is decided.

### `Tuple2`, `Tuple3`, `Tuple4`

Current behavior:

- Generated via `AutoClass`, so fields `x`, `y`, `z`, and `w` are reflected and named.
- Provide conversions and comparison operators.
- `Tuple3` and `Tuple4` equality/inequality contain apparent self-comparison bugs for `z` (`a.z == a.z`, `a.z != a.z`).

Modern replacement target:

- `std::pair` for two-value non-reflected internal values.
- `std::tuple` for tuple-like internal values.
- Named structs for public/reflected values where field names matter.

Recommended migration:

1. Fix the `Tuple3`/`Tuple4` comparison bug separately if tests or current behavior depend on tuple comparison.
2. Replace non-reflected local uses with `std::pair` or `std::tuple`.
3. Prefer small named structs over `std::tuple` where code readability or reflection field names matter.
4. Do not replace reflected `Tuple*` uses with `std::tuple` until `Type_Get<std::tuple<...>>()` and field naming policy exist.

### `AutoPtr<T>`

Current behavior:

- Owns a raw pointer.
- Copy construction and assignment steal from the source by mutating a `const&`, like old `std::auto_ptr`.
- Has public `.t`, implicit `T*`, debug null checks, reflection, and comparison helpers with `Pointer`/`Reference`.

Modern replacement target:

- `std::unique_ptr<T>`.

Recommended migration:

1. Find local `AutoPtr` variables that do not cross reflected/script boundaries.
2. Convert them to `std::unique_ptr<T>` with explicit `std::move`.
3. Replace `.t` uses with `.get()` before changing types.
4. Avoid changing APIs that accept or return `AutoPtr<T>` until ownership semantics are documented.
5. Do not implement transfer-on-copy compatibility for new code; make ownership moves explicit.

### `Reference<T>`

Current behavior:

- Intrusive reference-counted owner for `RefCounted` objects.
- Public `.t`, implicit pointer conversion, debug null checks, comparison helpers, reflection metadata, and pointee type metadata.
- Used by object handles, engine resources, scripts, UI, tasks, and components.

Modern replacement target:

- Long-term candidate: `std::shared_ptr<T>` or a modernized intrusive pointer.
- Near-term: keep `Reference<T>` but reduce implicit and `.t` dependencies.

Recommended migration:

1. Do not replace globally. This is a core object model change.
2. Add explicit `get()`, `reset()`, and possibly `operator==` helpers matching standard smart pointer habits.
3. Replace external `.t` access with `.get()` in low-risk files.
4. Audit whether `refCount` needs atomics or remains single-thread/game-thread only.
5. Consider a local `IntrusivePtr<T>` implementation with standard-like API as a bridge, preserving intrusive allocation and existing `RefCounted` layout.
6. Only evaluate `std::shared_ptr` after object creation/destruction, pooling, serialization, and script handles are mapped.

### `Pointer<T>`

Current behavior:

- Non-owning pointer wrapper with public `.t`, implicit pointer conversion, debug null checks, and reflection metadata.
- Used heavily in `Game` and `Component` APIs.

Modern replacement target:

- Raw `T*` for optional non-owning references.
- `T&` for required non-owning references.
- A local `ObserverPtr<T>` wrapper only if debug null checks and reflected pointer metadata still matter.

Recommended migration:

1. Do not replace before `Reference<T>` handling is clearer; they compare to each other through helper headers.
2. Add `.get()` and start replacing direct `.t` access.
3. Convert local non-reflected variables to raw pointers where lifetime is obvious.
4. Use references for required parameters in newly written code.
5. Keep reflected `Pointer<T>` fields until pointer metadata policy is explicit.

### `List<T>` and intrusive list helpers

Current behavior:

- Intrusive list over elements with `next` links.
- Supports erasing during iteration and optional `deleteElements()`.
- Is reflected.

Modern replacement target:

- `std::list<T>` only if stable node addresses and list ownership are needed.
- `std::vector<T*>` or `std::vector<std::unique_ptr<T>>` for most modern game-engine workloads.
- Keep intrusive lists where object layout or removal costs are deliberate.

Recommended migration:

1. Avoid assuming `std::list` is better. Intrusive lists can be intentional.
2. Replace only small local ownership lists first.
3. Remove `deleteElements()` by making ownership explicit in the element type.
4. Preserve reflected lists until serialization support exists.

### `Stack<T, MaxElements>`

Current behavior:

- Wraps `std::vector<T>` and enforces `MaxElements` with runtime asserts.
- Converts implicitly to the top element.
- Arithmetic assignment operators push derived top values.

Modern replacement target:

- `std::vector<T>` with explicit `back()` for dynamic stacks.
- `std::array<T, MaxElements>` plus an index for fixed-capacity stacks.
  
Recommended migration:

1. Replace local stacks that only use `push`, `pop`, `back`, and `size` with `std::vector<T>`.
2. Keep or rewrite stacks using implicit top conversion and arithmetic push behavior with a small named helper type.
3. Consider `std::array` only when avoiding heap allocation matters.

### `RingBuffer<T>`

Current behavior:

- Subclasses `Array<T>` and tracks a current index.
- Reflection stores only `index`; `Array<T>` base reflection stores data.
- No active production call sites remain after migrating the trail object to local `std::vector` storage.

Modern replacement target:

- `std::vector<T>` plus index.
- A small local `RingBuffer<T>` implemented in terms of `std::vector<T>` if the abstraction remains useful.

Recommended migration:

1. Keep `RingBuffer.h` available until an explicit dead-code/removal pass verifies no reflection, script, or generated metadata path depends on its type name.
2. Prefer a small modern wrapper over direct `std::deque` unless front/back insertion semantics are actually needed.
3. Preserve reflection behavior if this type is serialized.

### `StringList` and `StringTree`

Current behavior:

- Ref-counted parsed data structures, not ordinary containers.
- Use `Reference`, `Vector`, `Map`, `AutoClass`, and pooled allocation.
- Expose parse-tree semantics such as atom/list nodes, `GetString`, `GetValue`, `ToInt`, `ToFloat`, child traversal, and source line tracking.

Modern replacement target:

- Keep domain-specific tree/list types.
- Modernize internals later: `std::vector`, `std::variant`, `std::unique_ptr` or intrusive pointer, and `std::string_view` parsing.

Recommended migration:

1. Do not replace with `std::vector<std::string>`; that would lose syntax-tree behavior.
2. Add non-owning read APIs using `std::string_view` where parsing allows it.
3. Isolate parsing and tree ownership from the generic container modernization work.
4. Revisit after `Reference` and pooled allocation policy are decided.

### `Data`, `Type`, and reflection support

Current behavior:

- `Data` owns dynamically typed values using `Type` allocation, assignment, and destruction callbacks.
- `Type` stores metadata for fields, derived types, conversions, functions, and string conversion.
- Many LTE wrappers plug into this through metadata macros.

Modern replacement target:

- This is not directly replaceable with one STL type.
- Possible standard helpers include `std::type_index`, `std::any`, `std::variant`, `std::span`, and concepts, but none is a drop-in replacement for the existing runtime metadata system.

Recommended migration:

1. Keep `Data` and `Type` stable while migrating leaf implementation code.
2. Add metadata specializations for selected standard types only when there is a concrete field migration that needs them.
3. Define a canonical serialization shape for `std::vector<T>`, `std::map<K, V>`, and `std::unique_ptr<T>` before moving reflected fields.
4. Avoid mixing reflected `Vector<T>` and `std::vector<T>` in script-visible APIs until script function generation can name and map both consistently.

## Proposed Migration Phases

### Phase 0: Stabilize and measure

Objective:
Make modernization measurable and reduce hidden coupling before changing storage types.

Tasks:

- Add a small `NeuronClient/LTE/StdCompat.h` or similar helper header for standard-compatible helpers: `Contains`, `FindPtr`, `EraseValue`, `EraseSwap`, `RandomElement`, and `AsSpan`.
- Add `.get()` to `Pointer`, `Reference`, and `AutoPtr`, and prefer it in new code.
- Add `AsStdVector()` or equivalent named accessors to `Vector` before replacing `.v` call sites.
- Add compile-time guards or comments around deprecated direct internals: `.t`, `.v`, `.buffer`, `.m`.
- Document which code is reflected/script-visible before touching it.

Validation:

- Build after each helper addition.
- No behavior changes beyond newly available APIs.

### Phase 1: Low-risk standard containers

Objective:
Replace wrappers that are already STL-backed and lightly used.

Candidates:

- `HashMap` -> `std::unordered_map`.
- `HashSet` -> `std::unordered_set`.
- Local `Tuple2`/`Tuple3`/`Tuple4` -> `std::pair`, `std::tuple`, or named structs.
- Local `Stack<T>` -> `std::vector<T>` or fixed-capacity helper.
- The one active `RingBuffer<T>` use, if non-reflected. Complete for `ObjectType_Trail`; header removal remains deferred.

Tasks:

- Remove or quarantine the `std::tr1` fallback in `HashMap.h` and `HashSet.h` after platform support is confirmed.
- Convert non-reflected local uses only.
- Keep public headers stable unless all consumers are updated in the same small slice.

Validation:

- Build `NeuronClient` and `EarthRise` after each type family.
- Run startup smoke test when runtime-facing code changes.

### Phase 2: Local ownership cleanup

Objective:
Replace legacy pointer ownership where semantics are local and obvious.

Candidates:

- `AutoPtr<T>` local variables -> `std::unique_ptr<T>`.
- `Vector<T*>` plus `deleteElements()` -> `std::vector<std::unique_ptr<T>>`.
- `Array<T>` raw buffer use -> `std::vector<T>` or `std::span<T>`.

Tasks:

- Replace `.t` with `.get()` before changing ownership types.
- Make transfer explicit with `std::move`.
- Prefer function parameters as `std::span<T>` / `std::span<T const>` for buffer views.
- Avoid reflected fields and script-visible APIs in this phase.

Validation:

- Build after each small ownership conversion.
- Watch for lifetime changes around callbacks, global registries, and pooled types.

### Phase 3: Local `Vector`, `Map`, and `Array` conversions

Objective:
Move implementation-only containers to standard types while reflected APIs remain stable.

Candidates:

- Local `Vector<T>` temporaries in `.cpp` files.
- Local `Map<K, V>` instances that do not rely on reflection or `_ToStream`.
- Local `Array<T>` buffers in audio, shader, file, and decode paths.

Tasks:

- Convert one subsystem at a time: LTE internals first, then Module, then Game/Component.
- Keep API boundary types unchanged unless the whole call chain is local to the subsystem.
- Use standard algorithms and ranges where they simplify code without changing behavior.
- Avoid changing deterministic iteration behavior accidentally.

Validation:

- Build and smoke test after each subsystem.
- For buffer-heavy code, run any available runtime path that touches loading, shaders, audio, or mesh generation.

### Phase 4: Reflection-aware standard type support

Objective:
Allow selected standard types in reflected fields and script-visible data without breaking serialization.

Tasks:

- Add `Type_Get<std::vector<T>>()` support or a named reflected adapter with the same serialized shape as `Vector<T>`.
- Add `Type_Get<std::map<K, V>>()` support or a named adapter with the same serialized shape as `Map<K, V>`.
- Decide whether `std::string` and `LTE::String` share one runtime type name or remain distinct with conversions.
- Decide whether `std::unique_ptr<T>` fields are allowed in reflection. If yes, define null/pointee serialization behavior.
- Add tests or small validation tools for round-tripping reflected containers.

Validation:

- Round-trip representative data through existing `DataReader`/`DataWriter` paths.
- Verify script function metadata names remain stable or are deliberately migrated.

### Phase 5: Intrusive handle modernization

Objective:
Modernize `Reference<T>` and `Pointer<T>` semantics after container migration has reduced incidental coupling.

Options:

- Keep intrusive ownership but rename or reimplement as a standard-like `IntrusivePtr<T>` with `.get()`, `.reset()`, move support, and no public `.t` in new code.
- Move selected non-engine resources to `std::shared_ptr<T>` where pooling/reflection/script handles do not depend on intrusive ref counts.
- Use raw pointers or references for non-owning APIs where lifetime is obvious.

Tasks:

- Remove most external `.t` accesses first.
- Define threading policy for ref counts.
- Define weak-reference or observer behavior before replacing handles used by object graphs.
- Do not mix `std::shared_ptr` ownership with existing intrusive ownership for the same object type.

Validation:

- Build after each handle family.
- Runtime smoke tests focused on object creation/destruction, UI widgets, resources, and script-owned values.

## Recommended First Work Items

1. Add `.get()` to `Pointer`, `Reference`, and `AutoPtr` and convert a small set of `.t` call sites in `LTE` only.
2. Replace `.v` direct sorting/manipulation with `begin()`/`end()` or an explicit `AsStdVector()` accessor.
3. Convert `HashMap`/`HashSet` use sites to `std::unordered_map`/`std::unordered_set`, or at least remove the obsolete `std::tr1` platform branch.
4. Fix `Tuple3`/`Tuple4` comparison self-comparison bugs and convert non-reflected tuple uses to `std::pair`/`std::tuple` or named structs.
5. Audit `Array<T>` element types and replace the non-reflected byte-buffer cases first.
6. Convert local `AutoPtr` ownership to `std::unique_ptr` where it does not cross an API boundary.

These slices are useful because they reduce coupling and modernize real code without requiring a redesign of the runtime type system.

## What Not To Do First

- Do not globally alias `String` to `std::string`. Reflection, metadata, hashing, implicit C-string conversion, and utility functions make that unsafe.
- Do not globally replace `Vector<T>` with `std::vector<T>`. Reflected fields and script-generated member functions rely on `Vector` metadata.
- Do not replace `Reference<T>` with `std::shared_ptr<T>` mechanically. Existing objects are intrusive, and many call sites depend on `.t` or implicit raw pointer conversion.
- Do not replace `Pointer<T>` with `std::unique_ptr<T>`. `Pointer` is non-owning.
- Do not replace `Array<T>` with `std::array<T, N>`. `Array<T>` is runtime-sized heap storage, not compile-time fixed storage.
- Do not change ordered `Map` to `unordered_map` where iteration order affects serialization, deterministic gameplay, or debug output.

## Validation Strategy

For every modernization slice:

- Build with CMake Tools for the active Debug x64 configuration.
- Use compile errors as migration guide, not as permission for broad drive-by rewrites.
- Run at least `EarthRise.exe war` as a startup smoke test when runtime code changes.
- If reflected fields change, add or run a save/load round-trip test before continuing.
- Check git status before and after each slice so generated output, deleted assets, or unrelated working-tree changes are not folded into the modernization.

## Expected End State

The realistic end state is not "delete all LTE types." A healthier target is:

- New implementation code uses standard C++ types by default.
- Runtime/reflection/script boundaries use either explicitly reflected standard types or retained LTE adapters.
- Ownership is explicit: `std::unique_ptr` for local ownership, raw/reference/observer handles for non-owning references, and a modern intrusive pointer or carefully chosen `std::shared_ptr` for shared ownership.
- Old custom wrappers remain only where they encode domain behavior: reflection, script metadata, intrusive engine handles, parsed string trees, pooled AST-like structures, or compatibility layers awaiting migration.

This path gives the project modern C++ ergonomics without tearing out the engine runtime model in one risky pass.