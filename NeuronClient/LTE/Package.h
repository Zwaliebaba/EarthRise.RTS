#ifndef LTE_Package_h__
#define LTE_Package_h__

#include "Common.h"
#include "Reference.h"

struct PackageT : public RefCounted {
  ~PackageT();

  void AddFunction(Function const& function);
  void AddType(Type const& type);
  Function GetFunction(String const& name) const;
  Type GetType(String const& name) const;
};

Package Package_Create(String const& name);
Package Package_Global();

#endif
