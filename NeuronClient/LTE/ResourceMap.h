#ifndef LTE_ResourceMap_h__
#define LTE_ResourceMap_h__

#include "Reference.h"

struct ResourceMapT : RefCounted
{
  ~ResourceMapT() override = default;

  virtual void AddDirectory(const String& path, const String& alias = "") = 0;

  virtual void AddFile(const String& path, const String& alias = "") = 0;

  virtual bool Exists(const String& path) const = 0;

  virtual String Get(const String& path) const = 0;
};

ResourceMap ResourceMap_Create();

#endif
