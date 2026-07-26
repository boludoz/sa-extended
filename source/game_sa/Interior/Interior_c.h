#pragma once

#include "Base.h"

#include "rwplcore.h" // RwMatrix

#include "Vector.h"
#include "NodeAddress.h"
#include "InteriorInfo_t.h"
#include "List_c.h"
#include "ListItem_c.h"
#include "FurnitureEntity_c.h"
#include "InteriorGroup_c.h"

class CEntity;
class CObject;
class Furniture_c;
class InteriorGroup_c;

//! Grid size of an interior's floor plan, in tiles
static constexpr size_t NUM_INTERIOR_TILES = 30;

/// Occupancy state of one floor tile.
/// Only `TILE_EMPTY` has a confirmed meaning; the rest are placement categories
/// whose original names were never recovered, so they are left numbered.
enum eTileStatus { // 4 bytes wide, as the original passes it
    TILE_EMPTY = 0,
    TILE_STATE_1,
    TILE_STATE_2,
    TILE_STATE_3,
    TILE_STATE_4,
    TILE_STATE_5,
    TILE_STATE_6,
    TILE_STATE_7,
    TILE_STATE_8,
    TILE_STATE_9,
    TILE_STATE_10,
    TILE_STATE_11,
};

/// Which wall of the room a piece of furniture is placed against.
enum eInteriorWall {
    WALL_X_A = 0,
    WALL_Y_A,
    WALL_X_B,
    WALL_Y_B,
};

/// Furniture group of an interior, mirroring `tEffectInterior::m_type`.
enum eInteriorGroup {
    IG_SHOP = 0,
    IG_OFFICE,
    IG_LOUNGE,
    IG_BEDROOM,
    IG_KITCHEN,
};

/// A point the peds inside route through, pinned to a floor tile.
struct GotoPt_t {
    int8    tileX;
    int8    tileY;
    int8    link1;
    int8    link2;
    CVector pos;
};
VALIDATE_SIZE(GotoPt_t, 0x10);

class Interior_c : public ListItem_c<Interior_c> {
public:
    int32                      m_id;                                        // 0x8
    InteriorGroup_c*           m_pInteriorGroup;                            // 0xC
    int32                      m_areaCode;                                  // 0x10
    tEffectInterior*           m_box;                                       // 0x14
    RwMatrix                   m_mat;                                       // 0x18
    float                      m_distSq;                                    // 0x58
    TList_c<FurnitureEntity_c> m_furnitureEntityList;                       // 0x5C
    uint8                      m_tiles[NUM_INTERIOR_TILES][NUM_INTERIOR_TILES]; // 0x68
    CNodeAddress               m_exitAddr;                                  // 0x3EC
    CNodeAddress               m_doorAddr;                                  // 0x3F0
    CVector                    m_exitPos;                                   // 0x3F4
    CVector                    m_doorPos;                                   // 0x400
    int8                       m_numGotoPts;                                // 0x40C
    int8                       m_numInteriorInfos;                          // 0x40D
    GotoPt_t                   m_gotoPts[16];                               // 0x410
    GotoPt_t                   m_exitPts[8];                                // 0x510
    InteriorInfo_t             m_interiorInfos[16];                         // 0x590
    int8                       m_shopSubType;                               // 0x790
    int8                       m_style;                                     // 0x791
    int8                       m_style2;                                    // 0x792

public:
    static void InjectHooks();

    Interior_c() = default;
    ~Interior_c() = default; // 0x591360

    int32 Init(const CVector& pos);
    void Exit();

    CObject* Bedroom_AddTableItem(int32 furnitureGroupId, int32 furnitureSubgroupId, int32 wallId, int32 tileX, int32 tileY, int32 dir);
    void FurnishBedroom();
    CObject* Kitchen_FurnishEdges();
    void FurnishKitchen();
    CObject* Lounge_AddTV(int32 wallId, int32 unusedA, int32 unusedB, int32 unusedC);
    CObject* Lounge_AddHifi(int32 wallId, int32 tileX, int32 tileY, int32 unused);
    void Lounge_AddChairInfo(int32 wallId, int32 offset, CEntity* entityIgnoredCollision);
    void Lounge_AddSofaInfo(int32 wallId, int32 offset, CEntity* entityIgnoredCollision);
    void FurnishLounge();
    bool Office_PlaceEdgeFillers(int32 arg0, int32 a2, int32 a3, int32 a6, int32);
    int32 Office_PlaceDesk(int32 a3, int32 arg4, int32 offsetY, int32 a5, uint8 a6, int32 b);
    int32 Office_PlaceEdgeDesks(int32 unused, int32 tileX, int32 tileY, int32 wallId, int32 placement);
    void Office_FurnishEdges();
    int32 Office_PlaceDeskQuad(int32 unused, int32 tileX, int32 tileY, int32 style);
    int32 Office_FurnishCenter();
    void FurnishOffice();
    int8 Shop_Place3PieceUnit(int32 type, int32 tileX, int32 tileY, int32 wallId, int32 length);
    int32 Shop_PlaceEdgeUnits(int32 unitType, int32 tileX, int32 tileY, int32 wallId);
    int32 Shop_PlaceCounter(bool leftOfDoor);
    void Shop_PlaceFixedUnits();
    void Shop_FurnishCeiling();
    void Shop_AddShelfInfo(int32 tileX, int32 tileY, int32 dir);
    void Shop_FurnishEdges();
    /// Corners of the furniture block `entity` belongs to, grown by a ped radius.
    bool GetBoundingBox(FurnitureEntity_c* entity, CVector* outCorners);
    void ResetTiles();
    CObject* PlaceObject(uint8 isStealable, Furniture_c* furniture, float offsetX, float offsetY, float offsetZ, float rotationZ);
    FurnitureEntity_c* GetFurnitureEntity(CEntity*);
    bool IsPtInside(const CVector& pt, CVector bias = {});
    void CalcMatrix(CVector* translation);
    void Furnish();
    void Unfurnish();
    /// Whether every tile of the `w` x `d` block at (`tileX`, `tileY`) is free.
    bool CheckTilesEmpty(int32 tileX, int32 tileY, int32 w, int32 d, bool allowState9);
    void SetTilesStatus(int32 tileX, int32 tileY, int32 w, int32 d, eTileStatus status, bool force);
    /// Mark an L-shaped run of tiles along the two walls meeting at `corner`.
    void SetCornerTiles(int32 corner, int32 size, eTileStatus status, bool force);
    int32 GetTileStatus(int32 x, int32 y);
    /// How many consecutive free `runLength`-deep slots run along a wall from a tile.
    int32 GetNumEmptyTiles(int32 tileX, int32 tileY, int32 wallId, int32 runLength);
    /// Pick a random tile with the given status. Loops forever if none has it.
    int32 GetRandomTile(eTileStatus status, int32& outTileX, int32& outTileY);
    void Shop_FurnishAisles();
    /// World-space centre of the given floor tile.
    void GetTileCentre(float tileX, float tileY, CVector& outCentre);
    void AddGotoPt(int32 tileX, int32 tileY, float offsetX, float offsetY);
    bool AddInteriorInfo(eInteriorInfoType actionType, float offsetX, float offsetY, int32 direction, CEntity* entityIgnoredCollision);
    void AddPickups();
    /// Flood fill from a tile over its connected furniture block, growing the bounds.
    void FindBoundingBox(int32 tileX, int32 tileY, int32* minX, int32* maxX, int32* minY, int32* maxY, int32* visited);
    void CalcExitPts();
    bool IsVisible();
    void PlaceFurniture(Furniture_c* a1, int32 a2, int32 a3, float a4, int32 a5, int32 a6, int32* a7, int32* a8, uint8 a9);
    CObject* PlaceFurnitureOnWall(int32 furnitureGroupId, int32 furnitureSubgroupId, int32 furnitureId, float a5, int32 a6, int32 a7, int32 a8, int32 a9, int32* a10, int32* a11,
                              int32* a12, int32* a13, int32* a14, int32* a15);
    void PlaceFurnitureInCorner(int32 furnitureGroupId, int32 furnitureSubgroupId, int32 id, float a4, int32 a5, int32 a6, int32 a2, int32* a9, int32* a10, int32* a11, int32* a12,
                                int32* a13);
    /// Try 100 random spots for a free `w` x `d` block.
    bool FindEmptyTiles(int32 w, int32 d, int32& outTileX, int32& outTileY);
    void FurnishShop(int32 shopSubType);

    auto GetNodeAddress() const { return m_doorAddr; }
};
VALIDATE_SIZE(Interior_c, 0x794);
