#ifndef LTE_Serializer_h__
#define LTE_Serializer_h__

#include "AutoPtr.h"

namespace LTE {
  struct Serializer {
    virtual ~Serializer() {}
    virtual bool IsGood() const = 0;
    virtual int GetVersion() const = 0;
    virtual void Process(void* data, Type const& type) = 0;
  };

  Serializer* CreateLoader(Location const& location);
  Serializer* CreateSaver(Location const& location, int version);

  template <class T>
  bool LoadFrom(
    T& t,
    Location const& location,
    int minVersion = 0,
    int maxVersion = INT_MAX)
  {
    AutoPtr<Serializer> s = CreateLoader(location);
    if (!s->IsGood()
        || s->GetVersion() < minVersion
        || s->GetVersion() > maxVersion)
      return false;
    s->Process(&t, Type_Get(t));
    return s->IsGood();
  }

  template <class T>
  void SaveTo(
    T& t,
    Location const& location,
    int version)
  {
    AutoPtr<Serializer> s = CreateSaver(location, version);
    s->Process(&t, Type_Get(t));
  }
}

#endif
