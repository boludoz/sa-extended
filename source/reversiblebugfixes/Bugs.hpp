#pragma once

#include "ReversibleBugFix.hpp"

/*!
* This is where the bugs are defined.
* The usual naming convention is `ClassName_Function_BugName` (`BugName` is something unique you come up with).
* Unless the same bug is spanning across multiple functions.
* 
* Fixes that prevent OOB should use `GenericOOB`.
* Fixes that prevent crashes (Like additional null ptr checks) can use `GenericCrash`.
* Feel free to add a new generic bug type if there's nothing that fits your needs.
*/
namespace notsa::bugfixes {
//
// Generic
//
inline const ReversibleBugFix GenericCrashing{
    .Name        = "Generic crashes (null ptr access, etc)",
    .Description = "Fixes bugs that cause null ptr access, and similar anomalies crashing the game",
    .Credit      = "Contributors"
};
inline const ReversibleBugFix GenericUB{
    .Name        = "Generic undefined behaviour (Use-after-free, etc)",
    .Description = "Fixes game crashes/bugs",
    .Credit      = "Contributors"
};
inline const ReversibleBugFix GenericOOB{
    .Name        = "Generic out-of-bounds bugs",
    .Description = "Fixes generic out-of-bounds bugs across the codebase",
    .Credit      = "Contributors"
};


//
// Other bugs
//
inline const ReversibleBugFix PS2CoronaRendering{
    .Name        = "PS2 Corona Rendering",
    .Description = "Fix corona rendering, so they're like on PS2",
    .Credit      = "SilentPatch Contributors"
};
inline const ReversibleBugFix AnimBlendSequence_SetName_SetBoneTagFlag{
    .Name        = "BoneTag Name Flag",
    .Description = "Correctly set BoneTag flag in `CAnimBlendSequence::SetName`",
    .Credit      = "Pirulax"
};
inline const ReversibleBugFix AESound_UpdatePlayTime_DivisionByZero{
    .Name        = "UpdatePlayTime Division-By-Zero",
    .Description = "Avoid Division-by-zero in CAESound::UpdatePlayTime",
    .Credit      = "Pirulax"
};
inline const ReversibleBugFix Interior_c_Shop_PlaceFixedUnits_UninitCounterX{
    .Name        = "Shop_PlaceFixedUnits Uninitialised Counter Position",
    .Description = "When a shop has room for a counter on neither side of its door, the original never assigns the "
                   "counter's X tile yet still uses it to place two interior markers. The x86 build reads the stack "
                   "slot holding `this`, throwing the markers millions of tiles out; the ARM64 build reads 0. "
                   "Use 0, matching ARM64.",
    .Credit      = "Contributors"
};
inline const ReversibleBugFix CCarCtrl_RemoveDistantCars_UseAfterFree{
    .Name        = "CCarCtrl::RemoveDistantCars Use-After-Free",
    .Description = "Fix user-after-free of vehicles (possibly) deleted by PossiblyRemoveVehicle",
    .Credit      = "Pirulax"
};
};
