#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "LTE/Render/D3D12/D3D12ClearPresent.h"

#if EARTHRISE_D3D12_GENERATED_SHADERS
#include "BootstrapClearPS.h"
#include "BootstrapClearVS.h"
#endif

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <winrt/base.h>

namespace {
  constexpr UINT kFrameCount = 2;
  wchar_t const* kWindowClassName = L"EarthRiseD3D12BootstrapWindow";

  struct Win32Event {
    HANDLE handle;

    Win32Event() : handle(nullptr) {}

    ~Win32Event() {
      if (handle)
        CloseHandle(handle);
    }

    Win32Event(Win32Event const&) = delete;
    Win32Event& operator=(Win32Event const&) = delete;
  };

  bool IsSoftwareAdapter(DXGI_ADAPTER_DESC1 const& desc) {
    return (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0;
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

  D3D12_RESOURCE_BARRIER TransitionBarrier(
    ID3D12Resource* resource,
    D3D12_RESOURCE_STATES before,
    D3D12_RESOURCE_STATES after)
  {
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    return barrier;
  }

  D3D12_RESOURCE_DESC BufferDesc(UINT64 bytes) {
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = bytes;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    return desc;
  }

  class BootstrapWindow {
  public:
    BootstrapWindow() : hwnd(nullptr), width(0), height(0), closed(false) {}

    ~BootstrapWindow() {
      if (hwnd)
        DestroyWindow(hwnd);
    }

    BootstrapWindow(BootstrapWindow const&) = delete;
    BootstrapWindow& operator=(BootstrapWindow const&) = delete;

    HRESULT Create(std::wstring const& title, uint requestedWidth, uint requestedHeight) {
      HINSTANCE instance = GetModuleHandleW(nullptr);

      WNDCLASSEXW windowClass = {};
      windowClass.cbSize = sizeof(windowClass);
      windowClass.lpfnWndProc = &BootstrapWindow::WndProc;
      windowClass.hInstance = instance;
      windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
      windowClass.lpszClassName = kWindowClassName;

      if (!RegisterClassExW(&windowClass)) {
        DWORD error = GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS)
          return HRESULT_FROM_WIN32(error);
      }

      RECT rect = {
        0,
        0,
        static_cast<LONG>(requestedWidth),
        static_cast<LONG>(requestedHeight)
      };
      DWORD style = WS_OVERLAPPEDWINDOW;
      if (!AdjustWindowRectEx(&rect, style, FALSE, 0))
        return HRESULT_FROM_WIN32(GetLastError());

      hwnd = CreateWindowExW(
        0,
        kWindowClassName,
        title.c_str(),
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        instance,
        this);

      if (!hwnd)
        return HRESULT_FROM_WIN32(GetLastError());

      width = requestedWidth;
      height = requestedHeight;
      ShowWindow(hwnd, SW_SHOW);
      UpdateWindow(hwnd);
      return S_OK;
    }

    bool PumpMessages() {
      MSG message = {};
      while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
      }

      return !closed;
    }

    HWND GetHwnd() const {
      return hwnd;
    }

  private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
      BootstrapWindow* window = nullptr;

      if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        window = static_cast<BootstrapWindow*>(create->lpCreateParams);
        window->hwnd = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
      } else {
        window = reinterpret_cast<BootstrapWindow*>(
          GetWindowLongPtrW(hwnd, GWLP_USERDATA));
      }

      if (window) {
        switch (message) {
          case WM_CLOSE:
            window->closed = true;
            DestroyWindow(hwnd);
            return 0;

          case WM_DESTROY:
            window->closed = true;
            window->hwnd = nullptr;
            return 0;

          case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) {
              window->width = LOWORD(lParam);
              window->height = HIWORD(lParam);
            }
            return 0;

          default:
            break;
        }
      }

      return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    HWND hwnd;
    uint width;
    uint height;
    bool closed;
  };

  struct D3D12Bootstrap {
    winrt::com_ptr<IDXGIFactory6> factory;
    winrt::com_ptr<IDXGIAdapter1> adapter;
    winrt::com_ptr<ID3D12Device> device;
    winrt::com_ptr<ID3D12CommandQueue> commandQueue;
    winrt::com_ptr<IDXGISwapChain3> swapChain;
    winrt::com_ptr<ID3D12DescriptorHeap> rtvHeap;
    winrt::com_ptr<ID3D12Resource> renderTargets[kFrameCount];
    winrt::com_ptr<ID3D12CommandAllocator> commandAllocator;
    winrt::com_ptr<ID3D12GraphicsCommandList> commandList;
    winrt::com_ptr<ID3D12Fence> fence;
    winrt::com_ptr<ID3D12QueryHeap> timestampQueryHeap;
    winrt::com_ptr<ID3D12Resource> timestampReadback;
    Win32Event fenceEvent;
    UINT rtvDescriptorSize = 0;
    UINT frameIndex = 0;
    UINT64 fenceValue = 0;
    std::wstring adapterName;

    HRESULT Initialize(BootstrapWindow& window, LTE::D3D12::ClearPresentDesc const& desc) {
      if (desc.enableDebugLayer)
        TryEnableDebugLayer();

      HRESULT result = CreateFactory(desc.enableDebugLayer, factory);
      if (FAILED(result))
        return result;

      result = SelectAdapter();
      if (FAILED(result))
        return result;

      result = D3D12CreateDevice(
        adapter.get(),
        D3D_FEATURE_LEVEL_12_1,
        __uuidof(ID3D12Device),
        device.put_void());
      if (FAILED(result))
        return result;

      device->SetName(L"EarthRise D3D12 Bootstrap Device");

      D3D12_COMMAND_QUEUE_DESC queueDesc = {};
      queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
      queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
      result = device->CreateCommandQueue(
        &queueDesc,
        __uuidof(ID3D12CommandQueue),
        commandQueue.put_void());
      if (FAILED(result))
        return result;
      commandQueue->SetName(L"EarthRise D3D12 Bootstrap Direct Queue");

      DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
      swapChainDesc.BufferCount = kFrameCount;
      swapChainDesc.Width = desc.width;
      swapChainDesc.Height = desc.height;
      swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      swapChainDesc.SampleDesc.Count = 1;

      winrt::com_ptr<IDXGISwapChain1> swapChain1;
      result = factory->CreateSwapChainForHwnd(
        commandQueue.get(),
        window.GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        swapChain1.put());
      if (FAILED(result))
        return result;

      factory->MakeWindowAssociation(window.GetHwnd(), DXGI_MWA_NO_ALT_ENTER);
      swapChain = swapChain1.as<IDXGISwapChain3>();
      frameIndex = swapChain->GetCurrentBackBufferIndex();

      D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
      rtvHeapDesc.NumDescriptors = kFrameCount;
      rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      result = device->CreateDescriptorHeap(
        &rtvHeapDesc,
        __uuidof(ID3D12DescriptorHeap),
        rtvHeap.put_void());
      if (FAILED(result))
        return result;
      rtvHeap->SetName(L"EarthRise D3D12 Bootstrap RTV Heap");
      rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        rtvHeap->GetCPUDescriptorHandleForHeapStart();
      for (UINT i = 0; i < kFrameCount; ++i) {
        result = swapChain->GetBuffer(
          i,
          __uuidof(ID3D12Resource),
          renderTargets[i].put_void());
        if (FAILED(result))
          return result;

        wchar_t name[] = L"EarthRise D3D12 Bootstrap Back Buffer 0";
        name[sizeof(name) / sizeof(name[0]) - 2] = static_cast<wchar_t>(L'0' + i);
        renderTargets[i]->SetName(name);
        device->CreateRenderTargetView(renderTargets[i].get(), nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
      }

      result = device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        __uuidof(ID3D12CommandAllocator),
        commandAllocator.put_void());
      if (FAILED(result))
        return result;
      commandAllocator->SetName(L"EarthRise D3D12 Bootstrap Command Allocator");

      result = device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator.get(),
        nullptr,
        __uuidof(ID3D12GraphicsCommandList),
        commandList.put_void());
      if (FAILED(result))
        return result;
      commandList->SetName(L"EarthRise D3D12 Bootstrap Command List");
      commandList->Close();

      result = CreateTimestampResources();
      if (FAILED(result))
        return result;

      result = device->CreateFence(
        0,
        D3D12_FENCE_FLAG_NONE,
        __uuidof(ID3D12Fence),
        fence.put_void());
      if (FAILED(result))
        return result;
      fence->SetName(L"EarthRise D3D12 Bootstrap Fence");

      fenceEvent.handle = CreateEventW(nullptr, FALSE, FALSE, nullptr);
      if (!fenceEvent.handle)
        return HRESULT_FROM_WIN32(GetLastError());

      return S_OK;
    }

    HRESULT ClearPresent(
      LTE::D3D12::ClearPresentDesc const& desc,
      uint64& timestampTicks,
      uint64& timestampFrequency)
    {
      HRESULT result = commandAllocator->Reset();
      if (FAILED(result))
        return result;

      result = commandList->Reset(commandAllocator.get(), nullptr);
      if (FAILED(result))
        return result;

      D3D12_RESOURCE_BARRIER toRenderTarget = TransitionBarrier(
        renderTargets[frameIndex].get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
      commandList->ResourceBarrier(1, &toRenderTarget);

      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        rtvHeap->GetCPUDescriptorHandleForHeapStart();
      rtvHandle.ptr += frameIndex * rtvDescriptorSize;

      char const marker[] = "EarthRise D3D12 Bootstrap Clear";
      commandList->EndQuery(timestampQueryHeap.get(), D3D12_QUERY_TYPE_TIMESTAMP, 0);
      commandList->BeginEvent(0, marker, sizeof(marker));
      commandList->ClearRenderTargetView(rtvHandle, desc.clearColor, 0, nullptr);
      commandList->EndEvent();
      commandList->EndQuery(timestampQueryHeap.get(), D3D12_QUERY_TYPE_TIMESTAMP, 1);

      D3D12_RESOURCE_BARRIER toPresent = TransitionBarrier(
        renderTargets[frameIndex].get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
      commandList->ResourceBarrier(1, &toPresent);

      commandList->ResolveQueryData(
        timestampQueryHeap.get(),
        D3D12_QUERY_TYPE_TIMESTAMP,
        0,
        2,
        timestampReadback.get(),
        0);

      result = commandList->Close();
      if (FAILED(result))
        return result;

      ID3D12CommandList* commandLists[] = {commandList.get()};
      commandQueue->ExecuteCommandLists(1, commandLists);

      result = swapChain->Present(desc.vsync ? 1 : 0, 0);
      if (FAILED(result))
        return result;

      result = WaitForGpu();
      if (FAILED(result))
        return result;

      frameIndex = swapChain->GetCurrentBackBufferIndex();

      UINT64 frequency = 0;
      result = commandQueue->GetTimestampFrequency(&frequency);
      if (FAILED(result))
        return result;

      UINT64 timestamps[2] = {};
      D3D12_RANGE readRange = {0, sizeof(timestamps)};
      void* mapped = nullptr;
      result = timestampReadback->Map(0, &readRange, &mapped);
      if (FAILED(result))
        return result;

      memcpy(timestamps, mapped, sizeof(timestamps));
      D3D12_RANGE writtenRange = {0, 0};
      timestampReadback->Unmap(0, &writtenRange);

      timestampFrequency = frequency;
      timestampTicks = timestamps[1] >= timestamps[0]
        ? timestamps[1] - timestamps[0]
        : 0;
      return S_OK;
    }

  private:
    HRESULT SelectAdapter() {
      for (UINT adapterIndex = 0;; ++adapterIndex) {
        winrt::com_ptr<IDXGIAdapter1> candidate;
        HRESULT enumResult = factory->EnumAdapterByGpuPreference(
          adapterIndex,
          DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
          __uuidof(IDXGIAdapter1),
          candidate.put_void());

        if (enumResult == DXGI_ERROR_NOT_FOUND)
          return DXGI_ERROR_UNSUPPORTED;
        if (FAILED(enumResult))
          continue;

        DXGI_ADAPTER_DESC1 desc = {};
        if (FAILED(candidate->GetDesc1(&desc)) || IsSoftwareAdapter(desc))
          continue;

        winrt::com_ptr<ID3D12Device> testDevice;
        if (SUCCEEDED(D3D12CreateDevice(
              candidate.get(),
              D3D_FEATURE_LEVEL_12_1,
              __uuidof(ID3D12Device),
              testDevice.put_void())))
        {
          adapter = candidate;
          adapterName = desc.Description;
          return S_OK;
        }
      }
    }

    HRESULT CreateTimestampResources() {
      D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
      queryHeapDesc.Count = 2;
      queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
      HRESULT result = device->CreateQueryHeap(
        &queryHeapDesc,
        __uuidof(ID3D12QueryHeap),
        timestampQueryHeap.put_void());
      if (FAILED(result))
        return result;
      timestampQueryHeap->SetName(L"EarthRise D3D12 Bootstrap Timestamp Query Heap");

      D3D12_HEAP_PROPERTIES readbackHeap = {};
      readbackHeap.Type = D3D12_HEAP_TYPE_READBACK;
      D3D12_RESOURCE_DESC readbackDesc = BufferDesc(sizeof(UINT64) * 2);
      result = device->CreateCommittedResource(
        &readbackHeap,
        D3D12_HEAP_FLAG_NONE,
        &readbackDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        __uuidof(ID3D12Resource),
        timestampReadback.put_void());
      if (FAILED(result))
        return result;
      timestampReadback->SetName(L"EarthRise D3D12 Bootstrap Timestamp Readback");
      return S_OK;
    }

    HRESULT WaitForGpu() {
      UINT64 targetFence = ++fenceValue;
      HRESULT result = commandQueue->Signal(fence.get(), targetFence);
      if (FAILED(result))
        return result;

      if (fence->GetCompletedValue() < targetFence) {
        result = fence->SetEventOnCompletion(targetFence, fenceEvent.handle);
        if (FAILED(result))
          return result;
        WaitForSingleObject(fenceEvent.handle, INFINITE);
      }

      return S_OK;
    }
  };

  LTE::D3D12::ClearPresentResult Failure(HRESULT result, wchar_t const* message) {
    LTE::D3D12::ClearPresentResult output;
    output.succeeded = false;
    output.resultCode = result;
    output.message = message;
    return output;
  }
}

namespace LTE::D3D12 {
  ClearPresentResult RunClearPresentSmokeTest(ClearPresentDesc const& desc) {
    ClearPresentResult result;

#if EARTHRISE_D3D12_GENERATED_SHADERS
    result.generatedShaderBytes = sizeof(g_BootstrapClearVS) + sizeof(g_BootstrapClearPS);
#endif

    BootstrapWindow window;
    HRESULT hr = window.Create(desc.title, desc.width, desc.height);
    if (FAILED(hr))
      return Failure(hr, L"Failed to create native Win32 bootstrap window");

    window.PumpMessages();

    D3D12Bootstrap bootstrap;
    hr = bootstrap.Initialize(window, desc);
    if (FAILED(hr))
      return Failure(hr, L"Failed to initialize D3D12 bootstrap device and swap chain");

    hr = bootstrap.ClearPresent(
      desc,
      result.gpuTimestampTicks,
      result.gpuTimestampFrequency);
    if (FAILED(hr))
      return Failure(hr, L"Failed to clear and present D3D12 bootstrap frame");

    result.succeeded = true;
    result.resultCode = S_OK;
    result.message = L"D3D12 bootstrap clear-present completed";
    result.adapterName = bootstrap.adapterName;
    return result;
  }
}