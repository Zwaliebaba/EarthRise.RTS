#include "Location.h"
#include "Array.h"
#include "AutoClass.h"
#include "AutoPtr.h"
#include "OS.h"
#include "Pool.h"
#include "ProgramLog.h"
#include "ResourceMap.h"
#include "Vector.h"
#include "SFML/Network.hpp"
#include <fstream>
#include "FileSys.h"
#include "LTE/Debug.h"

TypeAlias(Reference<LocationT>, Location);

namespace
{
  const ResourceMap& GetResourceMap()
  {
    static ResourceMap resourceMap;
    if (!resourceMap)
    {
      resourceMap = ResourceMap_Create();
      resourceMap->AddDirectory(Neuron::FileSys::GetHomeDirectoryA(), "");
    }
    return resourceMap;
  }

  AutoClassDerived(LocationFile, LocationT, String, path)
    DERIVED_TYPE_EX(LocationFile)
    POOLED_TYPE

    LocationFile() {}

    Location Clone() const override { return new LocationFile(*this); }

    bool Exists() const override
    {
      std::ifstream stream(path.c_str(), std::ios::binary);
      return stream.good();
    }

    AutoPtr<Array<uchar>> Read() const override
    {
      std::ifstream stream(path.c_str(), std::ios::binary);
      if (!stream || !OS_IsFile(path))
      {
        Log_Warning("Failed to open file <" + path + "> for reading");
        return nullptr;
      }

      stream.seekg(0, std::ios_base::end);
      size_t size = stream.tellg();
      stream.seekg(0);

      auto buf = new char[size];
      if (!stream.read(buf, size))
      {
        free(buf);
        Log_Warning("Failed to read file <" + path + ">");
        return nullptr;
      }
      return (new Array<uchar>)->set((uchar*)buf, size);
    }

    String ToString() const override { return path; }

    bool Write(const Array<uchar>& data) const override
    {
      std::ofstream stream(path.c_str(), std::ios::binary);
      if (!stream)
      {
        OS_CreatePath(path.c_str());
        stream.close();
        stream.clear();
        stream.open(path.c_str(), std::ios::binary);
        if (!stream)
        {
          Log_Warning("Failed to open file <" + path + "> for writing");
          return false;
        }
      }
      stream.write((const char*)data.data(), data.size());
      return stream.good();
    }
  };

  DERIVED_IMPLEMENT(LocationFile)

  AutoClassDerived(LocationMemory, LocationT, Array<uchar>*, memory, bool, ownsMemory)
    DERIVED_TYPE_EX(LocationMemory)
    POOLED_TYPE

    LocationMemory()
      : memory(new Array<uchar>),
        ownsMemory(true) {}

    LocationMemory(Array<uchar>* memory)
      : memory(memory),
        ownsMemory(false) {}

    LocationMemory(const String& str)
      : memory(new Array<uchar>),
        ownsMemory(true)
    {
      memory->resize(str.size());
      memcpy(memory->data(), &str.front(), str.size());
    }

    ~LocationMemory() override
    {
      if (ownsMemory)
        delete memory;
    }

    Location Clone() const override { return new LocationMemory(*this); }

    bool Exists() const override { return true; }

    AutoPtr<Array<uchar>> Read() const override
    {
      auto arr = new Array<uchar>;
      *arr = *memory;
      return arr;
    }

    String ToString() const override { return "[Memory]"; }

    bool Write(const Array<uchar>& data) const override
    {
      *((LocationMemory*)this)->memory = data;
      return true;
    }
  };

  DERIVED_IMPLEMENT(LocationMemory)

  AutoClassDerived(LocationResource, LocationT, String, name)
    DERIVED_TYPE_EX(LocationResource)
    POOLED_TYPE

    LocationResource() {}

    Location Clone() const override { return new LocationResource(*this); }

    bool Exists() const override
    {
      return GetResourceMap()->Exists(name);
    }

    AutoPtr<Array<uchar>> Read() const override
    {
      return LocationFile(GetResourceMap()->Get(name)).Read();
    }

    String ToString() const override { return Neuron::FileSys::GetHomeDirectoryA() + name; }

    bool Write(const Array<uchar>& data) const override
    {
      return LocationFile(GetResourceMap()->Get(name)).Write(data);
    }
  };

  DERIVED_IMPLEMENT(LocationResource)

  AutoClassDerived(LocationWeb, LocationT, String, host, String, item)
    DERIVED_TYPE_EX(LocationWeb)
    POOLED_TYPE

    LocationWeb() {}

    Location Clone() const override { return new LocationWeb(*this); }

    bool Exists() const override { return true; }

    bool Write(const Array<uchar>& data) const override
    {
      DEBUG_ASSERT(!data.size());
      return false;
    }

    AutoPtr<Array<uchar>> Read() const override
    {
      sf::Http http(host);
      sf::Http::Response response = http.sendRequest(sf::Http::Request(item));
      if (response.getStatus() != sf::Http::Response::Status::Ok)
        return nullptr;

      String message = response.getBody();
      auto arr = new Array<uchar>(message.size());
      memcpy(arr->data(), &message.front(), message.size());
      return arr;
    }

    String ToString() const override { return host + "/" + item; }
  };

  DERIVED_IMPLEMENT(LocationWeb)
}

namespace LTE
{
  HashT LocationT::GetHash() const { return String_Hash(ReadAscii()); }

  String LocationT::ReadAscii() const
  {
    std::unique_ptr<Array<uchar> > arr(Read().release());
    String str;
    if (!arr)
      return str;

    str.reserve(arr->size());
    for (size_t i = 0; i < arr->size(); ++i)
    {
      char c = (*arr)[i];
      if (c != '\r')
        str.push_back(c);
    }
    return str;
  }

  DefineFunction(Location_Cache) { return Location_File(OS_GetUserDataPath() + "cache/" + args.name); }

  DefineFunction(Location_File) { return new LocationFile(args.file); }

  Location Location_Memory(const String& data) { return new LocationMemory(data); }

  Location Location_Memory(Array<uchar>* memory, bool ownsMemory) { return new LocationMemory(memory, ownsMemory); }

  DefineFunction(Location_Resource) { return new LocationResource(args.name); }

  DefineFunction(Location_Web) { return new LocationWeb(args.host, args.file); }
}
