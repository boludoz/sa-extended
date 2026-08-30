/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "eCheats.h"
#include "eModelID.h"

class CVehicle;

class CCheat {
public:
    static constexpr auto CHEAT_STRING_SIZE = 30;
    static constexpr auto CHEAT_MIN_HASH_SIZE = 6;

    //! How many cheats the game itself knows about. `TOTAL_CHEATS` counts the NOTSA entries
    //! appended to `eCheats` as well, and the three arrays below are sized by the *game*:
    //! the search loop at 0x438510 stops at 0x5C, and `ResetCheats` clears 0x17 dwords (= 92
    //! bytes) at 0x438453. Sizing them by `TOTAL_CHEATS` overruns - `m_aCheatFunctions` ends
    //! exactly where `m_aCheatHashKeys` begins, so the two would overlap.
    static constexpr auto NUM_GAME_CHEATS = 92;
    static_assert(NUM_GAME_CHEATS <= TOTAL_CHEATS);

#ifdef USE_ADDITIONAL_CHEATS
    static constexpr auto NUM_NOTSA_CHEATS = (CHEAT_REAL_INVINCIBILITY - TOTAL_CHEATS);
#endif

    // var: sa 0x96918C
    static bool m_bHasPlayerCheated;

    // var: sa 0x969130
    static bool m_aCheatsActive[NUM_GAME_CHEATS];

    // var: sa 0x969110
    static char m_CheatString[CHEAT_STRING_SIZE];

    // var: sa 0x8A5B58
    static void (*m_aCheatFunctions[NUM_GAME_CHEATS])();

    // var: sa 0x8A5CC8
    static uint32 m_aCheatHashKeys[NUM_GAME_CHEATS];

#ifdef USE_ADDITIONAL_CHEATS
    static int32 m_nLastScriptBypasstime;
    static bool m_bShowMappings;
    static inline bool m_aNotsaCheatsActive[NUM_NOTSA_CHEATS]{};
#endif

public:
    static void InjectHooks();

    static void AddToCheatString(char LastPressedKey);
    static void ResetCheats();
    static void DoCheats();
    static void ApplyCheat(eCheats cheat);

    static void AdrenalineCheat();
    static void AllCarsAreGreatCheat();
    static void AllCarsAreShitCheat();
    static void BeachPartyCheat();
    static void BlackCarsCheat();
    static void BlowUpCarsCheat();
    static void CountrysideInvasionCheat();
    static void DrivebyCheat();
    static void DuskCheat();
    static void ElvisLivesCheat();
    static void EverybodyAttacksPlayerCheat();
    static void FastTimeCheat();
    static void SlowTimeCheat();
    static void FatCheat();
    static void FunhouseCheat();
    static void GangLandCheat();
    static void GangsCheat();
    static void HandleSpecialCheats(eCheats cheat);
    static void HealthCheat();
    static void JetpackCheat();
    static void LoveConquersAllCheat();
    static void MayhemCheat();
    static void MidnightCheat();
    static void MoneyArmourHealthCheat();
    static void MuscleCheat();
    static void NinjaCheat();
    static void NotWantedCheat();
    static void ParachuteCheat();
    static void PinkCarsCheat();
    static void RiotCheat();
    static void SandstormCheat();
    static void SkinnyCheat();
    static void StaminaCheat();
    static void StormCheat();
    static void SuicideCheat();

    static void TankerCheat();
    static CVehicle* VehicleCheat(eModelID vehicleModelId);
    static void ApacheCheat();
    static void DozerCheat();
    static void FlyboyCheat();
    static void GolfcartCheat();
    static void HearseCheat();
    static void LovefistCheat();
    static void MonsterTruckCheat();
    static void PredatorCheat();
    static void QuadCheat();
    static void StockCarCheat();
    static void StockCar2Cheat();
    static void StockCar3Cheat();
    static void StockCar4Cheat();
    static void StuntPlaneCheat();
    static void TankCheat();
    static void TrashmasterCheat();
    static void VortexCheat();
    static void VehicleSkillsCheat();

    static void VillagePeopleCheat();
    static void WantedCheat();
    static void WantedLevelDownCheat();
    static void WantedLevelUpCheat();

    static void WeaponCheat1();
    static void WeaponCheat2();
    static void WeaponCheat3();
    static void WeaponSkillsCheat();

    static void CloudyWeatherCheat();
    static void ExtraSunnyWeatherCheat();
    static void FoggyWeatherCheat();
    static void RainyWeatherCheat();
    static void SunnyWeatherCheat();
    static bool IsZoneStreamingAllowed();

#ifdef USE_ADDITIONAL_CHEATS
    static void WeaponCheat4();
    static void TimeTravelCheat();
    static void TheGamblerCheat();
    static void BigHeadCheat();
    static void ThinBodyCheat();
    static void ScriptBypassCheat();
    static void ShowMappingsCheat();
    static void TogglePlayerInvincibility();
    static void ToggleShowTapToTarget();
    static void ToggleShowTargeting();

    static void WeaponSlotCheat();

    static void ProcessAllCheats();
    static void ProcessDebugCarCheats();
    static void ProcessDebugMissionSkip();
    static void ProcessCheats();
    static void ProcessCheatMenu();
    static void ProcessWeaponSlotCheats();
#endif

    static bool IsActive(eCheats cheat) {
        if (cheat < NUM_GAME_CHEATS) {
            return m_aCheatsActive[cheat];
        }
#ifdef USE_ADDITIONAL_CHEATS
        if (cheat > TOTAL_CHEATS && cheat <= CHEAT_REAL_INVINCIBILITY) {
            return m_aNotsaCheatsActive[cheat - (TOTAL_CHEATS + 1)];
        }
#endif
        return false;
    }
    static bool IsAnyActive(std::initializer_list<eCheats> cheats) { return rng::any_of(cheats, IsActive); }
    static void Toggle(eCheats cheat) {
        if (cheat < NUM_GAME_CHEATS) {
            m_aCheatsActive[cheat] ^= true;
#ifdef USE_ADDITIONAL_CHEATS
        } else if (cheat > TOTAL_CHEATS && cheat <= CHEAT_REAL_INVINCIBILITY) {
            m_aNotsaCheatsActive[cheat - (TOTAL_CHEATS + 1)] ^= true;
#endif
        }
    }
    static void Enable(eCheats cheat) {
        if (cheat < NUM_GAME_CHEATS) {
            m_aCheatsActive[cheat] = true;
#ifdef USE_ADDITIONAL_CHEATS
        } else if (cheat > TOTAL_CHEATS && cheat <= CHEAT_REAL_INVINCIBILITY) {
            m_aNotsaCheatsActive[cheat - (TOTAL_CHEATS + 1)] = true;
#endif
        }
    }
    static void Disable(eCheats cheat) {
        if (cheat < NUM_GAME_CHEATS) {
            m_aCheatsActive[cheat] = false;
#ifdef USE_ADDITIONAL_CHEATS
        } else if (cheat > TOTAL_CHEATS && cheat <= CHEAT_REAL_INVINCIBILITY) {
            m_aNotsaCheatsActive[cheat - (TOTAL_CHEATS + 1)] = false;
#endif
        }
    }
};
