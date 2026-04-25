#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "LTE/Render/D3D12/D3D12Probe.h"

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <limits>
#include <winrt/base.h>

namespace {
  bool IsSoftwareAdapter(DXGI_ADAPTER_DESC1 const& desc) {
    return (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0;
  }

  bool SupportsFeatureLevel12_1(IDXGIAdapter1* adapter) {
    winrt::com_ptr<ID3D12Device> device;
    return SUCCEEDED(D3D12CreateDevice(
      adapter,
      D3D_FEATURE_LEVEL_12_1,
      __uuidof(ID3D12Device),
      device.put_void()));
  }

  void TryEnableDebugLayer() {
    winrt::com_ptr<ID3D12Debug> debug;
    if (SUCCEEDED(D3D12GetDebugInterface(
          __uuidof(ID3D12Debug),
          debug.put_void())))
    {
      debug->EnableDebugLayer();
    }
  }

  HRESULT CreateFactory(
    bool enableDebugLayer,
    winrt::com_ptr<IDXGIFactory6>& factory)
  {
    UINT flags = enableDebugLayer ? DXGI_CREATE_FACTORY_DEBUG : 0;
    HRESULT result = CreateDXGIFactory2(
      flags,
      __uuidof(IDXGIFactory6),
      factory.put_void());

    if (FAILED(result) && flags) {
      factory = nullptr;
      result = CreateDXGIFactory2(
        0,
        __uuidof(IDXGIFactory6),
        factory.put_void());
    }

    return result;
  }
}

namespace LTE::D3D12 {
  char const* GetRequiredFeatureLevelName() {
    return "12_1";
  }

  ProbeResult ProbeAdapters(bool enableDebugLayer) {
    ProbeResult result;
    result.selectedAdapterIndex = std::numeric_limits<size_t>::max();
    result.hasHardwareFeatureLevel12_1Adapter = false;

    if (enableDebugLayer)
      TryEnableDebugLayer();

    winrt::com_ptr<IDXGIFactory6> factory;
    if (FAILED(CreateFactory(enableDebugLayer, factory)))
      return result;

    for (UINT adapterIndex = 0;; ++adapterIndex) {
      winrt::com_ptr<IDXGIAdapter1> adapter;
      HRESULT enumResult = factory->EnumAdapterByGpuPreference(
        adapterIndex,
        DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
        __uuidof(IDXGIAdapter1),
        adapter.put_void());

      if (enumResult == DXGI_ERROR_NOT_FOUND)
        break;
      if (FAILED(enumResult))
        continue;

      DXGI_ADAPTER_DESC1 desc = {};
      if (FAILED(adapter->GetDesc1(&desc)))
        continue;

      AdapterInfo info;
      info.name = desc.Description;
      info.vendorID = desc.VendorId;
      info.deviceID = desc.DeviceId;
      info.dedicatedVideoMemory = desc.DedicatedVideoMemory;
      info.isSoftware = IsSoftwareAdapter(desc);
      info.supportsFeatureLevel12_1 = SupportsFeatureLevel12_1(adapter.get());

      result.adapters.push_back(info);

      if (!info.isSoftware && info.supportsFeatureLevel12_1 &&
          !result.hasHardwareFeatureLevel12_1Adapter)
      {
        result.selectedAdapterIndex = result.adapters.size() - 1;
        result.hasHardwareFeatureLevel12_1Adapter = true;
      }
    }

    return result;
  }
}