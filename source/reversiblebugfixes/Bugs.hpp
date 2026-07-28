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
inline const ReversibleBugFix GenericFrameRate{
    .Name        = "Generic framerate related bugs",
    .Description = "Fixes parts of code to not be framerate dependent",
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
inline const ReversibleBugFix CTimeCycle_ExtraColour_NegativeIndex{
    .Name        = "Extra Colour Negative Table Index",
    .Description = "Opcode 04F9 (`set_interior <n> color_to <flag>`) passes `n - 1` to "
                   "`CTimeCycle::StartExtraColour` with no check for `n == 0`, which is how the stock "
                   "scripts switch the extra colour off. That leaves `m_ExtraColour` at -1, and every "
                   "timecycle table is then read at `[-1][21]`, two bytes before the array. Vanilla gets "
                   "away with it because `m_ExtraColourInter` is fading to zero at the same time, so the "
                   "garbage is blended in at ~0 weight. Clamp the indices to the table instead.",
    .Credit      = "Contributors"
};
inline const ReversibleBugFix CCarCtrl_RemoveDistantCars_UseAfterFree{
    .Name        = "CCarCtrl::RemoveDistantCars Use-After-Free",
    .Description = "Fix user-after-free of vehicles (possibly) deleted by PossiblyRemoveVehicle",
    .Credit      = "Pirulax"
};
inline const ReversibleBugFix CAEVehicleAudioEntity_PlayBicycleSound_VolumeFix{
    .Name        = "CAEVehicleAudioEntity::PlayBicycleSound Volume fix",
    .Description = "Original code didn't account for event base volume",
    .Credit      = "Pirulax"
};
inline const ReversibleBugFix CAEVehicleAudioEntity_PlayAircraftSound_VolumeFix{
    .Name        = "CAEVehicleAudioEntity::PlayAircraftSound Volume fix",
    .Description = "Original code didn't account for event base volume",
    .Credit      = "Pirulax"
};
inline const ReversibleBugFix CPathFind_SwitchRoadsOffInArea_StrayAreas{
    .Name        = "CPathFind::SwitchRoadsOffInArea Stray-Areas",
    .Description = "Fix multiple issues related to saving unused path areas after missions",
    .Credit      = "Contributors"
};
inline const ReversibleBugFix CCustomCarPlateMgr_GeneratePlateText_MissingLettersAndDigits{
    .Name        = "CCustomCarPlateMgr::GeneratePlateText - Expand plate character range",
    .Description = "The original game generates plate letters in the range A-W (23 chars) and digits in 0-8 (9 chars). "
    "This fix expands those ranges to A-Z (26 chars) and 0-9 (10 chars) for more variety.",
    .Credit      = "j0y"
};
inline const ReversibleBugFix CAEVehicleAudioEntity_Terminate_DanglingAudioEntity{
    .Name        = "CAEVehicleAudioEntity::Terminate - Dangling `CAESound::m_AudioEntity`",
    .Description = "`Terminate` only stops the sounds whose handle the entity kept. `AddAudioEvent` "
                   "(AE_BONNET_FLUBBER_FLUBBER) plays a `SOUND_REQUEST_UPDATES` sound tied to the flying "
                   "component's lifespan and throws the handle away, so that sound outlives the vehicle and "
                   "`CAESound::UpdateParameters` keeps making a virtual call into the freed pool slot. "
                   "Detach every remaining sound from the entity instead of leaving the pointer dangling.",
    .Credit      = "Contributors"
};
inline const ReversibleBugFix CAECollisionAudioEntity_PlayLoopingCollisionSound_InvalidSurfaceType{
    .Name        = "CAECollisionAudioEntity::PlayLoopingCollisionSound - Invalid surface type causing OOB",
    .Description = "The surface type passed in could've been an `AE_SURFACE_TYPE_*` which when passed to `g_surfaceInfos` causes an OOB. "
                   "The fix ensures the surface type is valid before accessing `g_surfaceInfos`.",
    .Credit      = "Pirulax"
};
};
