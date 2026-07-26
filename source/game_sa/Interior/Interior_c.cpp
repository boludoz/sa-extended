#include "StdInc.h"
#include "Interior_c.h"

#include "FurnitureManager_c.h"
#include "Furniture_c.h"

#include <reversiblebugfixes/Bugs.hpp>
#include "Pickups.h"
#include "PedGeometryAnalyser.h"
#include "Camera.h"
#include "InteriorManager_c.h"
#include "ModelIndices.h"

void Interior_c::InjectHooks() {
    RH_ScopedClass(Interior_c);
    RH_ScopedCategory("Interior");

    //RH_ScopedInstall(Constructor, 0x5921D0, { .reversed = false });
    //RH_ScopedInstall(Destructor, 0x591360, { .reversed = false });

    RH_ScopedInstall(Bedroom_AddTableItem, 0x593F10);
    RH_ScopedInstall(FurnishBedroom, 0x593FC0);
    RH_ScopedInstall(Kitchen_FurnishEdges, 0x596930, { .reversed = false });
    RH_ScopedInstall(FurnishKitchen, 0x5970B0);
    RH_ScopedInstall(Lounge_AddTV, 0x597240);
    RH_ScopedInstall(Lounge_AddHifi, 0x597430);
    RH_ScopedInstall(Lounge_AddChairInfo, 0x5974E0);
    RH_ScopedInstall(Lounge_AddSofaInfo, 0x5975C0);
    RH_ScopedInstall(FurnishLounge, 0x597740, { .reversed = false });
    RH_ScopedInstall(Office_PlaceEdgeFillers, 0x599210, { .reversed = false });
    RH_ScopedInstall(Office_PlaceDesk, 0x5993E0, { .reversed = false });
    RH_ScopedInstall(Office_PlaceEdgeDesks, 0x5995B0);
    RH_ScopedInstall(Office_FurnishEdges, 0x599770);
    RH_ScopedInstall(Office_PlaceDeskQuad, 0x599960);
    RH_ScopedInstall(Office_FurnishCenter, 0x599A30);
    RH_ScopedInstall(FurnishOffice, 0x599AF0);
    RH_ScopedInstall(Shop_Place3PieceUnit, 0x599BB0);
    RH_ScopedInstall(Shop_PlaceEdgeUnits, 0x599DC0);
    RH_ScopedInstall(Shop_PlaceCounter, 0x599EF0);
    RH_ScopedInstall(Shop_PlaceFixedUnits, 0x59A030);
    RH_ScopedInstall(Shop_FurnishCeiling, 0x59A130);
    RH_ScopedInstall(Shop_AddShelfInfo, 0x59A140);
    RH_ScopedInstall(Shop_FurnishEdges, 0x59A1B0, { .reversed = false });
    RH_ScopedInstall(GetBoundingBox, 0x593DB0);
    RH_ScopedInstall(Init, 0x593BF0);
    RH_ScopedInstall(ResetTiles, 0x593910);
    RH_ScopedInstall(PlaceObject, 0x5934E0);
    RH_ScopedInstall(GetFurnitureEntity, 0x5913B0);
    RH_ScopedInstall(IsPtInside, 0x5913E0);
    RH_ScopedInstall(CalcMatrix, 0x5914D0);
    RH_ScopedInstall(Furnish, 0x591590);
    RH_ScopedInstall(Unfurnish, 0x5915D0);
    RH_ScopedInstall(CheckTilesEmpty, 0x591680);
    RH_ScopedInstall(SetTilesStatus, 0x591700);
    RH_ScopedInstall(SetCornerTiles, 0x5917C0);
    RH_ScopedInstall(GetTileStatus, 0x5918E0);
    RH_ScopedInstall(GetNumEmptyTiles, 0x591920);
    RH_ScopedInstall(GetRandomTile, 0x591B20);
    RH_ScopedInstall(Shop_FurnishAisles, 0x59A590, { .reversed = false });
    RH_ScopedInstall(GetTileCentre, 0x591BD0);
    RH_ScopedInstall(AddGotoPt, 0x591D20);
    RH_ScopedInstall(AddInteriorInfo, 0x591E40);
    RH_ScopedInstall(AddPickups, 0x591F90);
    RH_ScopedInstall(Exit, 0x592230);
    RH_ScopedInstall(FindBoundingBox, 0x5922C0);
    RH_ScopedInstall(CalcExitPts, 0x5924A0, { .reversed = false });
    RH_ScopedInstall(IsVisible, 0x5929F0);
    RH_ScopedInstall(PlaceFurniture, 0x592AA0, { .reversed = false });
    RH_ScopedInstall(PlaceFurnitureOnWall, 0x593120, { .reversed = false });
    RH_ScopedInstall(PlaceFurnitureInCorner, 0x593340, { .reversed = false });
    RH_ScopedInstall(FindEmptyTiles, 0x591C50);
    RH_ScopedInstall(FurnishShop, 0x59A790);
}

// 0x593BF0
int32 Interior_c::Init(const CVector& pos) {
    CalcMatrix(const_cast<CVector*>(&pos));
    ResetTiles();

    auto* const groupEntity = m_pInteriorGroup->GetEntity();
    if (!groupEntity->m_matrix) {
        groupEntity->AllocateMatrix();
        groupEntity->m_placement.UpdateMatrix(groupEntity->m_matrix);
    }
    const CMatrix groupMat{ *groupEntity->m_matrix };

    // Seed the RNG from where the room sits in the world, so the same room is
    // always furnished the same way. Type 99 rooms are laid out by hand.
    if (m_box->m_type != 99) {
        const auto& p = groupMat.GetPosition();
        auto seed = (uint32)(m_box->m_seed + p.x * p.y * p.z);
        if (const auto* const enex = g_interiorMan.m_EnEx) {
            seed = (uint32)(m_box->m_seed + p.x * p.y * p.z
                          + enex->m_recEntrance.left * enex->m_recEntrance.top * enex->m_fEntranceZ);
        }
        srand(seed);
    }

    m_numGotoPts       = 0;
    m_numInteriorInfos = 0;

    switch (m_box->m_type) {
    case IG_SHOP:    FurnishShop(0);   break;
    case IG_OFFICE:  FurnishOffice();  break;
    case IG_LOUNGE:  FurnishLounge();  break;
    case IG_BEDROOM: FurnishBedroom(); break;
    case IG_KITCHEN: FurnishKitchen(); break;
    }

    CalcExitPts();

    if (!g_interiorMan.HasInteriorHadStealDataSetup(this) && g_interiorMan.m_InteriorCount < 64) {
        g_interiorMan.m_InteriorIds[g_interiorMan.m_InteriorCount++] = m_id;
    }

    if ((m_box->m_type & 0xFE) == 2) { // Lounges and bedrooms get loose pickups
        AddPickups();
    }

    return 1;
}

// 0x592230
void Interior_c::Exit() {
    const auto& pos = m_mat.pos;
    CPickups::RemovePickUpsInArea(pos.x - 50.0f, pos.x + 50.0f, pos.y - 50.0f, pos.y + 50.0f, pos.z - 50.0f, pos.z + 50.0f);
    Unfurnish();
}

// 0x593F10
CObject* Interior_c::Bedroom_AddTableItem(int32 furnitureGroupId, int32 furnitureSubgroupId, int32 wallId, int32 tileX, int32 tileY, int32 dir) {
    // Shift half a tile along whichever axis the wall runs, so the item ends up
    // against it rather than centred on the tile
    auto x = (float)tileX;
    auto y = (float)tileY;
    if (wallId == 0 || wallId == 2) {
        x += 0.5f;
    } else if (wallId == 1 || wallId == 3) {
        y += 0.5f;
    }

    return PlaceObject(
        true,
        g_furnitureMan.GetFurniture(furnitureGroupId, furnitureSubgroupId, -1, m_box->m_status),
        x + 0.5f,
        y + 0.5f,
        0.5f,
        (float)dir * 90.0f
    );
}

// 0x593FC0
void Interior_c::FurnishBedroom() {
    m_style = (int8)g_furnitureMan.GetRandomId(IG_BEDROOM, 1, m_box->m_status);

    SetTilesStatus(m_box->m_door - 1, 0, 2, 2, TILE_STATE_7, false);

    // The bed goes down first; the rest of the room is arranged around it
    int32 bedWall{}, bedPos{};
    auto* const bed = PlaceFurnitureOnWall(IG_BEDROOM, 0 /* beds */, -1, 0.0f, WALL_Y_A, -1, -1, 0, &bedWall, &bedPos, nullptr, nullptr, nullptr, nullptr);

    // A bedside unit on the near side of the bed, with a marker to walk up to it
    if (bedPos > 0 && PlaceFurnitureOnWall(IG_BEDROOM, 1, m_style, 0.0f, WALL_Y_A, bedWall, bedPos - 1, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)) {
        eInteriorInfoType infoType{};
        int32 dir{}, tileX{}, tileY{};
        float infoX{}, infoY{};
        switch (bedWall) {
        case WALL_X_A: infoType = eInteriorInfoType::LIEINBED_LEFT; dir = 1; infoX = (float)(bedPos - 1);        infoY = (float)(m_box->m_depth - 2); tileX = bedPos - 1;        tileY = m_box->m_depth - 3; break;
        case WALL_Y_A: infoType = eInteriorInfoType::LIEINBED_LEFT; dir = 2; infoX = 1.0f;                       infoY = (float)(bedPos - 1);         tileX = 2;                 tileY = bedPos - 1;         break;
        case WALL_X_B: infoType = eInteriorInfoType::LIEINBED_RIGHT; dir = 1; infoX = (float)(bedPos - 1);        infoY = 1.0f;                        tileX = bedPos - 1;        tileY = 2;                  break;
        case WALL_Y_B: infoType = eInteriorInfoType::LIEINBED_RIGHT; dir = 2; infoX = (float)(m_box->m_width - 2); infoY = (float)(bedPos - 1);        tileX = m_box->m_width - 3; tileY = bedPos - 1;        break;
        }
        AddInteriorInfo(infoType, infoX, infoY, dir, bed);
        SetTilesStatus(tileX, tileY, 1, 1, TILE_STATE_2, false);
    }

    // ...and one on the far side, facing the opposite way
    if (PlaceFurnitureOnWall(IG_BEDROOM, 1, m_style, 0.0f, WALL_Y_A, bedWall, bedPos + 2, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)) {
        eInteriorInfoType infoType{};
        int32 dir{}, tileX{}, tileY{};
        float infoX{}, infoY{};
        switch (bedWall) {
        case WALL_X_A: infoType = eInteriorInfoType::LIEINBED_RIGHT; dir = 3; infoX = (float)(bedPos + 2);        infoY = (float)(m_box->m_depth - 2); tileX = bedPos + 2;        tileY = m_box->m_depth - 3; break;
        case WALL_Y_A: infoType = eInteriorInfoType::LIEINBED_RIGHT; dir = 0; infoX = 1.0f;                       infoY = (float)(bedPos + 2);         tileX = 2;                 tileY = bedPos + 2;         break;
        case WALL_X_B: infoType = eInteriorInfoType::LIEINBED_LEFT; dir = 3; infoX = (float)(bedPos + 2);        infoY = 1.0f;                        tileX = bedPos + 2;        tileY = 2;                  break;
        case WALL_Y_B: infoType = eInteriorInfoType::LIEINBED_LEFT; dir = 0; infoX = (float)(m_box->m_width - 2); infoY = (float)(bedPos + 2);        tileX = m_box->m_width - 3; tileY = bedPos + 2;        break;
        }
        AddInteriorInfo(infoType, infoX, infoY, dir, bed);
        SetTilesStatus(tileX, tileY, 1, 1, TILE_STATE_2, false);
    }

    PlaceFurnitureOnWall(IG_BEDROOM, 3, m_style, 0.0f, WALL_Y_A, -1, -1, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    PlaceFurnitureOnWall(IG_BEDROOM, 2, m_style, 0.0f, WALL_Y_A, -1, -1, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    // Nought to two small items on top of whatever sits against a wall
    const auto AddTableItem = [this](int32 subGroup) {
        int32 wall{}, tileX{}, tileY{};
        if (PlaceFurnitureOnWall(2, 6, -1, 0.0f, WALL_Y_A, -1, -1, 0, &wall, nullptr, &tileX, &tileY, nullptr, nullptr)) {
            Bedroom_AddTableItem(2, subGroup, wall, tileX, tileY, wall);
        }
    };
    const auto roll = (int32)CGeneral::GetRandomNumberInRange(0.0f, 100.0f);
    if (roll < 25) {
        AddTableItem(8);
    } else if (roll < 50) {
        AddTableItem(3);
    } else if (roll < 75) {
        AddTableItem(8);
        AddTableItem(3);
    }

    // Poorer bedrooms get more junk lying about on the floor
    const auto wealth = m_box->m_status;
    const auto clutter = wealth >= 75 ? (int32)CGeneral::GetRandomNumberInRange(0.0f, 20.0f)
                       : wealth >= 50 ? 20 + (int32)CGeneral::GetRandomNumberInRange(0.0f, 30.0f)
                                      : CGeneral::GetRandomNumberInRange(50, 100);

    const auto PlaceClutter = [this, clutter](float chanceOver, int32 subGroup, int32 findW, int32 findD, int32 markW, int32 markD) {
        if ((int32)CGeneral::GetRandomNumberInRange(0.0f, chanceOver) >= clutter) {
            return;
        }
        int32 tileX, tileY;
        if (!FindEmptyTiles(findW, findD, tileX, tileY)) {
            return;
        }
        PlaceObject(false, g_furnitureMan.GetFurniture(8, subGroup, -1, m_box->m_status), (float)tileX + 0.5f, (float)tileY + 0.5f, 0.05f, 0.0f);
        SetTilesStatus(tileX, tileY, markW, markD, TILE_STATE_2, false);
    };
    PlaceClutter(60.0f, 2, 2, 2, 2, 2);
    PlaceClutter(100.0f, 5, 1, 1, 1, 1);
    PlaceClutter(100.0f, 4, 1, 1, 1, 1);
    PlaceClutter(100.0f, 3, 1, 1, 1, 1);
    PlaceClutter(100.0f, 6, 2, 2, 1, 1); // NOTE (original): finds a 2x2 spot but only marks 1x1 as taken

    // A rug or similar in the middle of the room
    auto* const centrePiece = g_furnitureMan.GetFurniture(8, 1, -1, m_box->m_status);
    int32 outA, outB;
    PlaceFurniture(
        centrePiece,
        (int32)((float)m_box->m_width * 0.5f - (float)centrePiece->m_nWidthX * 0.5f),
        (int32)((float)m_box->m_depth * 0.5f - (float)centrePiece->m_nWidthY * 0.5f),
        0.0f, 0, 0, &outA, &outB, 0
    );
}

// 0x596930
CObject* Interior_c::Kitchen_FurnishEdges() {
    return plugin::CallMethodAndReturn<CObject*, 0x596930, Interior_c*>(this);
}

// 0x5970B0
void Interior_c::FurnishKitchen() {
    SetTilesStatus(m_box->m_door - 1, 0, 2, 1, TILE_STATE_7, false);

    const auto lastX = m_box->m_width - 2;
    const auto lastY = m_box->m_depth - 2;

    // Run counters along all four walls, one tile in from the edge
    for (auto x = 1; x <= lastX; x++) {
        SetTilesStatus(x, lastY, 1, 1, TILE_STATE_3, false);
        SetTilesStatus(x, 0, 1, 1, TILE_STATE_3, false);
    }
    for (auto y = 0; y <= lastY; y++) {
        SetTilesStatus(1, y, 1, 1, TILE_STATE_3, false);
        SetTilesStatus(lastX, y, 1, 1, TILE_STATE_3, false);
    }

    AddGotoPt(1, 1, 0.0f, 0.0f);
    AddGotoPt(1, lastY, 0.0f, 0.0f);
    AddGotoPt(lastX, 1, 0.0f, 0.0f);
    AddGotoPt(lastX, lastY, 0.0f, 0.0f);

    m_style = (int8)g_furnitureMan.GetRandomId(IG_KITCHEN, 0, m_box->m_status);
    Kitchen_FurnishEdges();

    // A table in the middle of the room
    auto* const table = g_furnitureMan.GetFurniture(8, 1, -1, m_box->m_status);
    int32 outA, outB;
    PlaceFurniture(
        table,
        (int32)((float)m_box->m_width * 0.5f - (float)table->m_nWidthX * 0.5f),
        (int32)((float)m_box->m_depth * 0.5f - (float)table->m_nWidthY * 0.5f),
        0.0f, 0, 0, &outA, &outB, 0
    );
}

// 0x597240
CObject* Interior_c::Lounge_AddTV(int32 wallId, int32 unusedA, int32 unusedB, int32 unusedC) {
    const auto w = (float)m_box->m_width;
    const auto d = (float)m_box->m_depth;

    // The TV goes in a corner at 45 degrees, with a second unit alongside it
    float tvX, tvY, sideX, sideY;
    switch (wallId) {
    case WALL_X_A:
        tvX = 0.5f;      tvY = d - 0.5f;
        sideX = 1.5f;    sideY = d - 0.5f;
        AddInteriorInfo(eInteriorInfoType::NONE, 1.0f, d - 2.0f, -1, nullptr);
        break;
    case WALL_Y_A:
        tvX = 0.5f;      tvY = 0.5f;
        sideX = 0.5f;    sideY = 1.5f;
        AddInteriorInfo(eInteriorInfoType::NONE, 1.0f, 1.0f, -1, nullptr);
        break;
    case WALL_X_B:
        tvX = w - 0.5f;  tvY = 0.5f;
        sideX = w - 1.5f; sideY = 0.5f;
        AddInteriorInfo(eInteriorInfoType::NONE, w - 2.0f, 1.0f, -1, nullptr);
        break;
    case WALL_Y_B:
        tvX = w - 0.5f;  tvY = d - 0.5f;
        sideX = w - 0.5f; sideY = d - 1.5f;
        AddInteriorInfo(eInteriorInfoType::NONE, w - 2.0f, d - 2.0f, -1, nullptr);
        break;
    default:
        return nullptr;
    }

    const auto rot = (float)(wallId & 3) * 90.0f;

    PlaceObject(true, g_furnitureMan.GetFurniture(IG_LOUNGE, 3, -1, m_box->m_status), tvX, tvY, 0.5f, rot + 45.0f);

    const auto sideSubGroup = CGeneral::GetRandomNumber() >= 0x3FFF ? 9 : 7;
    return PlaceObject(true, g_furnitureMan.GetFurniture(IG_LOUNGE, sideSubGroup, -1, m_box->m_status), sideX, sideY, 0.5f, rot);
}

// 0x597430
CObject* Interior_c::Lounge_AddHifi(int32 wallId, int32 tileX, int32 tileY, int32 unused) {
    auto x = (float)tileX;
    auto y = (float)tileY;
    if (wallId == 0 || wallId == 2) {
        x += 0.5f;
    } else if (wallId == 1 || wallId == 3) {
        y += 0.5f;
    }

    return PlaceObject(
        true,
        g_furnitureMan.GetFurniture(IG_LOUNGE, 8, -1, m_box->m_status),
        x + 0.5f,
        y + 0.5f,
        0.5f,
        (float)(wallId & 3) * 90.0f
    );
}

// 0x5974E0
void Interior_c::Lounge_AddChairInfo(int32 wallId, int32 offset, CEntity* entityIgnoredCollision) {
    // The chair faces away from the wall it stands against
    const auto dir = (wallId - 2) & 3;

    switch (wallId) {
    case 0: AddInteriorInfo(eInteriorInfoType::SITIN_CHAIR, (float)offset + 0.5f, (float)(m_box->m_depth - 1) - 1.0f, dir, entityIgnoredCollision); break;
    case 1: AddInteriorInfo(eInteriorInfoType::SITIN_CHAIR, 1.0f, (float)offset + 0.5f, dir, entityIgnoredCollision); break;
    case 2: AddInteriorInfo(eInteriorInfoType::SITIN_CHAIR, (float)offset + 0.5f, 1.0f, dir, entityIgnoredCollision); break;
    case 3: AddInteriorInfo(eInteriorInfoType::SITIN_CHAIR, (float)(m_box->m_width - 1) - 1.0f, (float)offset + 0.5f, dir, entityIgnoredCollision); break;
    }
}

// 0x5975C0
void Interior_c::Lounge_AddSofaInfo(int32 wallId, int32 offset, CEntity* entityIgnoredCollision) {
    // A sofa seats two, so drop a marker on each half. Both face away from the wall.
    const auto dir = (wallId - 2) & 3;
    const auto at  = (float)offset + 0.5f;

    const auto Seat = [&](float x, float y) {
        AddInteriorInfo(eInteriorInfoType::SITIN_CHAIR, x, y, dir, entityIgnoredCollision);
    };

    switch (wallId) {
    case WALL_X_A: {
        const auto y = (float)(m_box->m_depth - 1) - 1.0f;
        Seat(at, y);
        Seat(at + 1.0f, y);
        break;
    }
    case WALL_Y_A:
        Seat(1.0f, at);
        Seat(1.0f, at + 1.0f);
        break;
    case WALL_X_B:
        Seat(at, 1.0f);
        Seat(at + 1.0f, 1.0f);
        break;
    case WALL_Y_B: {
        const auto x = (float)(m_box->m_width - 1) - 1.0f;
        Seat(x, at);
        Seat(x, at + 1.0f);
        break;
    }
    }
}

// 0x597740
void Interior_c::FurnishLounge() {
    plugin::CallMethod<0x597740, Interior_c*>(this);
}

// 0x599210
bool Interior_c::Office_PlaceEdgeFillers(int32 arg0, int32 a2, int32 a3, int32 a6, int32 a7) {
    return plugin::CallMethodAndReturn<bool, 0x599210, Interior_c*, int32, int32, int32, int32, int32>(this, arg0, a2, a3, a6, a7);
}

// 0x5993E0
int32 Interior_c::Office_PlaceDesk(int32 a3, int32 arg4, int32 offsetY, int32 a5, uint8 a6, int32 b) {
    return plugin::CallMethodAndReturn<int32, 0x5993E0, Interior_c*, int32, int32, int32, int32, uint8, int32>(this, a3, arg4, offsetY, a5, a6, b);
}

// 0x5995B0
int32 Interior_c::Office_PlaceEdgeDesks(int32 unused, int32 tileX, int32 tileY, int32 wallId, int32 placement) {
    const auto roll   = (int32)CGeneral::GetRandomNumberInRange(0.0f, 100.0f);
    const auto chance = 30 + (int32)CGeneral::GetRandomNumberInRange(0.0f, 40.0f);

    const auto numFree = GetNumEmptyTiles(tileX, tileY, wallId == WALL_X_A || wallId == WALL_X_B ? 1 : 2, 1);
    if (numFree <= 1) {
        return 1;
    }

    // NOTE (original): two separate rolls -- one to compare against, one for the
    // value actually used. Kept as-is; folding them would shift every later roll.
    const auto count = numFree / 2 >= 2 + (int32)CGeneral::GetRandomNumberInRange(0.0f, 2.0f)
                     ? 2 + (int32)CGeneral::GetRandomNumberInRange(0.0f, 2.0f)
                     : numFree / 2;

    if (roll > chance) {
        return 1;
    }

    const auto dir = (wallId - 2) & 3;

    auto placed = 0;
    for (auto i = 0; i < count; i++) {
        switch (placement) {
        case 0: placed += Office_PlaceDesk(placed + tileX, tileY - 1, dir, 70, 0, m_style); break;
        case 1: placed += Office_PlaceDesk(tileX, placed + tileY, dir, 70, 0, m_style); break;
        case 2: placed += Office_PlaceDesk(placed + tileX, tileY, dir, 70, 0, m_style); break;
        case 3: placed += Office_PlaceDesk(tileX - 1, placed + tileY, dir, 70, 0, m_style); break;
        }
    }
    return placed + 1;
}

// 0x599770
void Interior_c::Office_FurnishEdges() {
    const auto lastX = m_box->m_width - 3;
    const auto lastY = m_box->m_depth - 3;

    // Walkway one tile in from the desks that will line the walls
    for (auto x = 2; x <= lastX; x++) {
        SetTilesStatus(x, lastY, 1, 1, TILE_STATE_3, false);
        SetTilesStatus(x, 2, 1, 1, TILE_STATE_3, false);
    }
    for (auto y = 2; y <= lastY; y++) {
        SetTilesStatus(2, y, 1, 1, TILE_STATE_3, false);
        SetTilesStatus(lastX, y, 1, 1, TILE_STATE_3, false);
    }

    AddGotoPt(2, 2, 0.5f, 0.5f);
    AddGotoPt(2, lastY, 0.5f, -0.5f);
    AddGotoPt(lastX, 2, -0.5f, 0.5f);
    AddGotoPt(lastX, lastY, -0.5f, -0.5f);

    const auto maxX = m_box->m_width - 1;
    const auto maxY = m_box->m_depth - 1;
    SetTilesStatus(m_box->m_door - 2, 0, 4, 2, TILE_STATE_7, false);

    // Desks first, then fillers in whatever gaps are left. Each call reports how
    // many tiles it consumed, which is how the walk advances.
    for (auto x = 1; x < maxX;) x += Office_PlaceEdgeDesks(-1, x, 0, WALL_X_B, WALL_X_B);
    for (auto x = 1; x < maxX;) x += Office_PlaceEdgeDesks(-1, x, maxY, WALL_X_A, WALL_X_A);
    for (auto y = 1; y <= maxY - 1;) y += Office_PlaceEdgeDesks(-1, 0, y, WALL_Y_A, WALL_Y_A);
    for (auto y = 1; y <= maxY - 1;) y += Office_PlaceEdgeDesks(-1, maxX, y, WALL_Y_B, WALL_Y_B);

    for (auto x = 1; x < maxX;) x += Office_PlaceEdgeFillers(-1, x, 0, WALL_X_B, WALL_X_B);
    for (auto x = 1; x < maxX;) x += Office_PlaceEdgeFillers(-1, x, maxY, WALL_X_A, WALL_X_A);
    for (auto y = 1; y <= maxY - 1;) y += Office_PlaceEdgeFillers(-1, 0, y, WALL_Y_A, WALL_Y_A);
    for (auto y = 1; y <= maxY - 1;) y += Office_PlaceEdgeFillers(-1, maxX, y, WALL_Y_B, WALL_Y_B);
}

// 0x599960
int32 Interior_c::Office_PlaceDeskQuad(int32 unused, int32 tileX, int32 tileY, int32 style) {
    // Four desks back to back, facing outwards
    Office_PlaceDesk(tileX, tileY - 2, 2, 70, 0, style);
    Office_PlaceDesk(tileX, tileY, 0, 70, 0, style);
    Office_PlaceDesk(tileX - 2, tileY, 0, 70, 0, style);
    Office_PlaceDesk(tileX - 2, tileY - 2, 2, 70, 0, style);

    // Ring of walkable tiles around the block
    SetTilesStatus(tileX - 3, tileY - 3, 6, 1, TILE_STATE_3, false);
    SetTilesStatus(tileX - 3, tileY + 2, 6, 1, TILE_STATE_3, false);
    SetTilesStatus(tileX - 3, tileY - 2, 1, 4, TILE_STATE_3, false);
    SetTilesStatus(tileX + 2, tileY - 2, 1, 4, TILE_STATE_3, false);

    return 6;
}

// 0x599A30
int32 Interior_c::Office_FurnishCenter() {
    // Fill the middle of the room with 6x6 blocks of four desks, centred on
    // whatever space is left over once the edges are accounted for
    const auto availW = m_box->m_width - 6;
    const auto availD = m_box->m_depth - 6;
    const auto marginY = availD % 6 / 2;

    if (availW <= 0 || availD <= 0) {
        return marginY;
    }

    for (auto col = 0; col < availW / 6; col++) {
        for (auto row = 0; row < availD / 6; row++) {
            Office_PlaceDeskQuad(-1, availW % 6 / 2 + 6 + col * 6, marginY + 6 + row * 6, m_style);
        }
    }

    return marginY;
}

// 0x599AF0
void Interior_c::FurnishOffice() {
    // Keep the four corners clear of the edge furniture
    SetTilesStatus(0, 0, 2, 2, TILE_STATE_2, false);
    SetTilesStatus(0, m_box->m_depth - 2, 2, 2, TILE_STATE_2, false);
    SetTilesStatus(m_box->m_width - 2, 0, 2, 2, TILE_STATE_2, false);
    SetTilesStatus(m_box->m_width - 2, m_box->m_depth - 2, 2, 2, TILE_STATE_2, false);

    m_style  = (int8)g_furnitureMan.GetRandomId(IG_OFFICE, 0, m_box->m_status);
    m_style2 = (int8)g_furnitureMan.GetRandomId(IG_OFFICE, 1, m_box->m_status);

    Office_FurnishEdges();
    Office_FurnishCenter();
    Shop_FurnishCeiling();
}

// 0x599BB0
int8 Interior_c::Shop_Place3PieceUnit(int32 type, int32 tileX, int32 tileY, int32 wallId, int32 length) {
    // A run of shop units: one end piece, `length - 2` middle pieces, another end
    // piece. On two of the walls the ends swap so the unit reads the right way round.
    const auto swapEnds = (wallId | 1) == 3;

    auto* const first = g_furnitureMan.GetFurniture(m_shopSubType, type + (swapEnds ? 1 : 0), -1, m_box->m_status);
    auto* const last  = g_furnitureMan.GetFurniture(m_shopSubType, type + (swapEnds ? 0 : 1), first->m_nId, m_box->m_status);
    auto* const mid   = g_furnitureMan.GetFurniture(m_shopSubType, type + 2, first->m_nId, m_box->m_status);

    int32 usedX, usedY;
    PlaceFurniture(first, tileX, tileY, 0.0f, 1, wallId, &usedX, &usedY, 0);

    const auto alongX = (wallId | 2) == 2;
    if (alongX) {
        tileX += usedX;
    } else {
        tileY += usedY;
    }

    for (auto i = length - 2; i > 0; i--) {
        PlaceFurniture(mid, tileX, tileY, 0.0f, 1, wallId, &usedX, &usedY, 0);
        if (alongX) {
            tileX += usedX;
        } else {
            tileY += usedY;
        }
    }

    PlaceFurniture(last, tileX, tileY, 0.0f, 1, wallId, &usedX, &usedY, 0);
    return 1;
}

// 0x599DC0
int32 Interior_c::Shop_PlaceEdgeUnits(int32 unitType, int32 tileX, int32 tileY, int32 wallId) {
    //! Read but never written -- nothing in the whole executable assigns it, so it
    //! stays -1 and the pick below always lands on unit 9. The ARM build has the
    //! constant folded and no global at all. Kept as-is rather than folded here.
    static auto& s_ShopUnitTypePick = StaticRef<int32>(0xBB3DE4);

    const auto numFree = GetNumEmptyTiles(tileX, tileY, wallId == WALL_X_A || wallId == WALL_X_B ? 1 : 2, 1);
    if (numFree <= 1) {
        return 1;
    }

    auto length = 2 + (int32)CGeneral::GetRandomNumberInRange(0.0f, 3.0f);
    if (numFree == 3) {
        length = 3;
    } else if (numFree < length) {
        length = numFree;
    } else if (numFree - length == 1) {
        length--; // Don't leave a single tile stranded
    }

    Shop_Place3PieceUnit(
        unitType != -1     ? unitType
        : s_ShopUnitTypePick > 50 ? 0
        : s_ShopUnitTypePick > 25 ? 3
        : s_ShopUnitTypePick > 10 ? 6
                                  : 9,
        tileX, tileY, wallId, length
    );
    return length;
}

// 0x599EF0
int32 Interior_c::Shop_PlaceCounter(bool leftOfDoor) {
    auto* const counter = g_furnitureMan.GetFurniture(IG_SHOP, 12, -1, m_box->m_status);
    auto* const till    = g_furnitureMan.GetFurniture(IG_SHOP, 13, -1, m_box->m_status);

    const auto tillRot = (int32)CGeneral::GetRandomNumberInRange(0.0f, 4.0f);

    int32 outW, outH;
    int32 counterX;
    if (leftOfDoor) {
        counterX = m_box->m_door - 5;
        PlaceFurniture(counter, counterX, 1, 0.0f, 1, 0, &outW, &outH, 0);
        SetTilesStatus(counterX - 1, 0, outW + 1, 1, TILE_STATE_2, false);
        PlaceFurniture(till, m_box->m_door + 1, 0, 0.0f, 1, tillRot, &outW, &outH, 1);
    } else {
        counterX = m_box->m_door + 2;
        PlaceFurniture(counter, counterX, 1, 0.0f, 1, 0, &outW, &outH, 0);
        SetTilesStatus(counterX, 0, outW + 1, 1, TILE_STATE_2, false);
        PlaceFurniture(till, m_box->m_door - 2, 0, 0.0f, 1, tillRot, &outW, &outH, 1);
    }

    return counterX + 2;
}

// 0x59A030
void Interior_c::Shop_PlaceFixedUnits() {
    if (m_box->m_door == -1) {
        return;
    }
    SetTilesStatus(m_box->m_door - 1, 0, 2, 1, TILE_STATE_7, false);

    const auto roomLeft  = m_box->m_door - 2;
    const auto roomRight = m_box->m_width - m_box->m_door - 2;

    // When neither side has room, no counter is placed and `counterX` is never
    // assigned -- yet both AddInteriorInfo calls below still use it. The x86
    // build reads the stack slot `push ecx` allocated on entry, which holds
    // `this`. Reachable: FurnishShop admits a shop on `door - 1 >= 6`, and this
    // path needs `door < 8 && width - door < 8` -- e.g. width 14, door at 7.
    int32 counterX = notsa::bugfixes::Interior_c_Shop_PlaceFixedUnits_UninitCounterX
        ? 0
        : (int32)(uintptr)this;

    if (roomRight < 6) {
        if (roomLeft >= 6) {
            counterX = Shop_PlaceCounter(true);
        }
    } else if (roomLeft < 6) {
        counterX = Shop_PlaceCounter(false);
    } else {
        counterX = Shop_PlaceCounter(CGeneral::GetRandomNumber() < 0x3FFF);
    }

    AddInteriorInfo(eInteriorInfoType::RUNSEQUENCE_ANIM, (float)counterX, 2.0f, 0, nullptr);
    AddInteriorInfo(eInteriorInfoType::STANDSTILL_LOOPED, (float)counterX, 0.0f, 2, nullptr);
}

// 0x59A130
void Interior_c::Shop_FurnishCeiling() {
    // NOP
}

// 0x59A140
void Interior_c::Shop_AddShelfInfo(int32 tileX, int32 tileY, int32 dir) {
    // Shelves are spaced out: at least two skipped, then a 40% chance each tile
    static int32& lastPlacement = StaticRef<int32>(0x8D0948);

    if (lastPlacement <= 1 || CGeneral::GetRandomNumberInRange(0.0f, 100.0f) <= 60.0f) {
        lastPlacement++;
        return;
    }

    AddInteriorInfo(eInteriorInfoType::RUN_ANIM, (float)tileX, (float)tileY, dir, nullptr);
    lastPlacement = 1;
}

// 0x59A1B0
void Interior_c::Shop_FurnishEdges() {
    plugin::CallMethod<0x59A1B0, Interior_c*>(this);
}

// 0x593DB0
bool Interior_c::GetBoundingBox(FurnitureEntity_c* entity, CVector* outCorners) {
    if (m_box->m_type >= 2 && m_box->m_type != 6) {
        return false;
    }

    // Flood out from the entity's tile to find the whole run of furniture it belongs to
    int32 visited[NUM_INTERIOR_TILES * NUM_INTERIOR_TILES]{};
    visited[NUM_INTERIOR_TILES * entity->m_tileX + entity->m_tileY] = 1;

    auto minX = (int32)entity->m_tileX, maxX = (int32)entity->m_tileX;
    auto minY = (int32)entity->m_tileY, maxY = (int32)entity->m_tileY;
    FindBoundingBox(entity->m_tileX, entity->m_tileY, &minX, &maxX, &minY, &maxY, visited);

    // Grow by a ped's radius so peds route around it rather than into it
    const auto r  = CPedGeometryAnalyser::ms_fPedNominalRadius;
    const auto x0 = (float)minX - 0.5f - r;
    const auto x1 = (float)maxX + 0.5f + r;
    const auto y0 = (float)minY - 0.5f - r;
    const auto y1 = (float)maxY + 0.5f + r;

    GetTileCentre(x0, y1, outCorners[0]);
    GetTileCentre(x0, y0, outCorners[1]);
    GetTileCentre(x1, y0, outCorners[2]);
    GetTileCentre(x1, y1, outCorners[3]);

    return true;
}

// 0x593910
void Interior_c::ResetTiles() {
    std::memset(m_tiles, TILE_EMPTY, sizeof(m_tiles));

    const auto lastX = m_box->m_width - 1;
    const auto lastY = m_box->m_depth - 1;

    // Doorways and windows are reserved so nothing gets furnished in front of
    // them. The original inlines SetTilesStatus at each of these; the bounds
    // guards it emits are identical to the ones the function already does.
    if (m_box->m_lDoorStart != -1) {
        SetTilesStatus(0, m_box->m_lDoorStart, 1, m_box->m_lDoorEnd - m_box->m_lDoorStart, TILE_STATE_8, false);
    }
    if (m_box->m_rDoorStart != -1) {
        SetTilesStatus(lastX, m_box->m_rDoorStart, 1, m_box->m_rDoorEnd - m_box->m_rDoorStart, TILE_STATE_8, false);
    }
    if (m_box->m_tDoorStart != -1) {
        SetTilesStatus(m_box->m_tDoorStart, lastY, m_box->m_tDoorEnd - m_box->m_tDoorStart, 1, TILE_STATE_8, false);
    }

    if (m_box->m_lWindowStart != -1) {
        SetTilesStatus(0, m_box->m_lWindowStart, 1, m_box->m_lWindowEnd - m_box->m_lWindowStart, TILE_STATE_9, false);
    }
    if (m_box->m_rWindowStart != -1) {
        SetTilesStatus(lastX, m_box->m_rWindowStart, 1, m_box->m_rWindowEnd - m_box->m_rWindowStart, TILE_STATE_9, false);
    }
    if (m_box->m_tWindowStart != -1) {
        SetTilesStatus(m_box->m_tWindowStart, lastY, m_box->m_tWindowEnd - m_box->m_tWindowStart, 1, TILE_STATE_9, false);
    }

    // Areas the level designer marked as off limits
    for (auto i = 0u; i < std::size(m_box->m_noGoLeft); i++) {
        if (m_box->m_noGoLeft[i] == -1 || m_box->m_noGoBottom[i] == -1) {
            continue;
        }
        SetTilesStatus(m_box->m_noGoLeft[i], m_box->m_noGoBottom[i], m_box->m_noGoWidth[i], m_box->m_noGoDepth[i], TILE_STATE_11, false);
    }
}

// 0x5934E0
CObject* Interior_c::PlaceObject(uint8 isStealable, Furniture_c* furniture, float offsetX, float offsetY, float offsetZ, float rotationZ) {
    const auto& bbMin = CModelInfo::GetModelInfo(furniture->m_nModelId)->GetColModel()->m_boundBox.m_vecMin;

    // Room-local position: offsets come from a corner but the matrix is about the
    // centre. The Z term drops the model so its underside rests on the floor.
    const CVector localPos{
        offsetX - (float)m_box->m_width * 0.5f,
        offsetY - (float)m_box->m_depth * 0.5f,
        offsetZ - (float)m_box->m_height * 0.5f - bbMin.z
    };

    if (g_furnitureMan.m_furnitureEntityPool.GetNumItems() <= 0) {
        return nullptr;
    }

    CMatrix local{};
    local.SetUnity();
    local.RotateZ(DegreesToRadians(rotationZ));
    local.GetPosition() += localPos;

    auto world = CMatrix{ &m_mat, false } * local;

    auto* const fe = g_furnitureMan.m_furnitureEntityPool.RemoveHead();
    if (!fe) {
        return nullptr;
    }

    auto* const obj = new CObject(furniture->m_nModelId, false);
    fe->m_entity = obj;
    obj->SetMatrix(world);
    obj->SetAreaCode((eAreaCodes)m_areaCode);
    obj->m_bDontCastShadowsOn = true;
    obj->m_nObjectType        = OBJECT_TYPE_DECORATION;
    obj->SetIsStatic(true);
    CWorld::Add(obj);

    fe->m_tileX = (uint16)offsetX;
    fe->m_tileY = (uint16)offsetY;
    m_furnitureEntityList.AddItem(fe);

    if (isStealable) {
        obj->objectFlags.bIsLiftable = true;

        if (g_interiorMan.HasInteriorHadStealDataSetup(this)) {
            const auto id = g_interiorMan.FindStealableObjectId(m_id, furniture->m_nModelId, localPos);
            if (id >= 0 && g_interiorMan.m_Objects[id].wasStolen) {
                // The player already took this one, so don't put it back
                CWorld::Remove(obj);
                delete obj;

                fe->m_entity = nullptr;
                m_furnitureEntityList.RemoveItem(fe);
                g_furnitureMan.m_furnitureEntityPool.AddItem(fe);
                return nullptr;
            }
            if (id >= 0) {
                g_interiorMan.m_Objects[id].entity = obj;
            }
        } else {
            auto& slot      = g_interiorMan.m_Objects[g_interiorMan.m_ObjectCount++];
            slot.entity     = obj;
            slot.modelId    = furniture->m_nModelId;
            slot.interiorId = m_id;
            slot.pos        = localPos;
            slot.wasStolen  = false;
        }
    }

    return obj;
}

// 0x5913B0
FurnitureEntity_c* Interior_c::GetFurnitureEntity(CEntity* entity) {
    for (auto& fe : m_furnitureEntityList) {
        if (fe.m_entity == entity) {
            return &fe;
        }
    }
    return nullptr;
}

// 0x5913E0
bool Interior_c::IsPtInside(const CVector& pt, CVector bias) {
    // Project onto the room's own axes, then compare against its half-extents
    const auto rel = pt - m_mat.pos;
    return std::fabs(DotProduct(m_mat.right, rel)) <= (float)m_box->m_width * 0.5f + bias.x
        && std::fabs(DotProduct(m_mat.up, rel))    <= (float)m_box->m_depth * 0.5f + bias.y
        && std::fabs(DotProduct(m_mat.at, rel))    <= (float)m_box->m_height * 0.5f + bias.z;
}

// 0x5914D0
void Interior_c::CalcMatrix(CVector* translation) {
    const auto flags = m_mat.flags;
    m_mat.right = CVector{ 1.0f, 0.0f, 0.0f };
    m_mat.up    = CVector{ 0.0f, 1.0f, 0.0f };
    m_mat.at    = CVector{ 0.0f, 0.0f, 1.0f };
    m_mat.pos   = CVector{ 0.0f, 0.0f, 0.0f };
    m_mat.flags = flags | 0x20003;

    auto axis = CVector{ 0.0f, 0.0f, 1.0f };
    RwMatrixRotate(&m_mat, &axis, m_box->m_rot, rwCOMBINEREPLACE);
    RwMatrixTranslate(&m_mat, translation, rwCOMBINEPOSTCONCAT);

    // Interiors hang off their group's entity, so inherit its transform
    RwMatrixMultiply(&m_mat, &m_mat, m_pInteriorGroup->GetEntity()->GetModellingMatrix());
}

// 0x591590
void Interior_c::Furnish() {
    switch (m_box->m_type) {
    case IG_SHOP:    FurnishShop(m_box->m_type); break; // Always 0 here
    case IG_OFFICE:  FurnishOffice();  break;
    case IG_LOUNGE:  FurnishLounge();  break;
    case IG_BEDROOM: FurnishBedroom(); break;
    case IG_KITCHEN: FurnishKitchen(); break;
    }
}

// 0x5915D0
void Interior_c::Unfurnish() {
    for (auto* fe = m_furnitureEntityList.GetHead(); fe;) {
        auto* const next = fe->m_pNext;
        auto* const obj  = fe->m_entity->AsObject();

        auto* const player = FindPlayerPed();
        if (player
         && player->GetEntityThatThisPedIsHolding() == obj
         && obj->GetIsTypeObject()
         && obj->objectFlags.bIsLiftable
        ) {
            // The player walked off with it, so let them keep it for a good long while
            CObject::nNoTempObjects++;
            obj->m_nObjectType  = OBJECT_TEMPORARY;
            obj->m_nRemovalTime = CTimer::GetTimeInMS() + 99'999'999;
        } else {
            CWorld::Remove(obj);
            delete obj;
        }

        fe->m_entity = nullptr;
        m_furnitureEntityList.RemoveItem(fe);
        g_furnitureMan.m_furnitureEntityPool.AddItem(fe);

        fe = next;
    }
}

// 0x591680
bool Interior_c::CheckTilesEmpty(int32 tileX, int32 tileY, int32 w, int32 d, bool allowState9) {
    if (tileX < 0 || tileY < 0 || tileX + w > m_box->m_width || tileY + d > m_box->m_depth) {
        return false; // Doesn't fit in the room at all
    }

    for (auto x = 0; x < w; x++) {
        for (auto y = 0; y < d; y++) {
            const auto status = m_tiles[tileX + x][tileY + y];
            if (status != TILE_EMPTY && !(allowState9 && status == TILE_STATE_9)) {
                return false;
            }
        }
    }
    return true;
}

// 0x591700
void Interior_c::SetTilesStatus(int32 tileX, int32 tileY, int32 w, int32 d, eTileStatus status, bool force) {
    if (tileX < 0 || tileY < 0 || tileX + w > m_box->m_width || tileY + d > m_box->m_depth) {
        return;
    }

    for (auto x = 0; x < w; x++) {
        for (auto y = 0; y < d; y++) {
            auto& tile = m_tiles[tileX + x][tileY + y];

            if (tile == TILE_STATE_9 && status == TILE_STATE_5) {
                tile = TILE_STATE_10;
            } else if (force) {
                if (tile != TILE_STATE_5 && tile != TILE_STATE_7 && tile != TILE_STATE_8) { // These are never overwritten
                    tile = (uint8)status;
                }
            } else if (tile == TILE_STATE_3) {
                if (status == TILE_STATE_3) { // Already claimed by another of the same kind, give up entirely
                    return;
                }
                if (status == TILE_STATE_4) {
                    tile = TILE_STATE_4;
                }
            } else if (tile == TILE_EMPTY) {
                tile = (uint8)status;
            }
        }
    }
}

// 0x5917C0
void Interior_c::SetCornerTiles(int32 corner, int32 size, eTileStatus status, bool force) {
    // Each corner marks a run along the wall in X and another in Y, forming an L
    const auto w = (int32)m_box->m_width;
    const auto d = (int32)m_box->m_depth;

    switch (corner) {
    case 0:
        SetTilesStatus(0, d - 1, size, 1, status, force);
        SetTilesStatus(0, d - size, 1, size, status, force);
        break;
    case 1:
        SetTilesStatus(0, 0, size, 1, status, force);
        SetTilesStatus(0, 0, 1, size, status, force);
        break;
    case 2:
        SetTilesStatus(w - size, 0, size, 1, status, force);
        SetTilesStatus(w - 1, 0, 1, size, status, force);
        break;
    case 3:
        SetTilesStatus(w - size, d - 1, size, 1, status, force);
        SetTilesStatus(w - 1, d - size, 1, size, status, force);
        break;
    }
}

// 0x5918E0
int32 Interior_c::GetTileStatus(int32 x, int32 y) {
    if (x < 0 || y < 0 || x >= m_box->m_width || y >= m_box->m_depth) {
        return TILE_STATE_1; // Outside the room, so nothing may be placed there
    }
    return m_tiles[x][y];
}

// 0x591920
int32 Interior_c::GetNumEmptyTiles(int32 tileX, int32 tileY, int32 wallId, int32 runLength) {
    // Walk along the wall from (tileX, tileY), counting how many consecutive
    // `runLength`-deep slots are free before hitting something or the edge.
    const auto step    = wallId == WALL_X_A || wallId == WALL_Y_B ? -1 : 1;
    const auto walkingX = wallId == WALL_Y_A || wallId == WALL_Y_B;

    auto numRuns = 0;
    for (auto outer = walkingX ? tileX : tileY; ; outer += step) {
        for (auto n = 0; n < runLength; n++) {
            const auto inner = (walkingX ? tileY : tileX) + n;
            const auto x = walkingX ? outer : inner;
            const auto y = walkingX ? inner : outer;
            if (x < 0 || y < 0 || x >= m_box->m_width || y >= m_box->m_depth || m_tiles[x][y] != TILE_EMPTY) {
                return numRuns;
            }
        }
        numRuns++;
    }
}

// 0x591B20
int32 Interior_c::GetRandomTile(eTileStatus status, int32& outTileX, int32& outTileY) {
    int32 x, y;
    do {
        x = (int32)CGeneral::GetRandomNumberInRange(0.0f, (float)m_box->m_width);
        y = (int32)CGeneral::GetRandomNumberInRange(0.0f, (float)m_box->m_depth);
    } while (GetTileStatus(x, y) != status);

    outTileX = x;
    outTileY = y;
    return y;
}

// 0x59A590
void Interior_c::Shop_FurnishAisles() {
    plugin::CallMethod<0x59A590, Interior_c*>(this);
}

// 0x591BD0
void Interior_c::GetTileCentre(float tileX, float tileY, CVector& outCentre) {
    outCentre = CVector{
        tileX - (float)m_box->m_width * 0.5f + 0.5f,
        tileY - (float)m_box->m_depth * 0.5f + 0.5f,
        -(float)m_box->m_height * 0.5f
    };
    RwV3dTransformPoints(&outCentre, &outCentre, 1, &m_mat);
}

// 0x591D20
void Interior_c::AddGotoPt(int32 tileX, int32 tileY, float offsetX, float offsetY) {
    if (m_numGotoPts >= (int8)std::size(m_gotoPts)) {
        return;
    }

    const auto status = GetTileStatus(tileX, tileY);
    if (status != TILE_STATE_3 && status != TILE_STATE_7) {
        return;
    }

    auto& pt = m_gotoPts[m_numGotoPts];
    GetTileCentre((float)tileX + offsetX, (float)tileY + offsetY, pt.pos);
    pt.tileX = (int8)tileX;
    pt.tileY = (int8)tileY;

    SetTilesStatus(tileX, tileY, 1, 1, TILE_STATE_4, false);
    m_numGotoPts++;
}

// 0x591E40
bool Interior_c::AddInteriorInfo(eInteriorInfoType actionType, float offsetX, float offsetY, int32 direction, CEntity* entityIgnoredCollision) {
    if (m_numInteriorInfos >= (int8)std::size(m_interiorInfos)) {
        return false;
    }

    CVector pos;
    GetTileCentre(offsetX, offsetY, pos);
    pos.z += 0.8f; // Sit the marker above the floor

    CVector dir{};
    if (direction != -1) {
        switch (direction) {
        case 0: dir.y = -1.0f; break;
        case 1: dir.x =  1.0f; break;
        case 2: dir.y =  1.0f; break;
        case 3: dir.x = -1.0f; break;
        }
        RwV3dTransformVectors(&dir, &dir, 1, &m_mat);
    }

    auto& info = m_interiorInfos[m_numInteriorInfos++];
    info.Type                   = actionType;
    info.Pos                    = pos;
    info.Dir                    = dir;
    info.IsInUse                = false;
    info.EntityIgnoredCollision = entityIgnoredCollision;
    return true;
}

// 0x591F90
void Interior_c::AddPickups() {
    // At most one pickup per interior, and only every three minutes
    if (CTimer::GetTimeInMS() - g_interiorMan.m_TimeLastPickupsGenerated < 180'000) {
        return;
    }

    for (auto attempt = 0; attempt < 100; attempt++) {
        const auto tileX = (int32)CGeneral::GetRandomNumberInRange(0.0f, (float)(m_box->m_width - 1));
        const auto tileY = (int32)CGeneral::GetRandomNumberInRange(0.0f, (float)(m_box->m_depth - 1));
        if (tileX < 0 || tileY < 0 || tileX >= m_box->m_width || tileY >= m_box->m_depth) {
            continue;
        }

        const auto tile = m_tiles[tileX][tileY];
        if (tile != TILE_EMPTY && tile != TILE_STATE_3 && tile != TILE_STATE_4) {
            continue; // Something is already standing there
        }

        CVector pos;
        GetTileCentre((float)tileX, (float)tileY, pos);

        if ((int32)CGeneral::GetRandomNumberInRange(0.0f, 100.0f) < 75) {
            CPickups::GenerateNewOne(pos, ModelIndices::MI_MONEY, PICKUP_MONEY, 10 + (int32)CGeneral::GetRandomNumberInRange(0.0f, 40.0f), 0, false, nullptr);
        } else {
            pos.z += 0.5f;

            const auto roll = (int32)CGeneral::GetRandomNumberInRange(0.0f, 100.0f);
            const auto weapon = roll < 40 ? WEAPON_BASEBALLBAT
                              : roll < 80 ? WEAPON_PISTOL
                              : roll < 90 ? WEAPON_KNIFE
                                          : WEAPON_SHOTGUN;

            CPickups::GenerateNewOne_WeaponType(pos, weapon, PICKUP_ONCE, 3 + (int32)CGeneral::GetRandomNumberInRange(0.0f, 15.0f), false, nullptr);
        }
        return;
    }
}

// 0x5922C0
void Interior_c::FindBoundingBox(int32 tileX, int32 tileY, int32* minX, int32* maxX, int32* minY, int32* maxY, int32* visited) {
    // Flood fill outwards over tiles in state 5, growing the bounds as we go.
    // `visited` is a 30x30 grid of flags owned by the caller.
    const auto CanSpreadTo = [this, visited](int32 x, int32 y) {
        return x >= 0 && y >= 0
            && x < m_box->m_width && y < m_box->m_depth
            && m_tiles[x][y] == TILE_STATE_5
            && !visited[NUM_INTERIOR_TILES * x + y];
    };

    for (;;) {
        if (tileX > 0 && CanSpreadTo(tileX - 1, tileY)) {
            visited[NUM_INTERIOR_TILES * (tileX - 1) + tileY] = 1;
            *minX = std::min(*minX, tileX - 1);
            FindBoundingBox(tileX - 1, tileY, minX, maxX, minY, maxY, visited);
        }

        if (tileY < (int32)NUM_INTERIOR_TILES - 1 && CanSpreadTo(tileX, tileY + 1)) {
            visited[NUM_INTERIOR_TILES * tileX + tileY + 1] = 1;
            *maxY = std::max(*maxY, tileY + 1);
            FindBoundingBox(tileX, tileY + 1, minX, maxX, minY, maxY, visited);
        }

        if (tileX < (int32)NUM_INTERIOR_TILES - 1 && CanSpreadTo(tileX + 1, tileY)) {
            visited[NUM_INTERIOR_TILES * (tileX + 1) + tileY] = 1;
            *maxX = std::max(*maxX, tileX + 1);
            FindBoundingBox(tileX + 1, tileY, minX, maxX, minY, maxY, visited);
        }

        // -Y is a plain loop rather than a fourth recursion, as in the original
        if (tileY <= 0 || !CanSpreadTo(tileX, tileY - 1)) {
            return;
        }
        visited[NUM_INTERIOR_TILES * tileX + tileY - 1] = 1;
        *minY = std::min(*minY, tileY - 1);
        tileY--;
    }
}

// 0x5924A0
void Interior_c::CalcExitPts() {
    plugin::CallMethod<0x5924A0, Interior_c*>(this);
}

// 0x5929F0
bool Interior_c::IsVisible() {
    const auto camPos = TheCamera.GetPosition();
    if (IsPtInside(camPos, CVector{ 5.0f, 5.0f, 0.0f })) {
        return true;
    }

    if (m_box->m_door <= 0) { // No door, so it can only be seen from within
        return false;
    }

    // Otherwise it's visible while the camera is near the doorway
    return (CVector2D{ camPos } - CVector2D{ m_doorPos }).SquaredMagnitude() < 100.0f;
}

// 0x592AA0
void Interior_c::PlaceFurniture(Furniture_c* a1, int32 a2, int32 a3, float a4, int32 a5, int32 a6, int32* a7, int32* a8, uint8 a9) {
    plugin::CallMethod<0x592AA0, Interior_c*, Furniture_c*, int32, int32, float, int32, int32, int32*, int32*, uint8>(this, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

// 0x593120
CObject* Interior_c::PlaceFurnitureOnWall(int32 furnitureGroupId, int32 furnitureSubgroupId, int32 furnitureId, float a5, int32 a6, int32 a7, int32 a8, int32 a9, int32* a10, int32* a11,
                                          int32* a12, int32* a13, int32* a14, int32* a15) {
    return plugin::CallMethodAndReturn<CObject*, 0x593120, Interior_c*, int32, int32, int32, float, int32, int32, int32, int32, int32*, int32*, int32*, int32*, int32*, int32*>(
        this, furnitureGroupId, furnitureSubgroupId, furnitureId, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}

// 0x593340
void Interior_c::PlaceFurnitureInCorner(int32 furnitureGroupId, int32 furnitureSubgroupId, int32 id, float a4, int32 a5, int32 a6, int32 a2, int32* a9, int32* a10, int32* a11,
                                        int32* a12, int32* a13) {
    plugin::CallMethod<0x593340, Interior_c*, int32, int32, int32, float, int32, int32, int32, int32*, int32*, int32*, int32*, int32*>(this, furnitureGroupId, furnitureSubgroupId,
                                                                                                                                       id, a4, a5, a6, a2, a9, a10, a11, a12, a13);
}

// 0x591C50
bool Interior_c::FindEmptyTiles(int32 w, int32 d, int32& outTileX, int32& outTileY) {
    for (auto attempt = 0; attempt < 100; attempt++) {
        const auto x = (int32)CGeneral::GetRandomNumberInRange(0.0f, (float)(m_box->m_width - w));
        const auto y = (int32)CGeneral::GetRandomNumberInRange(0.0f, (float)(m_box->m_depth - d));
        if (CheckTilesEmpty(x, y, w, d, true)) {
            outTileX = x;
            outTileY = y;
            return true;
        }
    }
    return false; // Room is too full
}

// 0x59A790
void Interior_c::FurnishShop(int32 shopSubType) {
    m_shopSubType = (int8)shopSubType;

    // A door too close to the middle leaves no usable wall, so don't furnish at all
    if (m_box->m_door - 1 < 6 && m_box->m_width - m_box->m_door < 6) {
        return;
    }

    SetTilesStatus(0, 0, 1, 1, TILE_STATE_2, false);
    SetTilesStatus(0, m_box->m_depth - 1, 1, 1, TILE_STATE_2, false);
    SetTilesStatus(m_box->m_width - 1, 0, 1, 1, TILE_STATE_2, false);
    SetTilesStatus(m_box->m_width - 1, m_box->m_depth - 1, 1, 1, TILE_STATE_2, false);

    Shop_PlaceFixedUnits();
    Shop_FurnishEdges();
    Shop_FurnishAisles();
}
