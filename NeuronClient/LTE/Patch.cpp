#include "Patch.h"
#include "Array.h"
#include "AutoPtr.h"
#include "Diff.h"
#include "Location.h"
#include "ProgramLog.h"

#include <memory>
#include <utility>
#include <vector>

namespace {
  struct PatchEntry {
    Location loc;
    std::unique_ptr<Diff> diff;

    bool Apply() const {
      std::unique_ptr<Array<uchar> > srcData(
        loc->Exists() ? loc->Read().release() : new Array<uchar>);
      std::unique_ptr<Array<uchar> > dstData(diff->Inflate(*srcData));
      if (!dstData) {
        Log_Error("Source " + loc->ToString() + " not found");
        return false;
      }
      return loc->Write(*dstData);
    }
  };

  struct PatchImpl : public Patch {
    std::vector<PatchEntry> entries;

    void Add(Location const& target, Location const& patchFile) {
      std::unique_ptr<Array<uchar> > srcData(
        target->Exists() ? target->Read().release() : new Array<uchar>);
      std::unique_ptr<Array<uchar> > dstData(patchFile->Read().release());

      PatchEntry e;
      e.loc = target->Clone();
      e.diff.reset(Diff_Create(*srcData, *dstData).release());
      entries.push_back(std::move(e));
    }

    bool Apply() const {
      std::vector<std::unique_ptr<Array<uchar> > > backups(entries.size());

      for (size_t i = 0; i < entries.size(); ++i) {
        backups[i].reset(entries[i].loc->Read().release());
        if (!entries[i].Apply()) {
          Log_Warning("Patch application failed, attempting to restore");
          for (size_t j = 0; j < i; ++j) {
            if (backups[j]) {
              if (entries[j].loc->Write(*backups[j]))
                Log_Warning("Successfully restored " + entries[j].loc->ToString());
              else
                Log_Warning("Failed to restore file " + entries[j].loc->ToString());
            }
          }
          return false;
        }
      }
      return true;
    }
  };
}

namespace LTE {
  Patch* Patch_Create() {
    return new PatchImpl;
  }
}
