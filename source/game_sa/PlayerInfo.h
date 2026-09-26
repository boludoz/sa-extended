/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "RenderWare.h"
#include "Vector.h"
#include "PtrListDoubleLink.h"

class CPed;
class CVehicle;
class CPlayerPedData;
class CPlayerPed;
class CPlayerInfoSaveStructure;

enum ePlayerState : uint8 {
    PLAYERSTATE_PLAYING,
    PLAYERSTATE_HAS_DIED,
    PLAYERSTATE_HAS_BEEN_ARRESTED,
    PLAYERSTATE_FAILED_MISSION,
    PLAYERSTATE_LEFT_GAME
};

class CPlayerInfo {
public:
    CPlayerPed*    pPed;                    // Pointer to the player ped (should always be set)
    CPlayerPedData PlayerPedData;              // instance of player variables
    CVehicle*      pRemoteVehicle;          // Pointer to vehicle player is driving remotely at the moment.(NULL if on foot)
    CVehicle*      pSpecCar;                // which car is using the special collision model
    int32          Score;                  // Points for this player
    int32          DisplayScore;           // Points as they would be displayed
    uint32         CollectablesPickedUp;   // How many bags of sugar do we have
    uint32         TotalNumCollectables;   // How many bags of sugar are there to be had in the game
    uint32         nLastBumpPlayerCarTimer; // Keeps track of when the last ped bumped into the player car
    uint32         TaxiTimer;              // Keeps track of how long the player has been in a taxi with a passenger (in msecs)
    uint32         vehicle_time_counter;     // keeps track of how long player has been in car for driving skill
    bool           bTaxiTimerScore;         // If TRUE then add 1 to score for each second that the player is driving a taxi
    bool           m_bTryingToExitCar;        // if player holds exit car button, want to trigger getout once car slowed enough with a passenger
    CVehicle*      pLastTargetVehicle;      // Last vehicle player tried to enter.
    ePlayerState   PlayerState;
    bool           bAfterRemoteVehicleExplosion;
    bool           bCreateRemoteVehicleExplosion;
    bool           bFadeAfterRemoteVehicleExplosion;
    uint32         TimeOfRemoteVehicleExplosion;
    uint32         LastTimeEnergyLost;     // To make numbers flash on the HUD
    uint32         LastTimeArmourLost;
    uint32         LastTimeBigGunFired;   // Tank guns etc
    uint32         TimesUpsideDownInARow; // Make car blow up if car upside down
    uint32         TimesStuckInARow;      // Make car blow up if player cannot get out.
    uint32         nCarTwoWheelCounter;    // how long has player's car been on two wheels
    float          fCarTwoWheelDist;       // Make car blow up if player cannot get out.
    uint32         nCarLess3WheelCounter;  // how long has player's car been on less than 3 wheels
    uint32         nBikeRearWheelCounter;  // how long has player's bike been on rear wheel only
    float          fBikeRearWheelDist;
    uint32         nBikeFrontWheelCounter; // how long has player's bike been on front wheel only
    float          fBikeFrontWheelDist;
    uint32         nTempBufferCounter;     // so wheels can leave the ground for a few frames without stopping above counters
    uint32         nBestCarTwoWheelsTimeMs;
    float          fBestCarTwoWheelsDistM;
    uint32         nBestBikeWheelieTimeMs;
    float          fBestBikeWheelieDistM;
    uint32         nBestBikeStoppieTimeMs;
    float          fBestBikeStoppieDistM;
    uint16         CarDensityForCurrentZone;
    float          RoadDensityAroundPlayer; // 1.0f for an average city.
    uint32         TimeOfLastCarExplosionCaused;
    uint32         ExplosionMultiplier;
    uint32         HavocCaused;        // A counter going up when the player does bad stuff.
    uint16         TimeLastEaten;
    float          CurrentChaseValue;  // How 'ill' is the chase at the moment
    bool           DoesNotGetTired;
    bool           FastReload;
    bool           FireProof;
    uint8          MaxHealth;
    uint8          MaxArmour;
    bool           bGetOutOfJailFree; // Player doesn't lose money/weapons next time arrested
    bool           bFreeHealthCare;   // Player doesn't lose money nexed time patched up at hospital
    bool           bCanDoDriveBy;
    uint8          m_nBustedAudioStatus;
    uint16         m_nLastBustMessageNumber;
    uint32         m_nCrosshairActivated;
    CVector2D      m_vecCrosshairTarget; // -1 ... 1 on screen
    char           m_skinName[32];
    RwTexture*     m_pSkinTexture;
    bool           m_bParachuteReferenced;
    uint32         m_nRequireParachuteTimer;

public:
    CPlayerInfo();
    CPlayerInfo(const CPlayerInfo&) = delete;
    CPlayerInfo(CPlayerInfo&&) = delete;
    ~CPlayerInfo() = default; // 0x45B110

    CPlayerInfo& operator=(const CPlayerInfo& rhs);

    static void CancelPlayerEnteringCars(CVehicle* vehicle);
    static CEntity* FindObjectToSteal(CPed* ped);

    void Process(uint32 playerIndex);
    void FindClosestCarSectorList(CPtrListDoubleLink<CVehicle*>& ptrList, CPed* ped, float minX, float minY, float maxX, float maxY, float* outVehDist, CVehicle** outVehicle);
    void EvaluateCarPosition(CEntity* car, CPed* ped, float pedToVehDist, float* outDistance, CVehicle** outVehicle);
    void Clear();
    void GivePlayerParachute() const;
    void StreamParachuteWeapon(bool a2);
    void AddHealth(int32 amount) const;
    void BlowUpRCBuggy(bool bExplode) const;
    void MakePlayerSafe(bool canMove, float radius);
    void PlayerFailedCriticalMission();
    void WorkOutEnergyFromHunger();
    void ArrestPlayer();
    void KillPlayer();
    void LoadPlayerSkin();
    void DeletePlayerSkin();
    void SetPlayerSkin(const char* name);
    void SetLastTargetVehicle(CVehicle* vehicle);
    CVehicle* GetLastTargetVehicle() const { return pLastTargetVehicle; } //!< calineva API
    [[nodiscard]] bool IsRestartingAfterMissionFailed() const;
    [[nodiscard]] bool IsRestartingAfterArrest() const;
    [[nodiscard]] bool IsRestartingAfterDeath() const;
    [[nodiscard]] bool IsPlayerInRemoteMode() const;

    CVector GetPos() const;
    CVector GetSpeed() const;

    bool Load();
    bool Save();

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CVector* GetSpeed_Hook(CVector* out);
    CVector* GetPos_Hook(CVector* outPos);

    CPlayerInfo* Constructor();
};

VALIDATE_SIZE(CPlayerInfo, 0x190);

class CPlayerInfoSaveStructure {
    int32        m_nMoney;
    uint16       m_nCarDensityForCurrentZone;
    ePlayerState m_nPlayerState;
    float        m_fRoadDensityAroundPlayer;
    int32        m_nDisplayMoney;
    uint8        m_nNumHoursDidntEat;
    uint32       m_nCollectablesPickedUp;
    uint32       m_nTotalNumCollectables;
    bool         m_bDoesNotGetTired;
    bool         m_bFastReload;
    bool         m_bFireProof;
    uint8        m_nMaxHealth;
    uint8        m_nMaxArmour;
    bool         m_bGetOutOfJailFree;
    bool         m_bFreeHealthCare;
    bool         m_bCanDoDriveBy;
    uint8        m_nBustedAudioStatus;
    uint16       m_nLastBustMessageNumber;

public:
    void Construct(CPlayerInfo* info) {
        m_nMoney =                          info->Score;
        m_nCarDensityForCurrentZone =       info->CarDensityForCurrentZone;
        m_nPlayerState =                    info->PlayerState;
        m_fRoadDensityAroundPlayer =        info->RoadDensityAroundPlayer;
        m_nDisplayMoney =                   info->DisplayScore;
        m_nNumHoursDidntEat =               static_cast<uint8>(info->TimeLastEaten);
        m_nCollectablesPickedUp =           info->CollectablesPickedUp;
        m_nTotalNumCollectables =           info->TotalNumCollectables;
        m_bDoesNotGetTired =                info->DoesNotGetTired;
        m_bFastReload =                     info->FastReload;
        m_bFireProof =                      info->FireProof;
        m_nMaxHealth =                      info->MaxHealth;
        m_nMaxArmour =                      info->MaxArmour;
        m_bGetOutOfJailFree =               info->bGetOutOfJailFree;
        m_bFreeHealthCare =                 info->bFreeHealthCare;
        m_bCanDoDriveBy =                   info->bCanDoDriveBy;
        m_nBustedAudioStatus =              info->m_nBustedAudioStatus;
        m_nLastBustMessageNumber =          info->m_nLastBustMessageNumber;
    }

    void Extract(CPlayerInfo* info) {
        info->Score =                    m_nMoney;
        info->CarDensityForCurrentZone = m_nCarDensityForCurrentZone;
        info->PlayerState =              m_nPlayerState;
        // info->m_nCarDensityForCurrentZone = m_nCarDensityForCurrentZone; redundant
        info->RoadDensityAroundPlayer =  m_fRoadDensityAroundPlayer;
        info->DisplayScore =             m_nDisplayMoney;
        info->TimeLastEaten =         m_nNumHoursDidntEat;
        info->CollectablesPickedUp =     m_nCollectablesPickedUp;
        info->TotalNumCollectables =     m_nTotalNumCollectables;
        info->DoesNotGetTired =          m_bDoesNotGetTired;
        info->FastReload =               m_bFastReload;
        info->FireProof =                m_bFireProof;
        info->MaxHealth =                m_nMaxHealth;
        info->MaxArmour =                m_nMaxArmour;
        info->bGetOutOfJailFree =         m_bGetOutOfJailFree;
        info->bFreeHealthCare =           m_bFreeHealthCare;
        info->bCanDoDriveBy =             m_bCanDoDriveBy;
        info->m_nBustedAudioStatus =        m_nBustedAudioStatus;
        info->m_nLastBustMessageNumber =    m_nLastBustMessageNumber;
    }
};

VALIDATE_SIZE(CPlayerInfoSaveStructure, 0x28);
