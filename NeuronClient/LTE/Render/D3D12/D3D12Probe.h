#pragma once

#include "LTE/Common.h"

#include <string>
#include <vector>

namespace LTE::D3D12 {
  struct AdapterInfo {
    std::wstring name;
    uint vendorID;
    uint deviceID;
    size_t dedicatedVideoMemory;
    bool isSoftware;
    bool supportsFeatureLevel12_1;
  };

  struct ProbeResult {
    std::vector<AdapterInfo> adapters;
    size_t selectedAdapterIndex;
    bool hasHardwareFeatureLevel12_1Adapter;
  };

  LT_API char const* GetRequiredFeatureLevelName();

  LT_API ProbeResult ProbeAdapters(bool enableDebugLayer);
}