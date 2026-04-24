#include "ResourceMap.h"
#include "Map.h"
#include "OS.h"

#include "Debug.h"

namespace
{
  struct ResourceMapImpl : ResourceMapT
  {
    Map<String, String> pathMap;

    void AddDirectory(const String& path, const String& alias) override
    {
      Vector<String> elements = OS_ListDir(path);
      for (size_t i = 0; i < elements.size(); ++i)
      {
        const String& element = elements[i];
        if (element == "." || element == "..")
          continue;

        String fullPath = path + element;
        if (OS_IsDir(fullPath))
          AddDirectory(fullPath + "/", alias + element + "/");
        else if (OS_IsFile(fullPath))
          AddFile(fullPath, alias + element);
      }
    }

    void AddFile(const String& path, const String& alias) override { pathMap[alias] = path; }

    bool Exists(const String& path) const override { return pathMap.contains(path); }

    String Get(const String& path) const override
    {
      const String* result = pathMap.get(path);
      if (!result)
        dbg | "Bad lookup : " | path | endl;
      if (result)
        return *result;
      return String();
    }
  };
}

ResourceMap ResourceMap_Create() { return new ResourceMapImpl; }
