#pragma once

enum
{
    SURFACE_TYPE_DEFAULT,
    SURFACE_TYPE_TARMAC,
    SURFACE_TYPE_TARMAC_FUCKED,
    SURFACE_TYPE_TARMAC_REALLYFUCKED,
    SURFACE_TYPE_PAVEMENT,
    SURFACE_TYPE_PAVEMENT_FUCKED,
    SURFACE_TYPE_GRAVEL,
    SURFACE_TYPE_FUCKED_CONCRETE,
    SURFACE_TYPE_PAINTED_GROUND,
    SURFACE_TYPE_GRASS_SHORT_LUSH,
    SURFACE_TYPE_GRASS_MEDIUM_LUSH,
    SURFACE_TYPE_GRASS_LONG_LUSH,
    SURFACE_TYPE_GRASS_SHORT_DRY,
    SURFACE_TYPE_GRASS_MEDIUM_DRY,
    SURFACE_TYPE_GRASS_LONG_DRY,
    SURFACE_TYPE_GOLFGRASS_ROUGH,
    SURFACE_TYPE_GOLFGRASS_SMOOTH,
    SURFACE_TYPE_STEEP_SLIDYGRASS,
    SURFACE_TYPE_STEEP_CLIFF,
    SURFACE_TYPE_FLOWERBED,
    SURFACE_TYPE_MEADOW,
    SURFACE_TYPE_WASTEGROUND,
    SURFACE_TYPE_WOODLANDGROUND,
    SURFACE_TYPE_VEGETATION,
    SURFACE_TYPE_MUD_WET,
    SURFACE_TYPE_MUD_DRY,
    SURFACE_TYPE_DIRT,
    SURFACE_TYPE_DIRTTRACK,
    SURFACE_TYPE_SAND_DEEP,
    SURFACE_TYPE_SAND_MEDIUM,
    SURFACE_TYPE_SAND_COMPACT,
    SURFACE_TYPE_SAND_ARID,
    SURFACE_TYPE_SAND_MORE,
    SURFACE_TYPE_SAND_BEACH,
    SURFACE_TYPE_CONCRETE_BEACH,
    SURFACE_TYPE_ROCK_DRY,
    SURFACE_TYPE_ROCK_WET,
    SURFACE_TYPE_ROCK_CLIFF,
    SURFACE_TYPE_WATER_RIVERBED,
    SURFACE_TYPE_WATER_SHALLOW,
    SURFACE_TYPE_CORNFIELD,
    SURFACE_TYPE_HEDGE,
    SURFACE_TYPE_WOOD_CRATES,
    SURFACE_TYPE_WOOD_SOLID,
    SURFACE_TYPE_WOOD_THIN,
    SURFACE_TYPE_GLASS,
    SURFACE_TYPE_GLASS_WINDOWS_LARGE,
    SURFACE_TYPE_GLASS_WINDOWS_SMALL,
    SURFACE_TYPE_EMPTY1,
    SURFACE_TYPE_EMPTY2,
    SURFACE_TYPE_GARAGE_DOOR,
    SURFACE_TYPE_THICK_METAL_PLATE,
    SURFACE_TYPE_SCAFFOLD_POLE,
    SURFACE_TYPE_LAMP_POST,
    SURFACE_TYPE_METAL_GATE,
    SURFACE_TYPE_METAL_CHAIN_FENCE,
    SURFACE_TYPE_GIRDER,
    SURFACE_TYPE_FIRE_HYDRANT,
    SURFACE_TYPE_CONTAINER,
    SURFACE_TYPE_NEWS_VENDOR,
    SURFACE_TYPE_WHEELBASE,
    SURFACE_TYPE_CARDBOARDBOX,
    SURFACE_TYPE_PED,
    SURFACE_TYPE_CAR,
    SURFACE_TYPE_CAR_PANEL,
    SURFACE_TYPE_CAR_MOVINGCOMPONENT,
    SURFACE_TYPE_TRANSPARENT_CLOTH,
    SURFACE_TYPE_RUBBER,
    SURFACE_TYPE_PLASTIC,
    SURFACE_TYPE_TRANSPARENT_STONE,
    SURFACE_TYPE_WOOD_BENCH,
    SURFACE_TYPE_CARPET,
    SURFACE_TYPE_FLOORBOARD,
    SURFACE_TYPE_STAIRSWOOD,
    SURFACE_TYPE_P_SAND,
    SURFACE_TYPE_P_SAND_DENSE,
    SURFACE_TYPE_P_SAND_ARID,
    SURFACE_TYPE_P_SAND_COMPACT,
    SURFACE_TYPE_P_SAND_ROCKY,
    SURFACE_TYPE_P_SANDBEACH,
    SURFACE_TYPE_P_GRASS_SHORT,
    SURFACE_TYPE_P_GRASS_MEADOW,
    SURFACE_TYPE_P_GRASS_DRY,
    SURFACE_TYPE_P_WOODLAND,
    SURFACE_TYPE_P_WOODDENSE,
    SURFACE_TYPE_P_ROADSIDE,
    SURFACE_TYPE_P_ROADSIDEDES,
    SURFACE_TYPE_P_FLOWERBED,
    SURFACE_TYPE_P_WASTEGROUND,
    SURFACE_TYPE_P_CONCRETE,
    SURFACE_TYPE_P_OFFICEDESK,
    SURFACE_TYPE_P_711SHELF1,
    SURFACE_TYPE_P_711SHELF2,
    SURFACE_TYPE_P_711SHELF3,
    SURFACE_TYPE_P_RESTUARANTTABLE,
    SURFACE_TYPE_P_BARTABLE,
    SURFACE_TYPE_P_UNDERWATERLUSH,
    SURFACE_TYPE_P_UNDERWATERBARREN,
    SURFACE_TYPE_P_UNDERWATERCORAL,
    SURFACE_TYPE_P_UNDERWATERDEEP,
    SURFACE_TYPE_P_RIVERBED,
    SURFACE_TYPE_P_RUBBLE,
    SURFACE_TYPE_P_BEDROOMFLOOR,
    SURFACE_TYPE_P_KIRCHENFLOOR,
    SURFACE_TYPE_P_LIVINGRMFLOOR,
    SURFACE_TYPE_P_CORRIDORFLOOR,
    SURFACE_TYPE_P_711FLOOR,
    SURFACE_TYPE_P_FASTFOODFLOOR,
    SURFACE_TYPE_P_SKANKYFLOOR,
    SURFACE_TYPE_P_MOUNTAIN,
    SURFACE_TYPE_P_MARSH,
    SURFACE_TYPE_P_BUSHY,
    SURFACE_TYPE_P_BUSHYMIX,
    SURFACE_TYPE_P_BUSHYDRY,
    SURFACE_TYPE_P_BUSHYMID,
    SURFACE_TYPE_P_GRASSWEEFLOWERS,
    SURFACE_TYPE_P_GRASSDRYTALL,
    SURFACE_TYPE_P_GRASSLUSHTALL,
    SURFACE_TYPE_P_GRASSGRNMIX,
    SURFACE_TYPE_P_GRASSBRNMIX,
    SURFACE_TYPE_P_GRASSLOW,
    SURFACE_TYPE_P_GRASSROCKY,
    SURFACE_TYPE_P_GRASSSMALLTREES,
    SURFACE_TYPE_P_DIRTROCKY,
    SURFACE_TYPE_P_DIRTWEEDS,
    SURFACE_TYPE_P_GRASSWEEDS,
    SURFACE_TYPE_P_RIVEREDGE,
    SURFACE_TYPE_P_POOLSIDE,
    SURFACE_TYPE_P_FORESTSTUMPS,
    SURFACE_TYPE_P_FORESTSTICKS,
    SURFACE_TYPE_P_FORRESTLEAVES,
    SURFACE_TYPE_P_DESERTROCKS,
    SURFACE_TYPE_P_FORRESTDRY,
    SURFACE_TYPE_P_SPARSEFLOWERS,
    SURFACE_TYPE_P_BUILDINGSITE,
    SURFACE_TYPE_P_DOCKLANDS,
    SURFACE_TYPE_P_INDUSTRIAL,
    SURFACE_TYPE_P_INDUSTJETTY,
    SURFACE_TYPE_P_CONCRETELITTER,
    SURFACE_TYPE_P_ALLEYRUBISH,
    SURFACE_TYPE_P_JUNKYARDPILES,
    SURFACE_TYPE_P_JUNKYARDGRND,
    SURFACE_TYPE_P_DUMP,
    SURFACE_TYPE_P_CACTUSDENSE,
    SURFACE_TYPE_P_AIRPORTGRND,
    SURFACE_TYPE_P_CORNFIELD,
    SURFACE_TYPE_P_GRASSLIGHT,
    SURFACE_TYPE_P_GRASSLIGHTER,
    SURFACE_TYPE_P_GRASSLIGHTER2,
    SURFACE_TYPE_P_GRASSMID1,
    SURFACE_TYPE_P_GRASSMID2,
    SURFACE_TYPE_P_GRASSDARK,
    SURFACE_TYPE_P_GRASSDARK2,
    SURFACE_TYPE_P_GRASSDIRTMIX,
    SURFACE_TYPE_P_RIVERBEDSTONE,
    SURFACE_TYPE_P_RIVERBEDSHALLOW,
    SURFACE_TYPE_P_RIVERBEDWEEDS,
    SURFACE_TYPE_P_SEAWEED,
    SURFACE_TYPE_DOOR,
    SURFACE_TYPE_PLASTICBARRIER,
    SURFACE_TYPE_PARKGRASS,
    SURFACE_TYPE_STAIRSSTONE,
    SURFACE_TYPE_STAIRSMETAL,
    SURFACE_TYPE_STAIRSCARPET,
    SURFACE_TYPE_FLOORMETAL,
    SURFACE_TYPE_FLOORCONCRETE,
    SURFACE_TYPE_BIN_BAG,
    SURFACE_TYPE_THIN_METAL_SHEET,
    SURFACE_TYPE_METAL_BARREL,
    SURFACE_TYPE_PLASTIC_CONE,
    SURFACE_TYPE_PLASTIC_DUMPSTER,
    SURFACE_TYPE_METAL_DUMPSTER,
    SURFACE_TYPE_WOOD_PICKET_FENCE,
    SURFACE_TYPE_WOOD_SLATTED_FENCE,
    SURFACE_TYPE_WOOD_RANCH_FENCE,
    SURFACE_TYPE_UNBREAKABLE_GLASS,
    SURFACE_TYPE_HAY_BALE,
    SURFACE_TYPE_GORE,
    SURFACE_TYPE_RAILTRACK,
    CAR_RAILTRACK_SURFACETYPE = SURFACE_TYPE_RAILTRACK,
    NUM_SURFACE_TYPES
};

enum
{
    ADHESION_GROUP_RUBBER,
    ADHESION_GROUP_HARD,
    ADHESION_GROUP_ROAD,
    ADHESION_GROUP_LOOSE,
    ADHESION_GROUP_SAND,
    ADHESION_GROUP_WET,
    NUM_ADHESION_GROUPS
};

enum
{
    FRICTION_FX_NONE,
    FRICTION_FX_SPARKS
};

enum
{
    BULLETFX_NONE,
    BULLETFX_SPARKS,
    BULLETFX_SAND,
    BULLETFX_WOOD,
    BULLETFX_DUST
};

struct SurfaceInfo_t {
    signed char tyreGrip;
    signed char wetMult;

    uint32_t adhesionGroup : 3;
    uint32_t skidmarkType : 2;
    uint32_t frictionEffect : 3;
    uint32_t bulletFx : 3;
    uint32_t isSoftLanding : 1;
    uint32_t isSeeThrough : 1;
    uint32_t isShootThrough : 1;
    uint32_t isSand : 1;
    uint32_t isWater : 1;
    uint32_t isShallowWater : 1;
    uint32_t isBeach : 1;
    uint32_t isSteepSlope : 1;
    uint32_t isGlass : 1;
    uint32_t isStairs : 1;
    uint32_t isSkateable : 1;
    uint32_t isPavement : 1;
    uint32_t roughness : 2;
    uint32_t flammability : 2;
    uint32_t createsSparks : 1;
    uint32_t cantSprintOn : 1;
    uint32_t leavesFootsteps : 1;
    uint32_t producesFootDust : 1;

    uint32_t makesCarDirty : 1;
    uint32_t makesCarClean : 1;
    uint32_t createsWheelGrass : 1;
    uint32_t createsWheelGravel : 1;
    uint32_t createsWheelMud : 1;
    uint32_t createsWheelDust : 1;
    uint32_t createsWheelSand : 1;
    uint32_t createsWheelSpray : 1;
    uint32_t createsPlants : 1;
    uint32_t createsObjects : 1;
    uint32_t canClimb : 1;

    uint32_t isAudioConcrete : 1;
    uint32_t isAudioGrass : 1;
    uint32_t isAudioSand : 1;
    uint32_t isAudioGravel : 1;
    uint32_t isAudioWood : 1;
    uint32_t isAudioWater : 1;
    uint32_t isAudioMetal : 1;
    uint32_t isAudioLongGrass : 1;
    uint32_t isAudioTile : 1;
};

//static_assert(sizeof(SurfaceInfo_t) == 0xC, "SurfaceInfo_t size mismatch");

class SurfaceInfos_c
{
public:
    static void InjectHooks();

    SurfaceInfos_c();
    ~SurfaceInfos_c();

    void Init();

    uint32 GetSurfaceIdFromName(char* surfaceName);

    int32 GetAdhesionGroup(uint32 surfaceId);
    float GetTyreGrip(uint32 surfaceId);
    float GetWetMultiplier(uint32 surfaceId);

    uint32 GetSkidmarkType(uint32 surfaceId);
    int32 GetFrictionEffect(uint32 surfaceId);
    int32 GetBulletFx(uint32 surfaceId);

    bool IsSoftLanding(uint32 surfaceId);
    bool IsSeeThrough(uint32 surfaceId);
    bool IsShootThrough(uint32 surfaceId);

    bool IsSand(uint32 surfaceId);
    bool IsWater(uint32 surfaceId);
    bool IsShallowWater(uint32 surfaceId);
    bool IsBeach(uint32 surfaceId);
    bool IsSteepSlope(uint32 surfaceId);
    bool IsGlass(uint32 surfaceId);
    bool IsStairs(uint32 surfaceId);
    bool IsSkateable(uint32 surfaceId);
    bool IsPavement(uint32 surfaceId);

    int32 GetRoughness(uint32 surfaceId);
    int32 GetFlammability(uint32 surfaceId);

    bool CreatesSparks(uint32 surfaceId);
    bool CantSprintOn(uint32 surfaceId);
    bool LeavesFootsteps(uint32 surfaceId);
    bool ProducesFootDust(uint32 surfaceId);
    bool MakesCarDirty(uint32 surfaceId);
    bool MakesCarClean(uint32 surfaceId);
    bool CreatesWheelGrass(uint32 surfaceId);
    bool CreatesWheelGravel(uint32 surfaceId);
    bool CreatesWheelMud(uint32 surfaceId);
    bool CreatesWheelDust(uint32 surfaceId);
    bool CreatesWheelSand(uint32 surfaceId);
    bool CreatesWheelSpray(uint32 surfaceId);
    bool CreatesPlants(uint32 surfaceId);
    bool CreatesObjects(uint32 surfaceId);
    bool CanClimb(uint32 surfaceId);

    bool IsAudioConcrete(uint32 surfaceId);
    bool IsAudioGrass(uint32 surfaceId);
    bool IsAudioSand(uint32 surfaceId);
    bool IsAudioGravel(uint32 surfaceId);
    bool IsAudioWood(uint32 surfaceId);
    bool IsAudioWater(uint32 surfaceId);
    bool IsAudioMetal(uint32 surfaceId);
    bool IsAudioLongGrass(uint32 surfaceId);
    bool IsAudioTile(uint32 surfaceId);

    float GetAdhesiveLimit(CColPoint* colPoint);

private:
    void LoadAdhesiveLimits();
    void LoadSurfaceInfos();
    void LoadSurfaceAudioInfos();

    float m_adhesiveLimitTable[6][6];

    SurfaceInfo_t m_surfaceInfos[209];
};

//static_assert(sizeof(SurfaceInfos_c) == 0xA5C, "SurfaceInfos_c size mismatch");

extern SurfaceInfos_c g_surfaceInfos;
