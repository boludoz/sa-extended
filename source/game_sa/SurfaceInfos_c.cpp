#include "StdInc.h"

#include "SurfaceInfos_c.h"
#include "Skidmark.h"
#include "Weather.h"

void SurfaceInfos_c::InjectHooks()
{
    RH_ScopedClass(SurfaceInfos_c);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(GetSurfaceIdFromName, 0x55D220);
    RH_ScopedInstall(Init, 0x55F420);

    RH_ScopedInstall(GetAdhesionGroup, 0x55E5C0);
    RH_ScopedInstall(GetTyreGrip, 0x55E5E0);
    RH_ScopedInstall(GetWetMultiplier, 0x55E600);
    RH_ScopedInstall(GetSkidmarkType, 0x55E630);
    RH_ScopedInstall(GetFrictionEffect, 0x55E650);
    RH_ScopedInstall(GetBulletFx, 0x55E670);
    RH_ScopedInstall(IsSoftLanding, 0x55E690);
    RH_ScopedInstall(IsSeeThrough, 0x55E6B0);
    RH_ScopedInstall(IsShootThrough, 0x55E6D0);
    RH_ScopedInstall(IsSand, 0x55E6F0);
    RH_ScopedInstall(IsWater, 0x55E710);
    RH_ScopedInstall(IsShallowWater, 0x55E730);
    RH_ScopedInstall(IsBeach, 0x55E750);
    RH_ScopedInstall(IsSteepSlope, 0x55E770);
    RH_ScopedInstall(IsGlass, 0x55E790);
    RH_ScopedInstall(IsStairs, 0x55E7B0);
    RH_ScopedInstall(IsSkateable, 0x55E7D0);
    RH_ScopedInstall(IsPavement, 0x55E7F0);
    RH_ScopedInstall(GetRoughness, 0x55E810);
    RH_ScopedInstall(GetFlammability, 0x55E830);
    RH_ScopedInstall(CreatesSparks, 0x55E850);
    RH_ScopedInstall(CantSprintOn, 0x55E870);
    RH_ScopedInstall(LeavesFootsteps, 0x55E890);
    RH_ScopedInstall(ProducesFootDust, 0x55E8B0);
    RH_ScopedInstall(MakesCarDirty, 0x55E8D0);
    RH_ScopedInstall(MakesCarClean, 0x55E8F0);
    RH_ScopedInstall(CreatesWheelGrass, 0x55E910);
    RH_ScopedInstall(CreatesWheelGravel, 0x55E930);
    RH_ScopedInstall(CreatesWheelMud, 0x55E950);
    RH_ScopedInstall(CreatesWheelDust, 0x55E970);
    RH_ScopedInstall(CreatesWheelSand, 0x55E990);
    RH_ScopedInstall(CreatesWheelSpray, 0x55E9B0);
    RH_ScopedInstall(CreatesPlants, 0x55E9D0);
    RH_ScopedInstall(CreatesObjects, 0x55E9F0);
    RH_ScopedInstall(CanClimb, 0x55EA10);
    RH_ScopedInstall(IsAudioConcrete, 0x55EA30);
    RH_ScopedInstall(IsAudioGrass, 0x55EA50);
    RH_ScopedInstall(IsAudioSand, 0x55EA70);
    RH_ScopedInstall(IsAudioGravel, 0x55EA90);
    RH_ScopedInstall(IsAudioWood, 0x55EAB0);
    RH_ScopedInstall(IsAudioWater, 0x55EAD0);
    RH_ScopedInstall(IsAudioMetal, 0x55EAF0);
    RH_ScopedInstall(IsAudioLongGrass, 0x55EB10);
    RH_ScopedInstall(IsAudioTile, 0x55EB30);
    RH_ScopedInstall(GetAdhesiveLimit, 0x55EB50);
}

// var: sa 0xB79538
SurfaceInfos_c g_surfaceInfos;

// stub: sa 0x55D0C0
SurfaceInfos_c::SurfaceInfos_c()
{
    ;
}

// stub: sa 0x55D0D0
SurfaceInfos_c::~SurfaceInfos_c()
{
    ;
}

// stub: sa 0x55F420
void SurfaceInfos_c::Init()
{
    // They call CFileMgr::SetDir("") only once in LoadAdhesiveLimits
    LoadAdhesiveLimits();
    LoadSurfaceInfos();
    LoadSurfaceAudioInfos();
}

// 0x55D0E0
void SurfaceInfos_c::LoadAdhesiveLimits()
{
    CFileMgr::SetDir("");

    FILESTREAM fid = CFileMgr::OpenFile("data\\surface.dat", "rb");

    int32 i = 0;
    char* pLine = CFileLoader::LoadLine(fid);

    if (pLine != nullptr)
    {
        do
        {
            if (*pLine != ';' && *pLine != '\0')
            {
                char surfacename[256];
                sscanf(pLine, "%s", surfacename);

                while (*pLine != ' ' && *pLine != '\t')
                {
                    pLine++;
                }

                for (int32 j = 0; j <= i; j++)
                {
                    while (*pLine == ' ' || *pLine == '\t')
                    {
                        pLine++;
                    }

                    float fAdhesiveLimit = 0.0f;
                    if (*pLine != '-')
                    {
                        sscanf(pLine, "%f", &fAdhesiveLimit);
                    }

                    while (*pLine != ' ' && *pLine != '\t' && *pLine != '\0')
                    {
                        pLine++;
                    }

                    m_adhesiveLimitTable[i][j] = fAdhesiveLimit;
                    m_adhesiveLimitTable[j][i] = fAdhesiveLimit;
                }

                i++;
            }
            pLine = CFileLoader::LoadLine(fid);
        } while (pLine != nullptr);
    }
    CFileMgr::CloseFile(fid);
}

// 0x55D220
uint32 SurfaceInfos_c::GetSurfaceIdFromName(char* surfaceName)
{
    if (!strcmp(surfaceName, "DEFAULT"))
    {
        return 0;
    }
    if (!strcmp(surfaceName, "TARMAC"))
    {
        return 1;
    }
    if (!strcmp(surfaceName, "TARMAC_FUCKED"))
    {
        return 2;
    }
    if (!strcmp(surfaceName, "TARMAC_REALLYFUCKED"))
    {
        return 3;
    }
    if (!strcmp(surfaceName, "PAVEMENT"))
    {
        return 4;
    }
    if (!strcmp(surfaceName, "PAVEMENT_FUCKED"))
    {
        return 5;
    }
    if (!strcmp(surfaceName, "GRAVEL"))
    {
        return 6;
    }
    if (!strcmp(surfaceName, "FUCKED_CONCRETE"))
    {
        return 7;
    }
    if (!strcmp(surfaceName, "PAINTED_GROUND"))
    {
        return 8;
    }
    if (!strcmp(surfaceName, "GRASS_SHORT_LUSH"))
    {
        return 9;
    }
    if (!strcmp(surfaceName, "GRASS_MEDIUM_LUSH"))
    {
        return 10;
    }
    if (!strcmp(surfaceName, "GRASS_LONG_LUSH"))
    {
        return 11;
    }
    if (!strcmp(surfaceName, "GRASS_SHORT_DRY"))
    {
        return 12;
    }
    if (!strcmp(surfaceName, "GRASS_MEDIUM_DRY"))
    {
        return 13;
    }
    if (!strcmp(surfaceName, "GRASS_LONG_DRY"))
    {
        return 14;
    }
    if (!strcmp(surfaceName, "GOLFGRASS_ROUGH"))
    {
        return 15;
    }
    if (!strcmp(surfaceName, "GOLFGRASS_SMOOTH"))
    {
        return 16;
    }
    if (!strcmp(surfaceName, "STEEP_SLIDYGRASS"))
    {
        return 17;
    }
    if (!strcmp(surfaceName, "STEEP_CLIFF"))
    {
        return 18;
    }
    if (!strcmp(surfaceName, "FLOWERBED"))
    {
        return 19;
    }
    if (!strcmp(surfaceName, "MEADOW"))
    {
        return SURFACE_TYPE_MEADOW;
    }
    if (!strcmp(surfaceName, "WASTEGROUND"))
    {
        return 21;
    }
    if (!strcmp(surfaceName, "WOODLANDGROUND"))
    {
        return 22;
    }
    if (!strcmp(surfaceName, "VEGETATION"))
    {
        return 23;
    }
    if (!strcmp(surfaceName, "MUD_WET"))
    {
        return 24;
    }
    if (!strcmp(surfaceName, "MUD_DRY"))
    {
        return 25;
    }
    if (!strcmp(surfaceName, "DIRT"))
    {
        return 26;
    }
    if (!strcmp(surfaceName, "DIRTTRACK"))
    {
        return 27;
    }
    if (!strcmp(surfaceName, "SAND_DEEP"))
    {
        return 28;
    }
    if (!strcmp(surfaceName, "SAND_MEDIUM"))
    {
        return 29;
    }
    if (!strcmp(surfaceName, "SAND_COMPACT"))
    {
        return 30;
    }
    if (!strcmp(surfaceName, "SAND_ARID"))
    {
        return 31;
    }
    if (!strcmp(surfaceName, "SAND_MORE"))
    {
        return 32;
    }
    if (!strcmp(surfaceName, "SAND_BEACH"))
    {
        return 33;
    }
    if (!strcmp(surfaceName, "CONCRETE_BEACH"))
    {
        return 34;
    }
    if (!strcmp(surfaceName, "ROCK_DRY"))
    {
        return 35;
    }
    if (!strcmp(surfaceName, "ROCK_WET"))
    {
        return 36;
    }
    if (!strcmp(surfaceName, "ROCK_CLIFF"))
    {
        return 37;
    }
    if (!strcmp(surfaceName, "WATER_RIVERBED"))
    {
        return 38;
    }
    if (!strcmp(surfaceName, "WATER_SHALLOW"))
    {
        return 39;
    }
    if (!strcmp(surfaceName, "CORNFIELD"))
    {
        return 40;
    }
    if (!strcmp(surfaceName, "HEDGE"))
    {
        return 41;
    }
    if (!strcmp(surfaceName, "WOOD_CRATES"))
    {
        return 42;
    }
    if (!strcmp(surfaceName, "WOOD_SOLID"))
    {
        return 43;
    }
    if (!strcmp(surfaceName, "WOOD_THIN"))
    {
        return 44;
    }
    if (!strcmp(surfaceName, "GLASS"))
    {
        return 45;
    }
    if (!strcmp(surfaceName, "GLASS_WINDOWS_LARGE"))
    {
        return 46;
    }
    if (!strcmp(surfaceName, "GLASS_WINDOWS_SMALL"))
    {
        return 47;
    }
    if (!strcmp(surfaceName, "EMPTY1"))
    {
        return 48;
    }
    if (!strcmp(surfaceName, "EMPTY2"))
    {
        return 49;
    }
    if (!strcmp(surfaceName, "GARAGE_DOOR"))
    {
        return 50;
    }
    if (!strcmp(surfaceName, "THICK_METAL_PLATE"))
    {
        return 51;
    }
    if (!strcmp(surfaceName, "SCAFFOLD_POLE"))
    {
        return 52;
    }
    if (!strcmp(surfaceName, "LAMP_POST"))
    {
        return 53;
    }
    if (!strcmp(surfaceName, "METAL_GATE"))
    {
        return 54;
    }
    if (!strcmp(surfaceName, "METAL_CHAIN_FENCE"))
    {
        return 55;
    }
    if (!strcmp(surfaceName, "GIRDER"))
    {
        return 56;
    }
    if (!strcmp(surfaceName, "FIRE_HYDRANT"))
    {
        return 57;
    }
    if (!strcmp(surfaceName, "CONTAINER"))
    {
        return 58;
    }
    if (!strcmp(surfaceName, "NEWS_VENDOR"))
    {
        return 59;
    }
    if (!strcmp(surfaceName, "WHEELBASE"))
    {
        return 60;
    }
    if (!strcmp(surfaceName, "CARDBOARDBOX"))
    {
        return 61;
    }
    if (!strcmp(surfaceName, "PED"))
    {
        return 62;
    }
    if (!strcmp(surfaceName, "CAR"))
    {
        return 63;
    }
    if (!strcmp(surfaceName, "CAR_PANEL"))
    {
        return 64;
    }
    if (!strcmp(surfaceName, "CAR_MOVINGCOMPONENT"))
    {
        return 65;
    }
    if (!strcmp(surfaceName, "TRANSPARENT_CLOTH"))
    {
        return 66;
    }
    if (!strcmp(surfaceName, "RUBBER"))
    {
        return 67;
    }
    if (!strcmp(surfaceName, "PLASTIC"))
    {
        return 68;
    }
    if (!strcmp(surfaceName, "TRANSPARENT_STONE"))
    {
        return 69;
    }
    if (!strcmp(surfaceName, "WOOD_BENCH"))
    {
        return 70;
    }
    if (!strcmp(surfaceName, "CARPET"))
    {
        return 71;
    }
    if (!strcmp(surfaceName, "FLOORBOARD"))
    {
        return 72;
    }
    if (!strcmp(surfaceName, "STAIRSWOOD"))
    {
        return 73;
    }
    if (!strcmp(surfaceName, "P_SAND"))
    {
        return 74;
    }
    if (!strcmp(surfaceName, "P_SAND_DENSE"))
    {
        return 75;
    }
    if (!strcmp(surfaceName, "P_SAND_ARID"))
    {
        return 76;
    }
    if (!strcmp(surfaceName, "P_SAND_COMPACT"))
    {
        return 77;
    }
    if (!strcmp(surfaceName, "P_SAND_ROCKY"))
    {
        return 78;
    }
    if (!strcmp(surfaceName, "P_SANDBEACH"))
    {
        return 79;
    }
    if (!strcmp(surfaceName, "P_GRASS_SHORT"))
    {
        return 80;
    }
    if (!strcmp(surfaceName, "P_GRASS_MEADOW"))
    {
        return 81;
    }
    if (!strcmp(surfaceName, "P_GRASS_DRY"))
    {
        return 82;
    }
    if (!strcmp(surfaceName, "P_WOODLAND"))
    {
        return 83;
    }
    if (!strcmp(surfaceName, "P_WOODDENSE"))
    {
        return 84;
    }
    if (!strcmp(surfaceName, "P_ROADSIDE"))
    {
        return 85;
    }
    if (!strcmp(surfaceName, "P_ROADSIDEDES"))
    {
        return 86;
    }
    if (!strcmp(surfaceName, "P_FLOWERBED"))
    {
        return 87;
    }
    if (!strcmp(surfaceName, "P_WASTEGROUND"))
    {
        return 88;
    }
    if (!strcmp(surfaceName, "P_CONCRETE"))
    {
        return 89;
    }
    if (!strcmp(surfaceName, "P_OFFICEDESK"))
    {
        return 90;
    }
    if (!strcmp(surfaceName, "P_711SHELF1"))
    {
        return 91;
    }
    if (!strcmp(surfaceName, "P_711SHELF2"))
    {
        return 92;
    }
    if (!strcmp(surfaceName, "P_711SHELF3"))
    {
        return 93;
    }
    if (!strcmp(surfaceName, "P_RESTUARANTTABLE"))
    {
        return 94;
    }
    if (!strcmp(surfaceName, "P_BARTABLE"))
    {
        return 95;
    }
    if (!strcmp(surfaceName, "P_UNDERWATERLUSH"))
    {
        return 96;
    }
    if (!strcmp(surfaceName, "P_UNDERWATERBARREN"))
    {
        return 97;
    }
    if (!strcmp(surfaceName, "P_UNDERWATERCORAL"))
    {
        return 98;
    }
    if (!strcmp(surfaceName, "P_UNDERWATERDEEP"))
    {
        return 99;
    }
    if (!strcmp(surfaceName, "P_RIVERBED"))
    {
        return 100;
    }
    if (!strcmp(surfaceName, "P_RUBBLE"))
    {
        return 101;
    }
    if (!strcmp(surfaceName, "P_BEDROOMFLOOR"))
    {
        return 102;
    }
    if (!strcmp(surfaceName, "P_KIRCHENFLOOR"))
    {
        return 103;
    }
    if (!strcmp(surfaceName, "P_LIVINGRMFLOOR"))
    {
        return 104;
    }
    if (!strcmp(surfaceName, "P_CORRIDORFLOOR"))
    {
        return 105;
    }
    if (!strcmp(surfaceName, "P_711FLOOR"))
    {
        return 106;
    }
    if (!strcmp(surfaceName, "P_FASTFOODFLOOR"))
    {
        return 107;
    }
    if (!strcmp(surfaceName, "P_SKANKYFLOOR"))
    {
        return 108;
    }
    if (!strcmp(surfaceName, "P_MOUNTAIN"))
    {
        return 109;
    }
    if (!strcmp(surfaceName, "P_MARSH"))
    {
        return 110;
    }
    if (!strcmp(surfaceName, "P_BUSHY"))
    {
        return 111;
    }
    if (!strcmp(surfaceName, "P_BUSHYMIX"))
    {
        return 112;
    }
    if (!strcmp(surfaceName, "P_BUSHYDRY"))
    {
        return 113;
    }
    if (!strcmp(surfaceName, "P_BUSHYMID"))
    {
        return 114;
    }
    if (!strcmp(surfaceName, "P_GRASSWEEFLOWERS"))
    {
        return 115;
    }
    if (!strcmp(surfaceName, "P_GRASSDRYTALL"))
    {
        return 116;
    }
    if (!strcmp(surfaceName, "P_GRASSLUSHTALL"))
    {
        return 117;
    }
    if (!strcmp(surfaceName, "P_GRASSGRNMIX"))
    {
        return 118;
    }
    if (!strcmp(surfaceName, "P_GRASSBRNMIX"))
    {
        return 119;
    }
    if (!strcmp(surfaceName, "P_GRASSLOW"))
    {
        return 120;
    }
    if (!strcmp(surfaceName, "P_GRASSROCKY"))
    {
        return 121;
    }
    if (!strcmp(surfaceName, "P_GRASSSMALLTREES"))
    {
        return 122;
    }
    if (!strcmp(surfaceName, "P_DIRTROCKY"))
    {
        return 123;
    }
    if (!strcmp(surfaceName, "P_DIRTWEEDS"))
    {
        return 124;
    }
    if (!strcmp(surfaceName, "P_GRASSWEEDS"))
    {
        return 125;
    }
    if (!strcmp(surfaceName, "P_RIVEREDGE"))
    {
        return 126;
    }
    if (!strcmp(surfaceName, "P_POOLSIDE"))
    {
        return 127;
    }
    if (!strcmp(surfaceName, "P_FORESTSTUMPS"))
    {
        return 128;
    }
    if (!strcmp(surfaceName, "P_FORESTSTICKS"))
    {
        return 129;
    }
    if (!strcmp(surfaceName, "P_FORRESTLEAVES"))
    {
        return 130;
    }
    if (!strcmp(surfaceName, "P_DESERTROCKS"))
    {
        return 131;
    }
    if (!strcmp(surfaceName, "P_FORRESTDRY"))
    {
        return 132;
    }
    if (!strcmp(surfaceName, "P_SPARSEFLOWERS"))
    {
        return 133;
    }
    if (!strcmp(surfaceName, "P_BUILDINGSITE"))
    {
        return 134;
    }
    if (!strcmp(surfaceName, "P_DOCKLANDS"))
    {
        return 135;
    }
    if (!strcmp(surfaceName, "P_INDUSTRIAL"))
    {
        return 136;
    }
    if (!strcmp(surfaceName, "P_INDUSTJETTY"))
    {
        return 137;
    }
    if (!strcmp(surfaceName, "P_CONCRETELITTER"))
    {
        return 138;
    }
    if (!strcmp(surfaceName, "P_ALLEYRUBISH"))
    {
        return 139;
    }
    if (!strcmp(surfaceName, "P_JUNKYARDPILES"))
    {
        return 140;
    }
    if (!strcmp(surfaceName, "P_JUNKYARDGRND"))
    {
        return 141;
    }
    if (!strcmp(surfaceName, "P_DUMP"))
    {
        return 142;
    }
    if (!strcmp(surfaceName, "P_CACTUSDENSE"))
    {
        return 143;
    }
    if (!strcmp(surfaceName, "P_AIRPORTGRND"))
    {
        return 144;
    }
    if (!strcmp(surfaceName, "P_CORNFIELD"))
    {
        return 145;
    }
    if (!strcmp(surfaceName, "P_GRASSLIGHT"))
    {
        return 146;
    }
    if (!strcmp(surfaceName, "P_GRASSLIGHTER"))
    {
        return 147;
    }
    if (!strcmp(surfaceName, "P_GRASSLIGHTER2"))
    {
        return 148;
    }
    if (!strcmp(surfaceName, "P_GRASSMID1"))
    {
        return 149;
    }
    if (!strcmp(surfaceName, "P_GRASSMID2"))
    {
        return 150;
    }
    if (!strcmp(surfaceName, "P_GRASSDARK"))
    {
        return 151;
    }
    if (!strcmp(surfaceName, "P_GRASSDARK2"))
    {
        return 152;
    }
    if (!strcmp(surfaceName, "P_GRASSDIRTMIX"))
    {
        return 153;
    }
    if (!strcmp(surfaceName, "P_RIVERBEDSTONE"))
    {
        return SURFACE_TYPE_P_RIVERBEDSTONE;
    }
    if (!strcmp(surfaceName, "P_RIVERBEDSHALLOW"))
    {
        return SURFACE_TYPE_P_RIVERBEDSHALLOW;
    }
    if (!strcmp(surfaceName, "P_RIVERBEDWEEDS"))
    {
        return SURFACE_TYPE_P_RIVERBEDWEEDS;
    }
    if (!strcmp(surfaceName, "P_SEAWEED"))
    {
        return SURFACE_TYPE_P_SEAWEED;
    }
    if (!strcmp(surfaceName, "DOOR"))
    {
        return SURFACE_TYPE_DOOR;
    }
    if (!strcmp(surfaceName, "PLASTICBARRIER"))
    {
        return SURFACE_TYPE_PLASTICBARRIER;
    }
    if (!strcmp(surfaceName, "PARKGRASS"))
    {
        return SURFACE_TYPE_PARKGRASS;
    }
    if (!strcmp(surfaceName, "STAIRSSTONE"))
    {
        return SURFACE_TYPE_STAIRSSTONE;
    }
    if (!strcmp(surfaceName, "STAIRSMETAL"))
    {
        return SURFACE_TYPE_STAIRSMETAL;
    }
    if (!strcmp(surfaceName, "STAIRSCARPET"))
    {
        return SURFACE_TYPE_STAIRSCARPET;
    }
    if (!strcmp(surfaceName, "FLOORMETAL"))
    {
        return SURFACE_TYPE_FLOORMETAL;
    }
    if (!strcmp(surfaceName, "FLOORCONCRETE"))
    {
        return SURFACE_TYPE_FLOORCONCRETE;
    }
    if (!strcmp(surfaceName, "BIN_BAG"))
    {
        return SURFACE_TYPE_BIN_BAG;
    }
    if (!strcmp(surfaceName, "THIN_METAL_SHEET"))
    {
        return SURFACE_TYPE_THIN_METAL_SHEET;
    }
    if (!strcmp(surfaceName, "METAL_BARREL"))
    {
        return SURFACE_TYPE_METAL_BARREL;
    }
    if (!strcmp(surfaceName, "PLASTIC_CONE"))
    {
        return SURFACE_TYPE_PLASTIC_CONE;
    }
    if (!strcmp(surfaceName, "PLASTIC_DUMPSTER"))
    {
        return SURFACE_TYPE_PLASTIC_DUMPSTER;
    }
    if (!strcmp(surfaceName, "METAL_DUMPSTER"))
    {
        return SURFACE_TYPE_METAL_DUMPSTER;
    }
    if (!strcmp(surfaceName, "WOOD_PICKET_FENCE"))
    {
        return SURFACE_TYPE_WOOD_PICKET_FENCE;
    }
    if (!strcmp(surfaceName, "WOOD_SLATTED_FENCE"))
    {
        return SURFACE_TYPE_WOOD_SLATTED_FENCE;
    }
    if (!strcmp(surfaceName, "WOOD_RANCH_FENCE"))
    {
        return SURFACE_TYPE_WOOD_RANCH_FENCE;
    }
    if (!strcmp(surfaceName, "UNBREAKABLE_GLASS"))
    {
        return SURFACE_TYPE_UNBREAKABLE_GLASS;
    }
    if (!strcmp(surfaceName, "HAY_BALE"))
    {
        return SURFACE_TYPE_HAY_BALE;
    }
    if (!strcmp(surfaceName, "GORE"))
    {
        return SURFACE_TYPE_GORE;
    }
    return strcmp(surfaceName, "RAILTRACK") != SURFACE_TYPE_DEFAULT ? SURFACE_TYPE_DEFAULT : SURFACE_TYPE_RAILTRACK;
}

// 0x55EB90
void SurfaceInfos_c::LoadSurfaceInfos()
{
    FILESTREAM fileId = CFileMgr::OpenFile("data\\surfinfo.dat", "r");
    char* pLine = CFileLoader::LoadLine(fileId);

    if (pLine)
    {
        char surfaceName[64];
        char adhesionGroup[32];
        float typeGrip;
        float wetMult;
        char skidmarkType[32];
        char frictionEffect[32];
        int32 isSoftLanding;
        int32 isSeeThrough;
        int32 isShootThrough;
        int32 isSand;
        int32 isWater;
        int32 isShallowWater;
        int32 isBeach;
        int32 isSteepSlope;
        int32 isGlass;
        int32 isStairs;
        int32 isSkateable;
        int32 isPavement;
        int32 roughness;
        int32 flammability;
        int32 createsSparks;
        int32 cantSprintOn;
        int32 leavesFootsteps;
        int32 producesFootDust;
        int32 makesCarDirty;
        int32 makesCarClean;
        int32 createsWheelGrass;
        int32 createsWheelGravel;
        int32 createsWheelMud;
        int32 createsWheelDust;
        int32 createsWheelSand;
        int32 createsWheelSpray;
        int32 createsPlants;
        int32 createsObjects;
        int32 canClimb;
        char bulletFx[32];

        do
        {
            if (*pLine != '#' && *pLine != '\0')
            {
                sscanf(pLine,
                    "%s %s %f %f %s %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
                    "%s",
                    surfaceName, adhesionGroup, &typeGrip, &wetMult, skidmarkType, frictionEffect, &isSoftLanding, &isSeeThrough, &isShootThrough, &isSand, &isWater, &isShallowWater, &isBeach, &isSteepSlope, &isGlass, &isStairs, &isSkateable, &isPavement, &roughness, &flammability, &createsSparks, &cantSprintOn, &leavesFootsteps, &producesFootDust, &makesCarDirty, &makesCarClean, &createsWheelGrass, &createsWheelGravel, &createsWheelMud, &createsWheelDust, &createsWheelSand, &createsWheelSpray, &createsPlants, &createsObjects, &canClimb, bulletFx);

                uint32 surfaceId = GetSurfaceIdFromName(surfaceName);

                if (!strcmp(adhesionGroup, "RUBBER"))
                {
                    m_surfaceInfos[surfaceId].adhesionGroup = 0;
                }
                else if (!strcmp(adhesionGroup, "HARD"))
                {
                    m_surfaceInfos[surfaceId].adhesionGroup = 1;
                }
                else if (!strcmp(adhesionGroup, "ROAD"))
                {
                    m_surfaceInfos[surfaceId].adhesionGroup = 2;
                }
                else if (!strcmp(adhesionGroup, "LOOSE"))
                {
                    m_surfaceInfos[surfaceId].adhesionGroup = 3;
                }
                else if (!strcmp(adhesionGroup, "SAND"))
                {
                    m_surfaceInfos[surfaceId].adhesionGroup = 4;
                }
                else if (!strcmp(adhesionGroup, "WET"))
                {
                    m_surfaceInfos[surfaceId].adhesionGroup = 5;
                }

                m_surfaceInfos[surfaceId].tyreGrip = (int8)(typeGrip * 10.0f);
                m_surfaceInfos[surfaceId].wetMult = (int8)(wetMult * 100.0f);

                if (!strcmp(skidmarkType, "DEFAULT"))
                {
                    m_surfaceInfos[surfaceId].skidmarkType = 0;
                }
                else if (!strcmp(skidmarkType, "SANDY"))
                {
                    m_surfaceInfos[surfaceId].skidmarkType = 2;
                }
                else if (!strcmp(skidmarkType, "MUDDY"))
                {
                    m_surfaceInfos[surfaceId].skidmarkType = 1;
                }

                if (!strcmp(frictionEffect, "NONE"))
                {
                    m_surfaceInfos[surfaceId].frictionEffect = 0;
                }
                else if (!strcmp(frictionEffect, "SPARKS"))
                {
                    m_surfaceInfos[surfaceId].frictionEffect = 1;
                }

                if (!strcmp(bulletFx, "NONE"))
                {
                    m_surfaceInfos[surfaceId].bulletFx = 0;
                }
                else if (!strcmp(bulletFx, "SPARKS"))
                {
                    m_surfaceInfos[surfaceId].bulletFx = 1;
                }
                else if (!strcmp(bulletFx, "SAND"))
                {
                    m_surfaceInfos[surfaceId].bulletFx = 2;
                }
                else if (!strcmp(bulletFx, "WOOD"))
                {
                    m_surfaceInfos[surfaceId].bulletFx = 3;
                }
                else if (!strcmp(bulletFx, "DUST"))
                {
                    m_surfaceInfos[surfaceId].bulletFx = 4;
                }

                m_surfaceInfos[surfaceId].isSoftLanding = isSoftLanding;
                m_surfaceInfos[surfaceId].isSeeThrough = isSeeThrough;
                m_surfaceInfos[surfaceId].isShootThrough = isShootThrough;
                m_surfaceInfos[surfaceId].isSand = isSand;
                m_surfaceInfos[surfaceId].isWater = isWater;
                m_surfaceInfos[surfaceId].isShallowWater = isShallowWater;
                m_surfaceInfos[surfaceId].isBeach = isBeach;
                m_surfaceInfos[surfaceId].isSteepSlope = isSteepSlope;
                m_surfaceInfos[surfaceId].isGlass = isGlass;
                m_surfaceInfos[surfaceId].isStairs = isStairs;
                m_surfaceInfos[surfaceId].isSkateable = isSkateable;
                m_surfaceInfos[surfaceId].isPavement = isPavement;
                m_surfaceInfos[surfaceId].roughness = roughness;
                m_surfaceInfos[surfaceId].flammability = flammability;
                m_surfaceInfos[surfaceId].createsSparks = createsSparks;
                m_surfaceInfos[surfaceId].cantSprintOn = cantSprintOn;
                m_surfaceInfos[surfaceId].leavesFootsteps = leavesFootsteps;
                m_surfaceInfos[surfaceId].producesFootDust = producesFootDust;
                m_surfaceInfos[surfaceId].makesCarDirty = makesCarDirty;
                m_surfaceInfos[surfaceId].makesCarClean = makesCarClean;
                m_surfaceInfos[surfaceId].createsWheelGrass = createsWheelGrass;
                m_surfaceInfos[surfaceId].createsWheelGravel = createsWheelGravel;
                m_surfaceInfos[surfaceId].createsWheelMud = createsWheelMud;
                m_surfaceInfos[surfaceId].createsWheelDust = createsWheelDust;
                m_surfaceInfos[surfaceId].createsWheelSand = createsWheelSand;
                m_surfaceInfos[surfaceId].createsWheelSpray = createsWheelSpray;
                m_surfaceInfos[surfaceId].createsPlants = createsPlants;
                m_surfaceInfos[surfaceId].createsObjects = createsObjects;
                m_surfaceInfos[surfaceId].canClimb = canClimb;
            }

            pLine = CFileLoader::LoadLine(fileId);
        } while (pLine);
    }

    CFileMgr::CloseFile(fileId);
}

// 0x55F2B0
void SurfaceInfos_c::LoadSurfaceAudioInfos()
{
    FILESTREAM fileId = CFileMgr::OpenFile("data\\surfaud.dat", "r");
    char* pLine = CFileLoader::LoadLine(fileId);

    if (pLine)
    {
        char surfaceName[64];
        int32 isConcrete;
        int32 isGrass;
        int32 isSand;
        int32 isGravel;
        int32 isWood;
        int32 isWater;
        int32 isMetal;
        int32 isLongGrass;
        int32 isTile;

        do
        {
            if (*pLine != '#' && *pLine != '\0')
            {
                sscanf(pLine, "%s %d %d %d %d %d %d %d %d %d", surfaceName, &isConcrete, &isGrass, &isSand, &isGravel, &isWood, &isWater, &isMetal, &isLongGrass, &isTile);

                uint32 surfaceId = GetSurfaceIdFromName(surfaceName);

                m_surfaceInfos[surfaceId].isAudioConcrete = isConcrete;
                m_surfaceInfos[surfaceId].isAudioGrass = isGrass;
                m_surfaceInfos[surfaceId].isAudioSand = isSand;
                m_surfaceInfos[surfaceId].isAudioGravel = isGravel;
                m_surfaceInfos[surfaceId].isAudioWood = isWood;
                m_surfaceInfos[surfaceId].isAudioWater = isWater;
                m_surfaceInfos[surfaceId].isAudioMetal = isMetal;
                m_surfaceInfos[surfaceId].isAudioLongGrass = isLongGrass;
                m_surfaceInfos[surfaceId].isAudioTile = isTile;
            }

            pLine = CFileLoader::LoadLine(fileId);
        } while (pLine);
    }

    CFileMgr::CloseFile(fileId);
}

// 0x55E5C0
int32 SurfaceInfos_c::GetAdhesionGroup(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].adhesionGroup;
}

// 0x55E5E0
float SurfaceInfos_c::GetTyreGrip(uint32 surfaceId)
{
    return static_cast<float>(m_surfaceInfos[surfaceId].tyreGrip) * 0.1f;
}

// 0x55E600
float SurfaceInfos_c::GetWetMultiplier(uint32 surfaceId)
{
    return static_cast<float>(m_surfaceInfos[surfaceId].wetMult) * 0.01f * CWeather::WetRoads + 1.0f;
}

// 0x55E630
uint32 SurfaceInfos_c::GetSkidmarkType(uint32 surfaceId) {
    return static_cast<uint32>(m_surfaceInfos[surfaceId].skidmarkType);
}

// 0x55E650
int32 SurfaceInfos_c::GetFrictionEffect(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].frictionEffect;
}

// 0x55E670
int32 SurfaceInfos_c::GetBulletFx(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].bulletFx;
}

// 0x55E690
bool SurfaceInfos_c::IsSoftLanding(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isSoftLanding;
}

// 0x55E6B0
bool SurfaceInfos_c::IsSeeThrough(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isSeeThrough;
}

// 0x55E6D0
bool SurfaceInfos_c::IsShootThrough(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isShootThrough;
}

// 0x55E6F0
bool SurfaceInfos_c::IsSand(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isSand;
}

// 0x55E710
bool SurfaceInfos_c::IsWater(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isWater;
}

// 0x55E730
bool SurfaceInfos_c::IsShallowWater(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isShallowWater;
}

// 0x55E750
bool SurfaceInfos_c::IsBeach(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isBeach;
}

// 0x55E770
bool SurfaceInfos_c::IsSteepSlope(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isSteepSlope;
}

// 0x55E790
bool SurfaceInfos_c::IsGlass(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isGlass;
}

// 0x55E7B0
bool SurfaceInfos_c::IsStairs(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isStairs;
}

// 0x55E7D0
bool SurfaceInfos_c::IsSkateable(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isSkateable;
}

// 0x55E7F0
bool SurfaceInfos_c::IsPavement(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isPavement;
}

// 0x55E810
int32 SurfaceInfos_c::GetRoughness(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].roughness;
}

// 0x55E830
int32 SurfaceInfos_c::GetFlammability(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].flammability;
}

// 0x55E850
bool SurfaceInfos_c::CreatesSparks(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].createsSparks;
}

// 0x55E870
bool SurfaceInfos_c::CantSprintOn(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].cantSprintOn;
}

// 0x55E890
bool SurfaceInfos_c::LeavesFootsteps(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].leavesFootsteps;
}

// 0x55E8B0
bool SurfaceInfos_c::ProducesFootDust(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].producesFootDust;
}

// 0x55E8D0
bool SurfaceInfos_c::MakesCarDirty(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].makesCarDirty;
}

// 0x55E8F0
bool SurfaceInfos_c::MakesCarClean(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].makesCarClean;
}

// 0x55E910
bool SurfaceInfos_c::CreatesWheelGrass(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].createsWheelGrass;
}

// 0x55E930
bool SurfaceInfos_c::CreatesWheelGravel(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].createsWheelGravel;
}

// 0x55E950
bool SurfaceInfos_c::CreatesWheelMud(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].createsWheelMud;
}

// 0x55E970
bool SurfaceInfos_c::CreatesWheelDust(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].createsWheelDust;
}

// 0x55E990
bool SurfaceInfos_c::CreatesWheelSand(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].createsWheelSand;
}

// 0x55E9B0
bool SurfaceInfos_c::CreatesWheelSpray(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].createsWheelSpray;
}

// 0x55E9D0
bool SurfaceInfos_c::CreatesPlants(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].createsPlants;
}

// 0x55E9F0
bool SurfaceInfos_c::CreatesObjects(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].createsObjects;
}

// 0x55EA10
bool SurfaceInfos_c::CanClimb(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].canClimb;
}

// 0x55EA30
bool SurfaceInfos_c::IsAudioConcrete(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isAudioConcrete;
}

// 0x55EA50
bool SurfaceInfos_c::IsAudioGrass(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isAudioGrass;
}

// 0x55EA70
bool SurfaceInfos_c::IsAudioSand(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isAudioSand;
}

// 0x55EA90
bool SurfaceInfos_c::IsAudioGravel(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isAudioGravel;
}

// 0x55EAB0
bool SurfaceInfos_c::IsAudioWood(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isAudioWood;
}

// 0x55EAD0
bool SurfaceInfos_c::IsAudioWater(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isAudioWater;
}

// 0x55EAF0
bool SurfaceInfos_c::IsAudioMetal(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isAudioMetal;
}

// 0x55EB10
bool SurfaceInfos_c::IsAudioLongGrass(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isAudioLongGrass;
}

// 0x55EB30
bool SurfaceInfos_c::IsAudioTile(uint32 surfaceId)
{
    return m_surfaceInfos[surfaceId].isAudioTile;
}

// 0x55EB50
float SurfaceInfos_c::GetAdhesiveLimit(CColPoint* colPoint)
{
    uint32 surfA = colPoint->m_nSurfaceTypeA;
    uint32 surfB = colPoint->m_nSurfaceTypeB;

    int32 groupA = m_surfaceInfos[surfA].adhesionGroup;
    int32 groupB = m_surfaceInfos[surfB].adhesionGroup;

    return m_adhesiveLimitTable[groupB][groupA];
}
