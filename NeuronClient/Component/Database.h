#ifndef Component_Database_h__
#define Component_Database_h__

#include "Common.h"
#include "Game/Item.h"
#include "LTE/AutoClass.h"
#include "LTE/Map.h"

using DatabaseMapT = Map<Item, Quantity>;
using DatabaseIter = DatabaseMapT::iterator;
using DatabaseIterC = DatabaseMapT::const_iterator;

AutoClass(ComponentDatabase, DatabaseMapT, elements)

  ComponentDatabase() {}

  ~ComponentDatabase()
  {
    // CRITICAL
    // for (DatabaseIter it = elements.begin(); it != elements.end(); ++it)
    //  it->first->copies -= it->second;
  }

  void Mod(const Item& data, Quantity quantity)
  {
    // data->copies += quantity;
    Quantity& q = elements[data];
    q += quantity;
    if (q == 0)
      elements.erase(data);
  }

  Quantity GetCount(const Item& item) const
  {
    const Quantity* count = elements.get(item);
    return count ? *count : 0;
  }
};

AutoComponent(Database)

  bool AddItem(const Item& item, Quantity count, bool force = false)
  {
    if (item->GetStorageType() == StorageType_Database)
    {
      Database.Mod(item, count);
      return true;
    }
    return BaseT::AddItem(item, count, force);
  }

  Quantity GetItemCount(const Item& item) const
  {
    return item->GetStorageType() == StorageType_Database ? Database.GetCount(item) : BaseT::GetItemCount(item);
  }
};

#endif
