# Phase 3 Safety Inventory

## Scope
This inventory starts Phase 3 of `impl.md`: thread and memory safety hardening. It records the current reference-counting and type-registry risks before changing atomics, locks, or ownership behavior.

## Current Findings

### Intrusive Reference Counting
- Owner: `NeuronClient/LTE/Reference.h`
- Core type: `RefCounted`
- Current counter: non-atomic `uint refCount`
- Main wrapper: `Reference<T>`
- Mutation paths: `Reference<T>::Acquire`, `Reference<T>::Release`, `RefCountIncrement`, and `RefCountDecrement`
- Deletion behavior: `Reference<T>::Release` deletes the pointee when `RefCountDecrement` reaches zero.

Current contract is implicit. The code behaves as thread-confined intrusive reference counting. Sharing the same `RefCounted` object across threads can race increments/decrements and can cause double delete, use-after-free, or leaked references.

### Type Metadata Registry
- Owner: `NeuronClient/LTE/Type.h` and `NeuronClient/LTE/Type.cpp`
- Global storage: `Type_GetStorage<T>()` creates a static `Type` per reflected C++ type.
- Registry containers: `GetTypeList()` and `GetTypeMap()` return static legacy containers.
- Registration path: `Type_Create` inserts into both containers and increments a static `GUIDIndex`.
- Lazy initialization pattern: reflection macros check `if (!type)` before assigning the result of `Type_Create`.

Current contract is also implicit. Type metadata registration appears designed for startup or single-threaded reflection discovery. Concurrent first access to the same type or concurrent registration of different types can race on the static metadata pointer, registry containers, and GUID assignment.

## Recommended Contract Decisions
1. Treat `Reference<T>` as thread-confined until proven otherwise.
2. Document that cross-thread transfer of `Reference<T>` requires an owning handoff boundary, not shared concurrent mutation.
3. Treat type metadata registration as startup-only unless runtime registration is proven necessary.
4. If runtime type registration is required, protect `Type_Create`, registry containers, `GUIDIndex`, and each lazy `Type_GetStorage<T>()` initialization path.
5. Do not convert `refCount` to `std::atomic` mechanically until ownership transfer points are inventoried.

## Next Implementation Slice
1. Add comments or lightweight diagnostics that state the thread-confined `Reference<T>` contract.
2. Inventory cross-thread task, async loader, event, and network call sites that store or pass `Reference<T>`.
3. Decide whether the type registry should be explicitly initialized during startup or guarded for runtime lazy registration.
4. Add focused tests only after the chosen contracts are documented.

## Non-Goals For This Slice
- No broad replacement of `Reference<T>` with standard smart pointers.
- No mechanical conversion of every `refCount` access to atomics.
- No changes to reflection-visible ownership semantics.
- No renderer or D3D12 work.
