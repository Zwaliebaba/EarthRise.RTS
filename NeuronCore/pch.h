#pragma once

// Keep the core PCH renderer-agnostic and platform-light. Do not add Windows
// or graphics SDK headers here; NeuronCore should stay usable by client and
// future shared code without inheriting platform macro pollution.

#include "NeuronCore.h"
