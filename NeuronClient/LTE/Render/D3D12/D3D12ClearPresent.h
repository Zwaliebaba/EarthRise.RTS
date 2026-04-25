#pragma once

#include "LTE/Common.h"

#include <string>

namespace LTE::D3D12 {
  struct ClearPresentDesc {
    std::wstring title = L"EarthRise D3D12 Bootstrap";
    uint width = 1280;
    uint height = 720;
    bool enableDebugLayer = true;
    bool vsync = true;
    float clearColor[4] = {0.02f, 0.04f, 0.08f, 1.0f};
  };

  struct ClearPresentResult {
    bool succeeded = false;
    long resultCode = 0;
    std::wstring message;
    std::wstring adapterName;
    uint64 gpuTimestampTicks = 0;
    uint64 gpuTimestampFrequency = 0;
    size_t generatedShaderBytes = 0;
  };

  LT_API ClearPresentResult RunClearPresentSmokeTest(
    ClearPresentDesc const& desc);
}