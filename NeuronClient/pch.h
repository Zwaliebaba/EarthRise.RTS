#pragma once

// Precompiled header for NeuronClient.
// Curated STL-only set: must NOT transitively pull in <windows.h>,
// because that exposes legacy macros (near, far, min, max, DrawState, ...)
// which collide with identifiers used throughout the codebase.
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <exception>
#include <format>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <queue>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
