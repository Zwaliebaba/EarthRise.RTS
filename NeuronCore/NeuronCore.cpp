#include "pch.h"

namespace
{
  bool g_apartmentAvailable = false;
  bool g_apartmentInitializedByCore = false;
}

void CoreEngine::Startup()
{
  if (!g_apartmentAvailable)
  {
    try
    {
      init_apartment();
      g_apartmentInitializedByCore = true;
      g_apartmentAvailable = true;
    }
    catch (const winrt::hresult_error& error)
    {
      if (error.code() != winrt::hresult(RPC_E_CHANGED_MODE))
        throw;

      g_apartmentAvailable = true;
    }
  }

  if (!XMVerifyCPUSupport())
    Fatal(L"CPU does not support the right technology");

  Timer::Core::Startup();
}

void CoreEngine::Shutdown()
{
  if (g_apartmentInitializedByCore)
  {
    uninit_apartment();
    g_apartmentInitializedByCore = false;
  }

  g_apartmentAvailable = false;
}
