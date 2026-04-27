#include "../Tasks.h"

#include "Game/Events.h"
#include "Game/Icons.h"
#include "Game/Object.h"
#include "Game/Item/Blueprint.h"

#include "LTE/Math.h"
#include "LTE/Pool.h"

namespace {
  AutoClassDerived(TaskMint, TaskT,
    Task_Mint_Args, args)
    DERIVED_TYPE_EX(TaskMint)
    POOLED_TYPE

    TaskMint() {}

    float GetDuration() const {
      return args.blueprint->GetValue();
    }

    Icon GetIcon() const {
      return Icon_Disc();
    }

    String GetName() const {
      return "Mint Assembly Chip";
    }

    String GetNoun() const {
      return "Assembly Chip Lab";
    }

    Capability GetRateFactor() const {
      return Capability_Research(1);
    }

    void OnUpdate(Object const& self, float dt, Data& data) {
      if (RandExp() < dt) {
        Blueprint* bp = (Blueprint*)args.blueprint.get();
        self->GetRoot()->AddItem(bp->assemblyChip, 1);
      }
    }
  };
}

DefineFunction(Task_Mint) {
  return new TaskMint(args);
}
