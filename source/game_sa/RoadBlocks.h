#pragma once

#include "Vector.h"
#include "NodeAddress.h"

class CVehicle;
class CPed;

//! Number of path nodes stored in `data/paths/roadblox.dat`
static constexpr size_t NUM_ROADBLOX_NODES = 325;

/// A roadblock requested by a script, waiting to be turned into actual props.
struct CScriptRoadBlock {
    CVector Coors1;
    CVector Coors2;
    bool    bActive;
    bool    bSafeToCreate;  ///< The player is far enough away for the props to pop in unseen
    bool    bGangRoadBlock;
    uint8   _pad;
};
VALIDATE_SIZE(CScriptRoadBlock, 0x1C);

class CRoadBlocks {
public:
    static constexpr size_t NUM_SCRIPT_ROADBLOCKS = 16;

    static inline auto& aScriptRoadBlocks          = StaticRef<std::array<CScriptRoadBlock, NUM_SCRIPT_ROADBLOCKS>>(0xA43AB8);
    static inline auto& bGenerateDynamicRoadBlocks = StaticRef<bool>(0xA43584);

    /// Whether each node of `RoadBlockNodes` is currently in range of the player.
    static inline auto& InOrOut         = StaticRef<std::array<bool, NUM_ROADBLOX_NODES>>(0xA43438);
    static inline auto& NumRoadBlocks   = StaticRef<int32>(0xA43580);
    static inline auto& RoadBlockNodes  = StaticRef<std::array<CNodeAddress, NUM_ROADBLOX_NODES>>(0xA435A0);

    static void InjectHooks();

    static void Init();
    static void ClearScriptRoadBlocks();

    /// Delete whatever sits between the two corners so a roadblock prop can be placed there.
    /// Returns false when something that must not be deleted is in the way.
    static bool ClearSpaceForRoadBlockObject(CVector cornerA, CVector cornerB);

    static void CreateRoadBlockBetween2Points(CVector from, CVector to, bool isGangRoadBlock);
    static void GenerateRoadBlockCopsForCar(CVehicle* vehicle, int32 a2, ePedType pedType);
    static void GenerateRoadBlocks();

    /// Width of, and direction across, the road at the given path node.
    static bool GetRoadBlockNodeInfo(CNodeAddress nodeAddr, float& outWidth, CVector& outDir);

    static void RegisterScriptRoadBlock(CVector cornerA, CVector cornerB, bool isGangRoadBlock);
};
