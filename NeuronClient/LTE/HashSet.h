#ifndef LTE_HashSet_h__
#define LTE_HashSet_h__

#include <unordered_set>

template <class KeyT>
struct HashSetT {
  typedef std::unordered_set<KeyT> result;
};

namespace LTE {
  template <class KeyT>
  struct HashSet : public HashSetT<KeyT>::result {
    bool contains(KeyT const& key) const {
      return this->find(key) != this->end();
    }
  };
}

#endif
