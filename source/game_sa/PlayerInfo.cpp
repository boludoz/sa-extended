#include "StdInc.h"

#include "PlayerInfo.h"
#include "FireManager.h"
#include "MenuSystem.h"
#include "Hud.h"

void CPlayerInfo::InjectHooks() {
    RH_ScopedClass(CPlayerInfo);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Constructor, 0x571920, { .enabled = false, .locked = true }); // hooking ctor will produce bugs with weapons, you will never give weapon through cheat or something
    RH_ScopedInstall(CancelPlayerEnteringCars, 0x56E860);
    RH_ScopedInstall(FindObjectToSteal, 0x56DBD0, { .reversed = false });
    RH_ScopedInstall(EvaluateCarPosition, 0x56DAD0);
    RH_ScopedInstall(Process, 0x56F8D0, { .reversed = false });
    RH_ScopedInstall(FindClosestCarSectorList, 0x56F4E0, { .reversed = false });
    RH_ScopedInstall(Clear, 0x56F330);
    RH_ScopedInstall(StreamParachuteWeapon, 0x56EB30);
    RH_ScopedInstall(AddHealth, 0x56EAB0);
    RH_ScopedInstall(ArrestPlayer, 0x56E5D0);
    RH_ScopedInstall(SetPlayerSkin, 0x5717F0);
    RH_ScopedInstall(LoadPlayerSkin, 0x56F7D0);
    RH_ScopedInstall(DeletePlayerSkin, 0x56EA80);
    RH_ScopedInstall(BlowUpRCBuggy, 0x56EA30);
    RH_ScopedInstall(MakePlayerSafe, 0x56E870);
    RH_ScopedInstall(PlayerFailedCriticalMission, 0x56E830);
    RH_ScopedInstall(WorkOutEnergyFromHunger, 0x56E610);
    RH_ScopedInstall(KillPlayer, 0x56E580);
    RH_ScopedInstall(IsRestartingAfterMissionFailed, 0x56E570);
    RH_ScopedInstall(IsRestartingAfterArrest, 0x56E560);
    RH_ScopedInstall(IsRestartingAfterDeath, 0x56E550);
    RH_ScopedInstall(IsPlayerInRemoteMode, 0x56DAB0);
    RH_ScopedInstall(GetPos_Hook, 0x56DFB0);
    RH_ScopedInstall(GetSpeed_Hook, 0x56DF50);
    RH_ScopedInstall(GivePlayerParachute, 0x56EC40);
    RH_ScopedInstall(SetLastTargetVehicle, 0x56DA80);
    RH_ScopedInstall(Load, 0x5D3B00);
    RH_ScopedInstall(Save, 0x5D3AC0);
}

// 0x571920
// ASM Match: not measured
CPlayerInfo::CPlayerInfo()
{
    PlayerPedData = CPlayerPedData();

    m_pSkinTexture = nullptr;
    m_bParachuteReferenced = false;
    m_nRequireParachuteTimer = 0;
}

// 0x571920
CPlayerInfo* CPlayerInfo::Constructor() {
    this->CPlayerInfo::CPlayerInfo();
    return this;
}

CVector* CPlayerInfo::GetSpeed_Hook(CVector* out) {
    *out = GetSpeed();
    return out;
}

CVector* CPlayerInfo::GetPos_Hook(CVector* outPos) {
    *outPos = GetPos();
    return outPos;
}

// 0x56E860
void CPlayerInfo::CancelPlayerEnteringCars(CVehicle* vehicle) {
    // NOP
}

// 0x56DBD0
CEntity* CPlayerInfo::FindObjectToSteal(CPed* ped) {
    return plugin::CallAndReturn<CEntity*, 0x56DBD0, CPed*>(ped);
}

// 0x56DAD0
void CPlayerInfo::EvaluateCarPosition(CEntity* car, CPed* ped, float pedToVehDist, float* outDistance, CVehicle** outVehicle) {
    const auto& carPosn = car->GetPosition();
    const auto& pedPosn = ped->GetPosition();
    const auto& forward = ped->GetForward();

    // Find our rotation (so that is, at which angle the forward vector is)
    const auto angleFront = CGeneral::GetATanOfXY(forward.x, forward.y);

    // Find car angle, not relative to our rotation
    const auto carAngle = CGeneral::GetATanOfXY(carPosn.x - pedPosn.x, carPosn.y - pedPosn.y);

    // Make car's angle relative to our rotation (notice: it's an abs value)
    // Basically, we calculate how much the car is in our FOV.
    const auto carAngleFromFront = std::abs(CGeneral::LimitRadianAngle(angleFront - carAngle));

    // Calculate imaginary distance based on the car's angle: The higher the angle the greater the distance
    const auto distance = (1.0f - carAngleFromFront / TWO_PI) * (10.0f - pedToVehDist);
    if (distance >= *outDistance) {
        *outDistance = distance;
        *outVehicle = car->AsVehicle();
    }
}

// 0x56F7D0
void CPlayerInfo::LoadPlayerSkin() {
    DeletePlayerSkin();
    m_pSkinTexture = CPlayerSkin::GetSkinTexture(m_skinName);
}

// 0x56EA80
void CPlayerInfo::DeletePlayerSkin() {
    if (m_pSkinTexture) {
        RwTextureDestroy(m_pSkinTexture);
        m_pSkinTexture = nullptr;
    }
}

// 0x5717F0
void CPlayerInfo::SetPlayerSkin(const char* name) {
    strcpy_s(m_skinName, name); // NOTSA: They used `strcpy`, we use `_s` for safety
    LoadPlayerSkin();
}

// 0x56DA80
void CPlayerInfo::SetLastTargetVehicle(CVehicle* vehicle) {
    CEntity::SafeCleanUpRef(pLastTargetVehicle);
    pLastTargetVehicle = vehicle;
    CEntity::SafeRegisterRef(pLastTargetVehicle);
}

// 0x56F8D0
void CPlayerInfo::Process(uint32 playerIndex) {
    plugin::CallMethod<0x56F8D0, CPlayerInfo*, uint32>(this, playerIndex);
}

// 0x56F4E0
void CPlayerInfo::FindClosestCarSectorList(CPtrListDoubleLink<CVehicle*>& ptrList, CPed* ped, float minX, float minY, float maxX, float maxY, float* outVehDist, CVehicle** outVehicle) {
    plugin::CallMethod<0x56F4E0, CPlayerInfo*, CPtrListDoubleLink<CVehicle*>&, CPed*, float, float, float, float, float*, CVehicle**>(this, ptrList, ped, minX, minY, maxX, maxY, outVehDist, outVehicle);
}

// 0x56F330
void CPlayerInfo::Clear() {
    // TODO: This should just use the constructor and `swap`,
    // like to just swap ourselves to a default constructed object
    // and do all the cleanup in the destructor

    pPed = nullptr;
    pRemoteVehicle = nullptr;
    if (pSpecCar) {
        pSpecCar->m_nPhysicalFlags.bUsingSpecialColModel = false;
        pSpecCar = nullptr;
    }
    DisplayScore = 0;
    Score = 0;
    PlayerState = PLAYERSTATE_PLAYING;
    CarDensityForCurrentZone = 0;
    RoadDensityAroundPlayer = 1.0f;
    bAfterRemoteVehicleExplosion = false;
    bCreateRemoteVehicleExplosion = false;
    bFadeAfterRemoteVehicleExplosion = false;
    m_bTryingToExitCar = false;
    bTaxiTimerScore = false;
    TaxiTimer = 0;
    vehicle_time_counter = CTimer::GetTimeInMS();
    MaxArmour = 100;
    MaxHealth = 100;
    bCanDoDriveBy = true;
    CollectablesPickedUp = 0;
    TotalNumCollectables = 3;
    LastTimeEnergyLost = 0;
    LastTimeArmourLost = 0;
    LastTimeBigGunFired = 0;
    TimesStuckInARow = 0;
    TimesUpsideDownInARow = 0;
    nCarTwoWheelCounter = 0;
    fCarTwoWheelDist = 0.0f;
    nCarLess3WheelCounter = 0;
    nBikeRearWheelCounter = 0;
    fBikeRearWheelDist = 0.0f;
    nBikeFrontWheelCounter = 0;
    fBikeFrontWheelDist = 0.0f;
    nTempBufferCounter = 0;
    nBestCarTwoWheelsTimeMs = 0;
    fBestCarTwoWheelsDistM = 0.0f;
    nBestBikeWheelieTimeMs = 0;
    fBestBikeWheelieDistM = 0.0f;
    nBestBikeStoppieTimeMs = 0;
    fBestBikeStoppieDistM = 0.0f;
    DoesNotGetTired = false;
    FastReload = false;
    FireProof = false;
    bGetOutOfJailFree = false;
    bFreeHealthCare = false;
    TimeOfLastCarExplosionCaused = 0;
    ExplosionMultiplier = 0;
    HavocCaused = 0;
    FindPlayerInfo().TimeLastEaten = 0;
    m_nLastBustMessageNumber = 1;
    CurrentChaseValue = 0.0f;
    m_nBustedAudioStatus = 0;
    m_nCrosshairActivated = 0;

    m_nRequireParachuteTimer = 0;

    if (m_bParachuteReferenced) {
        CGameLogic::IsCoopGameGoingOn();
        if (m_bParachuteReferenced) {
            CStreaming::SetModelIsDeletable(MODEL_GUN_PARA);
            m_bParachuteReferenced = false;
            m_nRequireParachuteTimer = 0;
        }
    }
}

// 0x56EC40
void CPlayerInfo::GivePlayerParachute() const {
    if (m_nRequireParachuteTimer) {
        if (CStreaming::IsModelLoaded(MODEL_GUN_PARA)) {
            pPed->GiveWeapon(WEAPON_PARACHUTE, 1, true);
            pPed->SetSavedWeapon(WEAPON_PARACHUTE);
        }
    }
}

// 0x56EB30
void CPlayerInfo::StreamParachuteWeapon(bool unk) {
    if (CGameLogic::IsCoopGameGoingOn()) {
        if (unk) {
            return;
        }
        if (m_bParachuteReferenced) {
            CStreaming::SetModelIsDeletable(MODEL_GUN_PARA);
            m_bParachuteReferenced = false;
            m_nRequireParachuteTimer = 0;
        }
        return;
    }

    if (pPed && pPed->IsInVehicle()) {
        if (pPed->m_pMyVehicle->IsSubPlane() || pPed->m_pMyVehicle->IsSubHeli()) {
            if (m_nRequireParachuteTimer <= (uint32)CTimer::GetTimeStepInMS()) {
                const auto groundHeight = TheCamera.CalculateGroundHeight(eGroundHeightType::ENTITY_BB_BOTTOM);
                const auto vehToGroundZDist = pPed->m_pMyVehicle->GetPosition().z - groundHeight;
                m_nRequireParachuteTimer = (vehToGroundZDist <= 50.f) ? 0 : 5000;
            } else {
                m_nRequireParachuteTimer -= (uint32)CTimer::GetTimeStepInMS();
            }
        }
    }

    if (m_nRequireParachuteTimer) {
        CStreaming::RequestModel(MODEL_GUN_PARA, STREAMING_MISSION_REQUIRED);
        m_bParachuteReferenced = true;
        return;
    }

    if (m_bParachuteReferenced) {
        CStreaming::SetModelIsDeletable(MODEL_GUN_PARA);
        m_bParachuteReferenced = false;
        m_nRequireParachuteTimer = 0;
    }
}

// 0x56EAB0
void CPlayerInfo::AddHealth(int32 amount) const {
    const auto newValue = std::min((float)MaxHealth, pPed->m_fHealth + (float)amount); // Clamp to m_nMaxHealth
    pPed->m_fHealth = std::max(newValue, pPed->m_fHealth); // Don't change health to a lower value
}

// 0x56EA30
void CPlayerInfo::BlowUpRCBuggy(bool bExplode) const {
    if (pRemoteVehicle && !pRemoteVehicle->m_bRemoveFromWorld) {
        CRemote::TakeRemoteControlledCarFromPlayer(bExplode);
        if (bExplode)
            pRemoteVehicle->BlowUpCar(FindPlayerPed(), false);
    }
}

// 0x56E870
void CPlayerInfo::MakePlayerSafe(bool enable, float radius) {
    // Not quite SA, but this is the way to do it (instead of copy pasting it twice)
    auto& flags = pPed->m_nPhysicalFlags;
    flags.bOnlyDamagedByPlayer = enable;
    flags.bNotDamagedByBullets = enable;
    flags.bNotDamagedByFlames = enable;
    flags.bIgnoresExplosions = enable;
    flags.bNotDamagedByCollisions = enable;
    flags.bNotDamagedByMelee = enable;
    PlayerPedData.m_bCanBeDamaged = !enable;
    PlayerPedData.m_pWanted->m_bEverybodyBackOff = enable;
    pPed->GetPadFromPlayer()->bPlayerSafe = enable;
    CWorld::SetAllCarsCanBeDamaged(!enable);

    if (enable) {
        CWorld::StopAllLawEnforcersInTheirTracks();
        CPad::StopPadsShaking();

        pPed->ClearAdrenaline();
        PlayerPedData.m_fTimeCanRun = std::max(PlayerPedData.m_fTimeCanRun, 0.f);
        pPed->GetIntelligence()->ClearTasks(true, false);

        gFireManager.ExtinguishPoint(GetPos(), radius);
        CExplosion::RemoveAllExplosionsInArea(GetPos(), 4000.f);
        CProjectileInfo::RemoveAllProjectiles();
        CWorld::ExtinguishAllCarFiresInArea(GetPos(), radius);
        CReplay::DisableReplays();
        pPed->ClearWeaponTarget();
    } else {
        CReplay::EnableReplays();
    }
}

// 0x56E830
void CPlayerInfo::PlayerFailedCriticalMission() {
    if (PlayerState == PLAYERSTATE_PLAYING) {
        PlayerState = PLAYERSTATE_FAILED_MISSION;
        CGameLogic::SetMissionFailed();
        CDarkel::ResetOnPlayerDeath();
    }
}

// 0x56E610
void CPlayerInfo::WorkOutEnergyFromHunger() {

    static auto& s_lastTimeHungryStateProcessedInitialized = StaticRef<bool>(0xB9B8F4); // false
    static auto& s_lastTimeHungryStateProcessed = StaticRef<uint8>(0xB9B8F2);
    static auto& s_LastHungryState = StaticRef<int8>(0xB9B8F1);
    static auto& s_bHungryMessageShown = StaticRef<bool>(0xB9B8F0);

    if (CCheat::IsActive(NOFOOD_CHEAT)) {
        return;
    }

    if (!s_lastTimeHungryStateProcessedInitialized) {
        s_lastTimeHungryStateProcessedInitialized = true;
        s_lastTimeHungryStateProcessed = CClock::GetGameClockHours();
    }

    auto pad = CPad::GetPad();
    if (   pad->ArePlayerControlsDisabled()
        || CMenuSystem::num_menus_in_use
        || TheCamera.m_bWideScreenOn
        || CCutsceneMgr::ms_running
        || CGameLogic::IsCoopGameGoingOn()
        || pRemoteVehicle
    ) {
        return;
    }

    if (!pPed)
        return;

    if (pPed->m_pAttachedTo)
        return;

    if (CClock::GetGameClockHours() != s_lastTimeHungryStateProcessed) {
        if (!TimeLastEaten)
            s_LastHungryState = 0;
        TimeLastEaten += 1;
    }

    if (TimeLastEaten <= 48) {
        s_bHungryMessageShown = false;
    } else {
        if (CClock::GetGameClockHours() == s_lastTimeHungryStateProcessed)
            return;

        pPed->Say(CTX_GLOBAL_STOMACH_RUMBLE);
        pad->StartShake(400, 110u, 0);

        if (s_bHungryMessageShown) {
            bool bDecreaseHealth{};
            if (CStats::GetStatValue(STAT_FAT) > 0.0f) {
                CStats::DecrementStat(STAT_FAT, 25.0f);
                CStats::DisplayScriptStatUpdateMessage(STAT_UPDATE_DECREASE, STAT_FAT, 25.0f);
                bDecreaseHealth = true;
                if (!s_LastHungryState) {
                    s_LastHungryState = TimeLastEaten + 24;
                }
            }

            if (CStats::GetStatValue(STAT_MUSCLE) <= 0.0f || TimeLastEaten <= s_LastHungryState && s_LastHungryState) {
                if (!bDecreaseHealth) {
                    pPed->m_fHealth -= 2.0f;
                }
            } else {
                CStats::DecrementStat(STAT_MUSCLE, 25.0);
                CStats::DisplayScriptStatUpdateMessage(STAT_UPDATE_DECREASE, STAT_MUSCLE, 25.0f);
            }
        } else {
            CHud::SetHelpMessage(TheText.Get("NOTEAT"), true, false, true);
            s_bHungryMessageShown = true;
        }
    }

    if (CClock::GetGameClockHours() != s_lastTimeHungryStateProcessed) {
        s_lastTimeHungryStateProcessed = CClock::GetGameClockHours();
    }
}

// 0x56E5D0
void CPlayerInfo::ArrestPlayer() {
    if (PlayerState == PLAYERSTATE_PLAYING) {
        PlayerState = PLAYERSTATE_HAS_BEEN_ARRESTED;
        m_nBustedAudioStatus = 0;
        CDarkel::ResetOnPlayerDeath();
        CStats::IncrementStat(STAT_TIMES_BUSTED, 1.0f);
        CGangWars::EndGangWar(false);
    }
}

// 0x56E580
void CPlayerInfo::KillPlayer() {
    if (PlayerState == PLAYERSTATE_PLAYING) {
        PlayerState = PLAYERSTATE_HAS_DIED;
        CDarkel::ResetOnPlayerDeath();
        CMessages::AddBigMessage(TheText.Get("DEAD"), 4000, STYLE_WHITE_MIDDLE);
        CStats::IncrementStat(STAT_NUMBER_OF_HOSPITAL_VISITS, 1.0f);
        CGangWars::EndGangWar(false);
    }
}

// 0x56E570
bool CPlayerInfo::IsRestartingAfterMissionFailed() const {
    return PlayerState == PLAYERSTATE_FAILED_MISSION;
}

// 0x56E560
bool CPlayerInfo::IsRestartingAfterArrest() const {
    return PlayerState == PLAYERSTATE_HAS_BEEN_ARRESTED;
}

// 0x56E550
bool CPlayerInfo::IsRestartingAfterDeath() const {
    return PlayerState == PLAYERSTATE_HAS_DIED;
}

// 0x56DAB0
bool CPlayerInfo::IsPlayerInRemoteMode() const {
    return pRemoteVehicle || bAfterRemoteVehicleExplosion;
}

// 0x56DFB0
// Return occupied vehicle's (if in any) or player's ped position
CVector CPlayerInfo::GetPos() const {
    return pPed->IsInVehicle() ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
}

// 0x56DF50
// Return occupied vehicle's (if in any) or player's ped move speed
CVector CPlayerInfo::GetSpeed() const {
    return pPed->IsInVehicle() ? pPed->m_pMyVehicle->GetMoveSpeed() : pPed->GetMoveSpeed();
}

// 0x5D3B00
bool CPlayerInfo::Load() {
    CGenericGameStorage::LoadDataFromWorkBuffer<int32>(); // Discarded
    auto data = CGenericGameStorage::LoadDataFromWorkBuffer<CPlayerInfoSaveStructure>();
    data.Extract(this);
    return true;
}

// 0x5D3AC0
bool CPlayerInfo::Save() {
    CPlayerInfoSaveStructure data;
    data.Construct(this);
    CGenericGameStorage::SaveDataToWorkBuffer(sizeof(CPlayerInfoSaveStructure));
    CGenericGameStorage::SaveDataToWorkBuffer(data);
    return true;
}

// 0x45DEF0
CPlayerInfo& CPlayerInfo::operator=(const CPlayerInfo& rhs) {
    pPed                             = rhs.pPed;
    PlayerPedData                       = rhs.PlayerPedData;
    pRemoteVehicle                   = rhs.pRemoteVehicle;
    pSpecCar                         = rhs.pSpecCar;
    Score                           = rhs.Score;
    DisplayScore                    = rhs.DisplayScore;
    CollectablesPickedUp            = rhs.CollectablesPickedUp;
    TotalNumCollectables            = rhs.TotalNumCollectables;
    nLastBumpPlayerCarTimer          = rhs.nLastBumpPlayerCarTimer;
    TaxiTimer                       = rhs.TaxiTimer;
    vehicle_time_counter              = rhs.vehicle_time_counter;
    bTaxiTimerScore                  = rhs.bTaxiTimerScore;
    m_bTryingToExitCar                 = rhs.m_bTryingToExitCar;
    pLastTargetVehicle               = rhs.pLastTargetVehicle;
    PlayerState                     = rhs.PlayerState;
    bAfterRemoteVehicleExplosion     = rhs.bAfterRemoteVehicleExplosion;
    bCreateRemoteVehicleExplosion    = rhs.bCreateRemoteVehicleExplosion;
    bFadeAfterRemoteVehicleExplosion = rhs.bFadeAfterRemoteVehicleExplosion;
    TimeOfRemoteVehicleExplosion    = rhs.TimeOfRemoteVehicleExplosion;
    LastTimeEnergyLost              = rhs.LastTimeEnergyLost;
    LastTimeArmourLost              = rhs.LastTimeArmourLost;
    LastTimeBigGunFired             = rhs.LastTimeBigGunFired;
    TimesUpsideDownInARow           = rhs.TimesUpsideDownInARow;
    TimesStuckInARow                = rhs.TimesStuckInARow;
    nCarTwoWheelCounter              = rhs.nCarTwoWheelCounter;
    fCarTwoWheelDist                 = rhs.fCarTwoWheelDist;
    nCarLess3WheelCounter            = rhs.nCarLess3WheelCounter;
    nBikeRearWheelCounter            = rhs.nBikeRearWheelCounter;
    fBikeRearWheelDist               = rhs.fBikeRearWheelDist;
    nBikeFrontWheelCounter           = rhs.nBikeFrontWheelCounter;
    fBikeFrontWheelDist              = rhs.fBikeFrontWheelDist;
    nTempBufferCounter               = rhs.nTempBufferCounter;
    nBestCarTwoWheelsTimeMs          = rhs.nBestCarTwoWheelsTimeMs;
    fBestCarTwoWheelsDistM           = rhs.fBestCarTwoWheelsDistM;
    nBestBikeWheelieTimeMs           = rhs.nBestBikeWheelieTimeMs;
    fBestBikeWheelieDistM            = rhs.fBestBikeWheelieDistM;
    nBestBikeStoppieTimeMs           = rhs.nBestBikeStoppieTimeMs;
    fBestBikeStoppieDistM            = rhs.fBestBikeStoppieDistM;
    CarDensityForCurrentZone        = rhs.CarDensityForCurrentZone;
    RoadDensityAroundPlayer         = rhs.RoadDensityAroundPlayer;
    TimeOfLastCarExplosionCaused    = rhs.TimeOfLastCarExplosionCaused;
    ExplosionMultiplier             = rhs.ExplosionMultiplier;
    HavocCaused                     = rhs.HavocCaused;
    TimeLastEaten                = rhs.TimeLastEaten;
    CurrentChaseValue               = rhs.CurrentChaseValue;
    DoesNotGetTired                 = rhs.DoesNotGetTired;
    FastReload                      = rhs.FastReload;
    FireProof                       = rhs.FireProof;
    MaxHealth                       = rhs.MaxHealth;
    MaxArmour                       = rhs.MaxArmour;
    bGetOutOfJailFree                = rhs.bGetOutOfJailFree;
    bFreeHealthCare                  = rhs.bFreeHealthCare;
    bCanDoDriveBy                    = rhs.bCanDoDriveBy;
    m_nBustedAudioStatus               = rhs.m_nBustedAudioStatus;
    m_nLastBustMessageNumber           = rhs.m_nLastBustMessageNumber;
    m_nCrosshairActivated              = rhs.m_nCrosshairActivated;
    m_vecCrosshairTarget               = rhs.m_vecCrosshairTarget;
    m_pSkinTexture                     = rhs.m_pSkinTexture;
    m_bParachuteReferenced             = rhs.m_bParachuteReferenced;
    m_nRequireParachuteTimer           = rhs.m_nRequireParachuteTimer;
    strcpy_s(m_skinName, rhs.m_skinName);
    return *this;
}
