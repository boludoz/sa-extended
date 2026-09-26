/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Physical.h"
#include "ObjectData.h"

enum eObjectType {
    OBJECT_UNKNOWN         = 0,
    OBJECT_GAME            = 1,
    OBJECT_MISSION         = 2,
    OBJECT_TEMPORARY       = 3, // AKA OBJECT_TYPE_FLYING_COMPONENT
    OBJECT_TYPE_CUTSCENE   = 4,
    OBJECT_TYPE_DECORATION = 5, // Hand object, projectiles, escalator step, water creatures, no clue what this enum value should be called
    OBJECT_MISSION2        = 6
};

class CDummyObject;
class CFire;

class NOTSA_EXPORT_VTABLE CObject : public CPhysical {
public:
    CPtrNodeDoubleLink<CObject*>* m_pControlCodeList;
    uint8                         m_nObjectType; // see enum eObjectType
    uint8                         m_nBonusValue;
    uint16                        m_wCostValue;
    //! calineva API
    enum {
        OBJECT_DOESNT_USE_SCRIPT_BRAIN,
        OBJECT_SCRIPT_BRAIN_NOT_LOADED,
        OBJECT_WAITING_FOR_SCRIPT_BRAIN_TO_LOAD,
        OBJECT_RUNNING_SCRIPT_BRAIN
    };

    struct CObjectFlags {
        uint32 bIsPickUp : 1;               // 0x1
        uint32 bNoPickUpEffects : 1;                   // 0x2 - collision related
        uint32 bPickUpCostsMoney : 1;  // 0x4
        uint32 bPickUpOutOfStock : 1; // 0x8
        uint32 bGlassShattered : 1;         // 0x10
        uint32 bGlassBrokenAltogether : 1;  // 0x20
        uint32 bHasExploded : 1;             // 0x40
        uint32 bParentIsACar : 1;        // 0x80

        uint32 bLampPostCollision : 1;
        uint32 bCanBeTargettedByPlayer : 1;
        uint32 bHasBeenShattered : 1;
        uint32 bTrainNearby : 1;
        uint32 bHasBeenPhotographed : 1;
        uint32 bIsStealable : 1;
        uint32 bWasDoorLocked : 1;
        uint32 bDoorOpenedEnough : 1;

        uint32 bReferencedCollision : 1;
        uint32 bScaled : 1;
        uint32 bWinchCanPickMeUp : 1;
        uint32 bLandedOnMovingCol : 1;
        uint32 ScriptBrainStatus : 2; // something something scripts for brains
        uint32 bFadeOut : 1; // works only for objects with type 2 (OBJECT_MISSION)
        uint32 bCalculateLighting : 1;

        uint32 bEnableDisabledAttractors : 1;
        uint32 bDoPreRenderButDontRender : 1;
        uint32 bFadingIn2 : 1;
        uint32 b0x08000000 : 1;
        uint32 b0x10000000 : 1;
        uint32 b0x20000000 : 1;
        uint32 b0x40000000 : 1;
        uint32 b0x80000000 : 1;
    };
    union {
        CObjectFlags m_nObjectFlags;
        uint32 m_nObjectFlagsRaw;
    };
    uint8         m_nColDamageEffect;        // see eObjectColDamageEffect
    uint8         m_nSpecialColResponseCase; // see eObjectSpecialColResponseCases
    char          field_146;
    int8          m_nGarageDoorGarageIndex;
    uint8         m_nLastWeaponDamage;
    tColLighting  m_nColLighting;
    int16         m_nRefModelIndex;
    uint8         m_nCarColor[4];  // this is used for detached car parts
    uint32        m_nRemovalTime;  // time when this object must be deleted
    float         m_fHealth;
    float         m_fDoorStartAngle; // this is used for door objects
    float         m_fScale;
    CObjectData*  m_pObjectInfo;
    CFire*        m_pFire;
    int16         StreamedScriptBrainToLoad;
    int16         m_wRemapTxd;     // this is used for detached car parts
    RwTexture*    m_pRemapTexture; // this is used for detached car parts
    CDummyObject* m_pDummyObject;  // used for dynamic objects like garage doors, train crossings etc.
    uint32        m_nBurnTime;     // time when particles must be stopped
    float         m_fBurnDamage;

    static inline auto& nNoTempObjects = StaticRef<uint16>(0xBB4A70);
    static inline auto& fDistToNearestTree = StaticRef<float>(0x8D0A20);
    static inline auto& bAircraftCarrierSamSiteDisabled = StaticRef<bool>(0x8D0A24);
    static inline auto& bArea51SamSiteDisabled = StaticRef<bool>(0xBB4A72);

public:
    CObject();
    CObject(int32 modelId, bool bCreate);
    explicit CObject(CDummyObject* dummyObj);
    ~CObject() override;

    static void* operator new(size_t size);
    static void* operator new(size_t size, int32 poolRef);
    static void operator delete(void* obj);
    static void operator delete(void* obj, int32 poolRef);

    void  SetIsStatic(bool isStatic) override;
    void  CreateRwObject() override;
    void  ProcessControl() override;
    void  Teleport(CVector destination, bool resetRotation) override;
    void  SpecialEntityPreCollisionStuff(CPhysical* colPhysical, bool bIgnoreStuckCheck, bool& bCollisionDisabled, bool& bCollidedEntityCollisionIgnored, bool& bCollidedEntityUnableToMove, bool& bThisOrCollidedEntityStuck) override;
    uint8 SpecialEntityCalcCollisionSteps(bool& bProcessCollisionBeforeSettingTimeStep, bool& unk2) override;
    void  PreRender() override;
    void  Render() override;
    bool  SetupLighting() override;
    void  RemoveLighting(bool bRemove) override;

    bool Load();
    bool Save();

    void     ProcessGarageDoorBehaviour();
    [[nodiscard]] bool CanBeDeleted() const;
    void     SetRelatedDummy(CDummyObject* relatedDummy);
    bool     TryToExplode();
    void     SetObjectTargettable(bool targetable);
    [[nodiscard]] bool CanBeTargetted() const;
    [[nodiscard]] bool IsObjectDamaged() const { return m_nObjectFlags.bLandedOnMovingCol; }
    void     RefModelInfo(int32 modelIndex);
    void     SetRemapTexture(RwTexture* remapTexture, int16 txdIndex);
    float    GetRopeHeight();
    void     SetRopeHeight(float height);
    CEntity* GetObjectCarriedWithRope();
    void     ReleaseObjectCarriedWithRope();
    void     AddToControlCodeList();
    void     RemoveFromControlCodeList();
    void     ResetDoorAngle();
    void     LockDoor();
    void     Init();
    void     DoBurnEffect() const;
    void     GetLightingFromCollisionBelow();
    void     ProcessSamSiteBehaviour();
    void     ProcessTrainCrossingBehaviour();
    void     ObjectDamage(float damage, const CVector* fxOrigin, const CVector* fxDirection, CEntity* damager, eWeaponType weaponType);
    void     Explode();
    void     ObjectFireDamage(float damage, CEntity* damager);

    void GrabObjectToCarryWithRope(CPhysical* attachTo);
    bool CanBeUsedToTakeCoverBehind();
    void ProcessControlLogic();

    static CObject* Create(int32 modelIndex, bool bUnused);
    static CObject* Create(CDummyObject* dummyObject);

    static void SetMatrixForTrainCrossing(CMatrix* matrix, float fAngle);
    static void TryToFreeUpTempObjects(int32 numObjects);
    static void DeleteAllTempObjects();
    static void DeleteAllMissionObjects();
    static void DeleteAllTempObjectsInArea(CVector point, float radius);

    // Helpers
    [[nodiscard]] bool IsTemporary() const     { return m_nObjectType == OBJECT_TEMPORARY; }
    [[nodiscard]] bool IsMissionObject() const { return m_nObjectType == OBJECT_MISSION || m_nObjectType == OBJECT_MISSION2; }
    [[nodiscard]] bool IsCraneMovingPart() const;
    [[nodiscard]] bool IsFallenLampPost() const { return m_nObjectFlags.bLampPostCollision && m_matrix->GetUp().z < 0.66F; }
    [[nodiscard]] bool IsExploded() const       { return m_nObjectFlags.bHasExploded; }
    [[nodiscard]] bool CanBeSmashed() const     { return m_nColDamageEffect >= COL_DAMAGE_EFFECT_SMASH_COMPLETELY; }

private:
    friend void InjectHooksMain();
    static void InjectHooks();

};
VALIDATE_SIZE(CObject, 0x17C);

bool IsObjectPointerValid_NotInWorld(CObject* object);
bool IsObjectPointerValid(CObject* object);
