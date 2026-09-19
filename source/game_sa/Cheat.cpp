#include "StdInc.h"

#include "Cheat.h"
#include "PedType.h"
#include "PedClothesDesc.h"

#include "Events/EventAcquaintancePedHate.h"
#include "TaskSimpleJetPack.h"
#include "PostEffects.h"
#include "Hud.h"
#include <CarGenerator.h>

/*
 * Interesting links:
 *
 * https://youtube.com/watch?v=W_eFZ4HzU7Q GTA SA - Alternative Cheats - Feat. Badger Goodger
 * https://youtube.com/watch?v=MVpMTw0rWoc GTA SA - New Secret Cheats - Feat. Spoofer
 * https://youtube.com/watch?v=L97xXbFnFWM GTA SA - Bugs that break your save game - Feat. BadgerGoodger
 *
 */


// var: sa 0x96918C
bool CCheat::m_bHasPlayerCheated;

#ifdef USE_ADDITIONAL_CHEATS
int32 CCheat::m_nLastScriptBypasstime;
bool CCheat::m_bShowMappings;
#endif

// var: sa 0x969130
bool CCheat::m_aCheatsActive[NUM_GAME_CHEATS];

// var: sa 0x969110
char CCheat::m_CheatString[30];

// var: sa 0x8A5B58
void (*CCheat::m_aCheatFunctions[NUM_GAME_CHEATS])() = {
    CCheat::WeaponCheat1,
    CCheat::WeaponCheat2,
    CCheat::WeaponCheat3,
    CCheat::MoneyArmourHealthCheat,
    CCheat::WantedLevelUpCheat,
    CCheat::WantedLevelDownCheat,
    CCheat::SunnyWeatherCheat,
    CCheat::ExtraSunnyWeatherCheat,
    CCheat::CloudyWeatherCheat,
    CCheat::RainyWeatherCheat,
    CCheat::FoggyWeatherCheat,
    nullptr, // FASTER_CLOCK
    CCheat::FastTimeCheat,
    CCheat::SlowTimeCheat,
    CCheat::MayhemCheat,
    CCheat::EverybodyAttacksPlayerCheat,
    nullptr, // EVERYONE_ARMED
    CCheat::TankCheat,
    CCheat::StockCarCheat,
    CCheat::StockCar2Cheat,
    CCheat::StockCar3Cheat,
    CCheat::StockCar4Cheat,
    CCheat::HearseCheat,
    CCheat::LovefistCheat,
    CCheat::TrashmasterCheat,
    CCheat::GolfcartCheat,
    CCheat::BlowUpCarsCheat,
    nullptr, // INVISIBLE_CAR
    nullptr, // PERFECT_HANDLING
    CCheat::SuicideCheat,
    nullptr, // BGREEN_LIGHTS
    nullptr, // AGGRESSIVE_DRIVERS
    CCheat::PinkCarsCheat,
    CCheat::BlackCarsCheat,
    nullptr, // CARS_ON_WATER
    nullptr, // BOATS_FLY
    CCheat::FatCheat,
    CCheat::MuscleCheat,
    CCheat::SkinnyCheat,
    CCheat::ElvisLivesCheat,
    CCheat::VillagePeopleCheat,
    CCheat::BeachPartyCheat,
    CCheat::GangsCheat,
    CCheat::GangLandCheat,
    CCheat::NinjaCheat,
    CCheat::LoveConquersAllCheat,
    CCheat::AllCarsAreShitCheat,
    CCheat::AllCarsAreGreatCheat,
    nullptr, // CARS_FLY
    nullptr, // HUGE_BUNNY_HOP
    CCheat::FlyboyCheat,
    CCheat::VortexCheat,
    nullptr, // SMASH_N_BOOM
    nullptr, // ALL_CARS_HAVE_NITRO
    nullptr, // CARS_FLOAT_AWAY_WHEN_HIT
    CCheat::MidnightCheat,
    CCheat::DuskCheat,
    CCheat::StormCheat,
    CCheat::SandstormCheat,
    CCheat::PredatorCheat,
    nullptr, // MEGAJUMP
    nullptr, // INFINITE_HEALTH
    nullptr, // INFINITE_OXYGEN
    CCheat::ParachuteCheat,
    CCheat::JetpackCheat,
    CCheat::NotWantedCheat,
    CCheat::WantedCheat,
    nullptr, // MEGA_PUNCH
    nullptr, // NEVER_GET_HUNGRY
    CCheat::RiotCheat,
    CCheat::FunhouseCheat,
    CCheat::AdrenalineCheat,
    nullptr, // INFINITE_AMMO
    CCheat::DrivebyCheat,
    nullptr, // REDUCED_TRAFFIC
    CCheat::CountrysideInvasionCheat,
    nullptr, // WANNA_BE_IN_MY_GANG
    nullptr, // NO_ONE_CAN_STOP_US
    nullptr, // ROCKET_MAYHEM
    nullptr, // MAX_RESPECT
    nullptr, // MAX_SEX_APPEAL
    CCheat::StaminaCheat,
    CCheat::WeaponSkillsCheat,
    CCheat::VehicleSkillsCheat,
    CCheat::ApacheCheat,
    CCheat::QuadCheat,
    CCheat::TankerCheat,
    CCheat::DozerCheat,
    CCheat::StuntPlaneCheat,
    CCheat::MonsterTruckCheat,
    nullptr, // PROSTITUTES_PAY_YOU
    nullptr, // ALL_TAXIS_NITRO
};

// index 34 is the cars driving on water cheat (add the hash from silentpatch)
// var: sa 0x8A5CC8
uint32 CCheat::m_aCheatHashKeys[NUM_GAME_CHEATS] = {
    0xDE4B237D, 0xB22A28D1, 0x5A783FAE, 0xEECCEA2B, 0x42AF1E28, 0x555FC201, 0x2A845345, 0xE1EF01EA, 0x771B83FC,
    0x5BF12848, 0x44453A17, 0xFCFF1D08, 0xB69E8532, 0x8B828076, 0xDD6ED9E9, 0xA290FD8C, 0x3484B5A7, 0x43DB914E,
    0xDBC0DD65,
    0x00000000,  // STOCKCAR2_CHEAT
    0xD08A30FE, 0x37BF1B4E, 0xB5D40866, 0xE63B0D99, 0x675B8945, 0x4987D5EE, 0x2E8F84E8, 0x1A9AA3D6, 0xE842F3BC,
    0x0D5C6A4E, 0x74D4FCB1, 0xB01D13B8, 0x66516EBC, 0x4B137E45,
#ifdef FIX_BUGS
    0xE5FC92C3,  // BACKTOTHEFUTURE_CHEAT: "SeaRoader" hash from SilentPatch
#else
    0x00000000,  // BACKTOTHEFUTURE_CHEAT
#endif
    0x78520E33, 0x3A577325, 0xD4966D59, 0x5FD1B49D, 0xA7613F99, 0x1792D871, 0xCBC579DF, 0x4FEDCCFF, 0x44B34866,
    0x2EF877DB, 0x2781E797, 0x2BC1A045, 0xB2AFE368, 0xFA8DD45B, 0x8DED75BD, 0x1A5526BC, 0xA48A770B, 0xB07D3B32,
    0x80C1E54B, 0x5DAD0087, 0x7F80B950, 0x6C0FA650, 0xF46F2FA4, 0x70164385,
    0x00000000,  // PREDATOR_CHEAT
    0x885D0B50, 0x151BDCB3, 0xADFA640A, 0xE57F96CE, 0x040CF761, 0xE1B33EB9, 0xFEDA77F7, 0x8CA870DD, 0x9A629401,
    0xF53EF5A5, 0xF2AA0C1D, 0xF36345A8, 0x8990D5E1, 0xB7013B1B, 0xCAEC94EE, 0x31F0C3CC, 0xB3B3E72A, 0xC25CDBFF,
    0xD5CF4EFF, 0x680416B1, 0xCF5FDA18, 0xF01286E9, 0xA841CC0A, 0x31EA09CF, 0xE958788A, 0x02C83A7C, 0xE49C3ED4,
    0x171BA8CC, 0x86988DAE, 0x2BDD2FA1,
    0x00000000,  // PIMP_CHEAT
    0x00000000,  // TAXINITRO_CHEAT
};

// 0x438480
#ifdef USE_ADDITIONAL_CHEATS
struct MobileCheatEntry {
    uint32  m_nHash;
    eCheats m_nCheat;
};

static constexpr MobileCheatEntry aMobileCheats[] = {
    // Money, Health, Armor
    { 0xE1614CCB, CHEAT_REAL_INVINCIBILITY },              // "GONPXWR"
    { 0x3ADFFFA8, SCUBA_CHEAT },                 // "POOOJOX"
    { 0x12D59071, MONEYARMOURHEALTH_CHEAT },               // "PJYNQCQ"
    // Weapons
    { 0x42EDF90A, WEAPON1_CHEAT },                     // "BEFWKSBQ"
    { 0xCDE9F55B, WEAPON2_CHEAT },                     // "SHHIHJJG"
    { 0xC3A29663, WEAPON3_CHEAT },                     // "GOIZSSX"
    { 0x48642074, CHEAT_WEAPON_SET4 },                     // "BIEUHQY"
    { 0x2301C349, WEAPONSKILLS_CHEAT },    // "SDWBWHE"
    { 0xD12EF2EB, AMMO_CHEAT },                   // "NECUMZ"
    { 0x132A0F27, SUICIDE_CHEAT },                         // "SLOTSFK"
    // Vehicles
    { 0x6BCC38EB, STRONGGRIP_CHEAT },                // "DLNNHZJ"
    { 0x31EA09CF, VEHICLESKILLS_CHEAT },              // "VQIMAHA"
    { 0x1437E993, VEHICLEOFDEATH_CHEAT },                    // "BXBTUBTI"
    { 0x56B01D89, DRIVEBY_CHEAT },     // "RYSMRM"
    { 0xC979142B, DRIVEBY_CHEAT },     // "IWANTTODRIVEBY"
    { 0xF9ACF74C, DRIVEBY_CHEAT },     // "IWANTDRIVEBY"
    { 0x6E4CACD5, DRIVEBY_CHEAT },     // "DRIVEBY"
    { 0x68098ED7, TRAFFICLIGHTS_CHEAT },             // "ENQCFMA"
    { 0x9CED655E, MADDRIVERS_CHEAT },              // "IOKXTFJ"
    { 0xDED59DF1, FLYINGCARS_CHEAT },                        // "DOTBSFK"
    { 0xC9993735, FLYINGFISH_CHEAT },                       // "PTHSEOT"
    { 0xD52392AD, NITRO_CHEAT },             // "WUSDOTO"
    { 0xF01286E9, TAXINITRO_CHEAT },                 // "VKYPQCF"
    { 0xC5CE5AB5, BACKTOTHEFUTURE_CHEAT },                   // "GKPNMQ"
    { 0x765D7A6D, BLOWUPCARS_CHEAT },                // "BKFONFE"
    { 0xA81A5482, SUPERPOWERCAR_CHEAT },        // "JBVIJXA"
    { 0x65AD3F35, PINKCARS_CHEAT },                    // "GYKVYTR"
    { 0x4BC59B1F, BLACKCARS_CHEAT },                   // "GOYDVAO"
    { 0x05FB6B3C, TANK_CHEAT },                     // "AYNVQVK"
    { 0x4166529E, JETPACK_CHEAT },                     // "CDGUDEP"
    { 0x2BDD2FA1, MONSTERTRUCK_CHEAT },                   // "AGBDLCID"
    { 0x5B8EF3FE, PARACHUTE_CHEAT },                   // "GSUMLEG"
    { 0x99CF7EC2, STOCKCAR_CHEAT },           // "BIGLWCDD"
    { 0x4CE89DD5, GOLFCART_CHEAT },                     // "DAHESZY"
    { 0x171BA8CC, DOZER_CHEAT },                     // "EEGCYXT"
    { 0xD2A85D8B, STOCKCAR3_CHEAT },                  // "BGJPSYC"
    { 0x2B9B8339, STOCKCAR4_CHEAT },                  // "BIEAVBAY"
    { 0x860428E8, FLYBOY_CHEAT },                     // "AWPTMIIQ"
    { 0x9E2897FA, LOVEFIST_CHEAT },                   // "IXSMWCQ"
    { 0x6DAFE294, HEARSE_CHEAT },                    // "PSPNATX"
    { 0x02C83A7C, QUAD_CHEAT },                      // "AKJJYGLC"
    { 0x6272855B, STOCKCAR2_CHEAT },                   // "HPGPIJZ"
    { 0x86988DAE, STUNTPLANE_CHEAT },               // "URKQSRK"
    { 0xE49C3ED4, TANKER_CHEAT },              // "AMOMHRER"
    { 0x8D284EBD, TRASHMASTER_CHEAT },               // "QPOLSVK"
    { 0xA48A770B, VORTEX_CHEAT },                    // "KGGGDKP"
    // Stats
    { 0xF1BA89BC, STAMINA_CHEAT },                     // "AEZLCKXU"
    { 0x1ED903DD, NOFOOD_CHEAT },                // "KBTMUVH"
    { 0x8CCA8D6B, FAT_CHEAT },                      // "AESHXWQI"
    { 0x0D9DDD15, MUSCLE_CHEAT },                      // "SGVDSQW"
    { 0x5FD1B49D, SKINNY_CHEAT },                   // "KVGYZQK"
    { 0x4C53B1F1, RESPECT_CHEAT },                     // "MTGIISR"
    { 0xE60DEED4, SEXAPPEAL_CHEAT },                  // "APGZLQR"
    { 0xFA300554, CHEAT_THE_GAMBLER },                     // "AFJKBNRP"
    // Wanted
    { 0xD9944593, WANTEDLEVELUP_CHEAT },             // "NCBXXDX"
    { 0xAA5FE043, WANTEDLEVELDOWN_CHEAT },              // "KDTZNHON"
    { 0x7607D7B8, WANTED_CHEAT },                // "GWJZWCB"
    { 0x26079684, NOTWANTED_CHEAT },                // "BYKGOAB"
    // Gameplay, Weather, Peds
    { 0x9BC9E237, FASTWEATHER_CHEAT },                    // "OWAKIJ"
    { 0x08AC509B, FASTTIME_CHEAT },                 // "EHWBWDS"
    { 0x9701CFB4, SLOWTIME_CHEAT },                 // "FNJFCZC"
    { 0x34F5060A, ADRENALINE_CHEAT },                 // "SLSNRKKK"
    { 0xF77AE750, DUSK_CHEAT },      // "KTGDLXY"
    { 0x045A0CAD, CHEAT_TIME_TRAVEL },                     // "YACKMWS"
    { 0x7EABE709, MIDNIGHT_CHEAT },                 // "AWUJNBB"
    { 0x1CA1A783, SUNNYWEATHER_CHEAT },                   // "AAEXPPQC"
    { 0x182BBB8E, EXTRASUNNYWEATHER_CHEAT },              // "HTRTTVJ"
    { 0x90668987, CLOUDYWEATHER_CHEAT },                // "VBWEMQX"
    { 0xFAF7CFF1, RAINYWEATHER_CHEAT },                   // "TAVPIER"
    { 0x4C40B7BC, STORM_CHEAT },                   // "EAKILHM"
    { 0xD83323B0, FOGGYWEATHER_CHEAT },                   // "EGCEBVM"
    { 0x62AB7CD3, SANDSTORM_CHEAT },                      // "JBWDWWO"
    { 0x7A7365B6, WASTELAND_CHEAT },                 // "DEHDRX"
    { 0xB6BD1EF8, COUNTRYSIDEINVASION_CHEAT },                 // "JTBCSN"
    { 0x7FF6FDAD, ALLCARSAREGREAT_CHEAT },                    // "FRIUBIL"
    { 0x17E2371C, GANGS_CHEAT },          // "EAMLJNN"
    { 0x4551B5B2, RECRUITME_CHEAT },             // "JEZRPI"
    { 0xF187CB21, RECRUITME_AK47_CHEAT },              // "AWIOMPH"
    { 0x76F94A7B, RECRUITME_ROCKET_CHEAT },                   // "QAONHOH"
    { 0x60299030, GANGLAND_CHEAT },      // "HAPOHXR"
    { 0x5AE74D6E, FUNHOUSE_CHEAT },                  // "HDLIWGB"
    { 0x9321FFA4, BEACHPARTY_CHEAT },                     // "BWCMMTD"
    { 0xD0C58345, WEAPONSFORALL_CHEAT },                  // "GDNXHDK"
    { 0xBB9E7E86, EVERYBODYATTACKSPLAYER_CHEAT },       // "FARYJHZ"
    { 0x7222BA4B, RIOT_CHEAT },                       // "NJXPCYE"
    { 0x73A8AA87, MAYHEM_CHEAT }, // "AKOZBCH"
    { 0x9AD03DD3, VILLAGEPEOPLE_CHEAT },    // "ERFBFNI"
    { 0xB5DFCBC6, LOVECONQUERSALL_CHEAT },                     // "BNIZWSBT"
    { 0xEB788F38, ELVISLIVES_CHEAT },             // "LNHVOAE"
    { 0x1CB6F385, NINJA_CHEAT },                     // "AAUSQP"
    { 0x091A31D0, ALLCARSARESHIT_CHEAT },                   // "OAXCCRI"
    { 0x92A93BA7, CJPHONEHOME_CHEAT },                  // "TDBKCEH"
    { 0xFFD40169, HIGHJUMP_CHEAT },                        // "BFMANNZP"
    { 0x0A8FBE7C, SUPERPUNCH_CHEAT },                      // "LRMYOJM"
};
#endif

// 0x438480
void CCheat::AddToCheatString(char LastPressedKey) {
    if (CCutsceneMgr::ms_running) {
        return;
    }

    LastPressedKey = (char)toupper((uint8)LastPressedKey);

    char testStr[CHEAT_STRING_SIZE];

    for (int32 i = CHEAT_STRING_SIZE - 2; i >= 0; --i) {
        m_CheatString[i + 1] = m_CheatString[i];
    }
    m_CheatString[0] = LastPressedKey;
    m_CheatString[CHEAT_STRING_SIZE - 1] = '\0';

    uint32 len = (uint32)strlen(m_CheatString);
    if (len >= CHEAT_MIN_HASH_SIZE) {
        strcpy(testStr, m_CheatString);
        for (int32 i = len; i >= CHEAT_MIN_HASH_SIZE; --i) {
            uint32 hashKey = CKeyGen::GetKey(testStr);
            for (int32 j = 0; j < NUM_GAME_CHEATS; ++j) {
                if (m_aCheatHashKeys[j] == (int32)hashKey) {
                    if (m_aCheatsActive[j]) {
                        CHud::SetHelpMessage(TheText.Get("CHEAT8"), true, false, false);
                    } else {
                        CHud::SetHelpMessage(TheText.Get("CHEAT1"), true, false, false);
                    }
                    CStats::IncrementStat(STAT_TIMES_CHEATED, 1.0f);
                    m_bHasPlayerCheated = true;
                    ApplyCheat((eCheats)j);
                    m_CheatString[0] = '\0';
                    return;
                }
            }
#ifdef USE_ADDITIONAL_CHEATS
            for (const auto& mobileCheat : aMobileCheats) {
                if (mobileCheat.m_nHash == hashKey) {
                    if (IsActive(mobileCheat.m_nCheat)) {
                        CHud::SetHelpMessage(TheText.Get("CHEAT8"), true, false, false);
                    } else {
                        CHud::SetHelpMessage(TheText.Get("CHEAT1"), true, false, false);
                    }
                    CStats::IncrementStat(STAT_TIMES_CHEATED, 1.0f);
                    m_bHasPlayerCheated = true;
                    ApplyCheat(mobileCheat.m_nCheat);
                    m_CheatString[0] = '\0';
                    return;
                }
            }
            // Android leaked & debug cheats
            switch (hashKey) {
            case 0x0C735788: // "BYIXZIY" -> Script Bypass
                CHud::SetHelpMessage(TheText.Get("CHEAT1"), true, false, false);
                CStats::IncrementStat(STAT_TIMES_CHEATED, 1.0f);
                m_bHasPlayerCheated = true;
                ScriptBypassCheat();
                m_CheatString[0] = '\0';
                return;
            case 0x7FBD1F6D: // "KRRIHBT" -> Show Mappings
                CHud::SetHelpMessage(TheText.Get("CHEAT1"), true, false, false);
                CStats::IncrementStat(STAT_TIMES_CHEATED, 1.0f);
                m_bHasPlayerCheated = true;
                ShowMappingsCheat();
                m_CheatString[0] = '\0';
                return;
            case 0xF0A6B85C: // "MIZYXHZ" -> Show Tap To Target
                ToggleShowTapToTarget();
                CHud::SetHelpMessage(TheText.Get("CHEAT1"), true, false, false);
                CStats::IncrementStat(STAT_TIMES_CHEATED, 1.0f);
                m_bHasPlayerCheated = true;
                m_CheatString[0] = '\0';
                return;
            case 0xFD5AE965: // "JQFUDUB" -> Show Targeting
                ToggleShowTargeting();
                CHud::SetHelpMessage(TheText.Get("CHEAT1"), true, false, false);
                CStats::IncrementStat(STAT_TIMES_CHEATED, 1.0f);
                m_bHasPlayerCheated = true;
                m_CheatString[0] = '\0';
                return;
            }
#endif

            testStr[i - 1] = '\0';
        }
    }
}

// Activates the cheat without changing statistics
// unknown name
// 0x438370
void CCheat::ApplyCheat(eCheats cheat) {
#ifdef USE_ADDITIONAL_CHEATS
    if (cheat == CHEAT_WEAPON_SET4) {
        return WeaponCheat4();
    }
    if (cheat == CHEAT_TIME_TRAVEL) {
        return TimeTravelCheat();
    }
    if (cheat == CHEAT_THE_GAMBLER) {
        return TheGamblerCheat();
    }
    if (cheat == CHEAT_REAL_INVINCIBILITY) {
        return TogglePlayerInvincibility();
    }
#endif

    if (cheat < NUM_GAME_CHEATS) {
        auto func = m_aCheatFunctions[cheat];
        if (func) {
            return func();
        }
    }
    return Toggle(cheat);
}

// 0x438450
void CCheat::ResetCheats() {
    ZoneScoped;

    memset(&m_aCheatsActive, 0, sizeof(m_aCheatsActive));
#ifdef USE_ADDITIONAL_CHEATS
    memset(&m_aNotsaCheatsActive, 0, sizeof(m_aNotsaCheatsActive));
#endif
    CWeather::ReleaseWeather();
    CTimer::ResetTimeScale();
    m_CheatString[0] = '\0';
    m_bHasPlayerCheated = false;
}

// 0x439AF0
void CCheat::DoCheats() {
    ZoneScoped;

    for (int32 i = 0; i < 256; ++i) {
        if (CPad::NewKeyState.standardKeys[i] != 0 && CPad::OldKeyState.standardKeys[i] == 0) {
            AddToCheatString((char)i);
        }
    }
}

// 0x439880
void CCheat::AdrenalineCheat() {
    Toggle(ADRENALINE_CHEAT);
    if (IsActive(ADRENALINE_CHEAT)) {
        CPickups::GivePlayerGoodiesWithPickUpMI(ModelIndices::MI_PICKUP_ADRENALINE, 0);
    } else {
        FindPlayerPed()->ClearAdrenaline(); // FIX_BUGS
    }
}

// 0x4394e0
void CCheat::AllCarsAreGreatCheat() {
    Toggle(ALLCARSAREGREAT_CHEAT);
    if (IsActive(ALLCARSAREGREAT_CHEAT)) {
        HandleSpecialCheats(ALLCARSAREGREAT_CHEAT);
        CStreaming::ReclassifyLoadedCars();
    }
}

// 0x4394b0
void CCheat::AllCarsAreShitCheat() {
    Toggle(ALLCARSARESHIT_CHEAT);
    if (IsActive(ALLCARSARESHIT_CHEAT)) {
        HandleSpecialCheats(ALLCARSARESHIT_CHEAT);
        CStreaming::ReclassifyLoadedCars();
    }
}

// 0x439230
void CCheat::BeachPartyCheat() {
    Toggle(BEACHPARTY_CHEAT);
    if (IsActive(BEACHPARTY_CHEAT)) {
        int32 peds[] = {
            MODEL_BMYBE, MODEL_HMYBE, MODEL_WFYBE, MODEL_BFYBE,
            MODEL_HFYBE, UNLOAD_MODEL, UNLOAD_MODEL, UNLOAD_MODEL,
        };
        CStreaming::StreamPedsIntoRandomSlots(peds);

        CPlayerPed* player = FindPlayerPed();
        player->GetClothesDesc()->SetTextureAndModel(nullptr, nullptr, CLOTHES_TEXTURE_SPECIAL); // FIX_BUGS
        player->GetClothesDesc()->SetTextureAndModel("torso", nullptr, CLOTHES_TEXTURE_TORSO);
        player->GetClothesDesc()->SetTextureAndModel("shortskhaki", "shorts", CLOTHES_TEXTURE_LEGS);
        player->GetClothesDesc()->SetTextureAndModel("flipflop", "flipflop", CLOTHES_TEXTURE_SHOES);
        player->GetClothesDesc()->SetTextureAndModel("glasses04dark", "glasses04", CLOTHES_TEXTURE_GLASSES);

        if (player->m_nPedState != PEDSTATE_DRIVING) {
            CClothes::RebuildPlayer(player, false);
        }

        ExtraSunnyWeatherCheat();
        HandleSpecialCheats(BEACHPARTY_CHEAT);
        CStreaming::ReclassifyLoadedCars();
    }
}

// 0x4390f0
void CCheat::BlackCarsCheat() {
    Toggle(BLACKCARS_CHEAT);
    if (IsActive(BLACKCARS_CHEAT)) {
        Disable(PINKCARS_CHEAT);
    }
}

// 0x439d80
void CCheat::BlowUpCarsCheat() {
    for (auto& veh : GetVehiclePool()->GetAllValid()) {
        veh.BlowUpCar(nullptr, false);
    }
}

// 0x439f60
void CCheat::CountrysideInvasionCheat() {
    Toggle(COUNTRYSIDEINVASION_CHEAT);
    if (IsActive(COUNTRYSIDEINVASION_CHEAT)) {
        int32 peds[] = {
            MODEL_CWFOFR, MODEL_CWFOHB, MODEL_CWFYFR1, MODEL_CWFYHB,
            MODEL_CWMOFR, MODEL_CWMOHB1, MODEL_CWMOHB2, MODEL_CWMYFR,
        };
        CStreaming::StreamPedsIntoRandomSlots(peds);

        CPlayerPed* player = FindPlayerPed();
        player->GetClothesDesc()->SetTextureAndModel("timberfawn", "bask1", CLOTHES_TEXTURE_SHOES);
        player->GetClothesDesc()->SetTextureAndModel("captruck", "captruck", CLOTHES_TEXTURE_HATS);
        player->GetClothesDesc()->SetTextureAndModel("countrytr", "countrytr", CLOTHES_TEXTURE_SPECIAL);

        if (player->m_nPedState != PEDSTATE_DRIVING) {
            CClothes::RebuildPlayer(player, false);
        }
        if (IsActive(BEACHPARTY_CHEAT)) {
            BeachPartyCheat();
        }
        if (IsActive(ALLCARSARESHIT_CHEAT)) {
            Disable(ALLCARSARESHIT_CHEAT);
        }
        if (IsActive(ALLCARSAREGREAT_CHEAT)) {
            Disable(ALLCARSAREGREAT_CHEAT);
        }
        if (IsActive(FUNHOUSE_CHEAT)) {
            FunhouseCheat();
        }

        CStreaming::ReclassifyLoadedCars();
    }
}

// 0x4398d0
void CCheat::DrivebyCheat() {
    Toggle(DRIVEBY_CHEAT);

    CPlayerPed *player = FindPlayerPed();
    if (IsActive(DRIVEBY_CHEAT) && player->GetWeaponInSlot(eWeaponSlot::SMG).m_Type == WEAPON_UNARMED) {
        player->GiveDelayedWeapon(WEAPON_MICRO_UZI, 150);
        player->SetCurrentWeapon(WEAPON_MICRO_UZI);
    }
}

// 0x439540
void CCheat::DuskCheat() {
    Toggle(DUSK_CHEAT);
    if (!IsActive(DUSK_CHEAT)) {
        CClock::ms_nGameClockHours = 21;
        CClock::ms_nGameClockMinutes = 0;
        CClock::ms_nGameClockSeconds = 0;
    }
}

// 0x4391d0
void CCheat::ElvisLivesCheat() {
    Toggle(ELVISLIVES_CHEAT);
    if (IsActive(ELVISLIVES_CHEAT)) {
        int32 peds[] = {
            MODEL_VHMYELV, MODEL_VBMYELV, MODEL_VIMYELV, UNLOAD_MODEL,
            UNLOAD_MODEL, UNLOAD_MODEL, UNLOAD_MODEL, UNLOAD_MODEL,
        };
        CStreaming::StreamPedsIntoRandomSlots(peds);
    }
}

// 0x439c70
void CCheat::EverybodyAttacksPlayerCheat() {
    Toggle(EVERYBODYATTACKSPLAYER_CHEAT);
    if (IsActive(EVERYBODYATTACKSPLAYER_CHEAT)) {
        auto* const player = FindPlayerPed();
        for (auto& ped : GetPedPool()->GetAllValid()) {
            if (ped.IsPlayer())
                continue;

            ped.GetAcquaintance().SetAsAcquaintance(ACQUAINTANCE_HATE, CPedType::GetPedFlag(PED_TYPE_PLAYER1));

            CEventAcquaintancePedHate event(player);
            event.m_TaskId = TASK_COMPLEX_KILL_PED_ON_FOOT;
            ped.GetEventGroup().Add(&event, false);
        }
    }
}

// 0x438f90
void CCheat::FastTimeCheat() {
    if (CTimer::GetTimeScale() < 4.0f) {
        CTimer::SetTimeScale(CTimer::GetTimeScale() * 2.0f);
    }
}

// 0x438fc0
void CCheat::SlowTimeCheat() {
    if (CTimer::GetTimeScale() > 0.25f) {
        CTimer::SetTimeScale(CTimer::GetTimeScale() / 2.0f);
    }
}

// 0x439110
void CCheat::FatCheat() {
    CStats::SetStatValue(STAT_FAT, 1000.0f);

    CPlayerPed *player = FindPlayerPed();
    if (player->m_nPedState != PEDSTATE_DRIVING) {
        CClothes::RebuildPlayer(player, false);
    }
}

// 0x439720
void CCheat::FunhouseCheat() {
    Toggle(FUNHOUSE_CHEAT);
    if (IsActive(FUNHOUSE_CHEAT)) {
        CPostEffects::m_bHeatHazeFX = false;
    } else {
        int32 peds[] = {
            MODEL_WMYBELL, MODEL_WFYBURG, MODEL_WMOICE, UNLOAD_MODEL,
            UNLOAD_MODEL, UNLOAD_MODEL, UNLOAD_MODEL, UNLOAD_MODEL,
        };
        CStreaming::StreamPedsIntoRandomSlots(peds);

        CPlayerPed* player = FindPlayerPed();
        player->GetClothesDesc()->SetTextureAndModel(nullptr, nullptr, CLOTHES_TEXTURE_SPECIAL); // FIX_BUGS
        player->GetClothesDesc()->SetTextureAndModel("torso", "torso", CLOTHES_TEXTURE_TORSO);
        player->GetClothesDesc()->SetTextureAndModel("legsheart", "legs", CLOTHES_TEXTURE_LEGS);
        player->GetClothesDesc()->SetTextureAndModel("timberhike", "bask1", CLOTHES_TEXTURE_SHOES);
        player->GetClothesDesc()->SetTextureAndModel("groucho", "grouchos", CLOTHES_TEXTURE_GLASSES);
        player->GetClothesDesc()->SetTextureAndModel("hairpink", "head", CLOTHES_TEXTURE_HEAD);

        if (player->m_nPedState != PEDSTATE_DRIVING) {
            CClothes::RebuildPlayer(player, false);
        }

        HandleSpecialCheats(FUNHOUSE_CHEAT);
        CStreaming::ReclassifyLoadedCars();
        ExtraSunnyWeatherCheat();
        CPostEffects::m_bHeatHazeFX = true;
    }
}

// 0x4393d0
void CCheat::GangLandCheat() {
    Toggle(GANGLAND_CHEAT);
    CPopulation::m_bOnlyCreateRandomGangMembers = IsActive(GANGLAND_CHEAT);
}

// 0x439360
void CCheat::GangsCheat() {
    Toggle(GANGS_CHEAT);
    if (IsActive(GANGS_CHEAT)) {
        int32 peds[] = {
            MODEL_BALLAS1, MODEL_BALLAS2, MODEL_BALLAS3, MODEL_FAM1,
            MODEL_FAM2, MODEL_LSV1, MODEL_LSV2, MODEL_LSV3,
        };
        CStreaming::StreamPedsIntoRandomSlots(peds);
    }
}

// Handles BeachParty, Funhouse, AllCarsAreGreat, AllCarsAreCheap cheats toggling
// 0x439A10
void CCheat::HandleSpecialCheats(eCheats cheat) {
    if (IsActive(BEACHPARTY_CHEAT) && cheat != BEACHPARTY_CHEAT) {
        BeachPartyCheat();
    }
    if (IsActive(ALLCARSARESHIT_CHEAT) && cheat != ALLCARSARESHIT_CHEAT) {
        Disable(ALLCARSARESHIT_CHEAT);
    }
    if (IsActive(ALLCARSAREGREAT_CHEAT) && cheat != ALLCARSAREGREAT_CHEAT) {
        Disable(ALLCARSAREGREAT_CHEAT);
    }
    if (IsActive(FUNHOUSE_CHEAT) && cheat != FUNHOUSE_CHEAT) {
        FunhouseCheat();
    }
    if (IsActive(COUNTRYSIDEINVASION_CHEAT) && cheat != COUNTRYSIDEINVASION_CHEAT) {
        CountrysideInvasionCheat();
    }
}

// 0x438D60
void CCheat::HealthCheat() {
    CPlayerPed* player = FindPlayerPed();
    player->m_fHealth = player->GetPlayerInfoForThisPlayerPed()->m_nMaxHealth;

    CVehicle* vehicle = FindPlayerVehicle();
    if (!vehicle) {
        return;
    }

    vehicle->m_fHealth = 1000.0f;
    if (vehicle->IsBike()) {
        vehicle->AsBike()->m_BlowUpTimer = 0.0f;
        vehicle->AsBike()->Fix();
    } else if (vehicle->IsAutomobile()) {
        vehicle->AsAutomobile()->m_fBurnTimer = 0.0f;
        vehicle->AsAutomobile()->Fix();
    }
}

// 0x439600
void CCheat::JetpackCheat() {
    auto player = FindPlayerPed();
    CTaskSimpleJetPack* task = player->GetIntelligence()->GetTaskJetPack();
    if (!task) {
        auto jetpackTask = new CTaskSimpleJetPack(nullptr, 10, 0, nullptr);
        CEventScriptCommand event(TASK_PRIMARY_PRIMARY, jetpackTask, false);
        player->GetEventGroup().Add(&event, false);
    }
}

// 0x4393f0
void CCheat::LoveConquersAllCheat() {
    Toggle(LOVECONQUERSALL_CHEAT);
    if (IsActive(LOVECONQUERSALL_CHEAT)) {
        int32 peds[] = {
            MODEL_BMYPIMP, MODEL_BFYPRO, MODEL_HFYPRO, MODEL_SWFOPRO,
            MODEL_SBFYPRO, MODEL_VWFYPRO, MODEL_VHFYPRO, UNLOAD_MODEL,
        };
        CStreaming::StreamPedsIntoRandomSlots(peds);

        CPlayerPed* player = FindPlayerPed();
        player->GetClothesDesc()->SetTextureAndModel("gimpleg", "gimpleg", CLOTHES_TEXTURE_SPECIAL);
        if (player->m_nPedState != PEDSTATE_DRIVING) {
            CClothes::RebuildPlayer(player, false);
        }
    }
}

// 0x439B20
void CCheat::MayhemCheat() {
    Toggle(MAYHEM_CHEAT);
    if (IsActive(MAYHEM_CHEAT)) {
        std::swap(CPedType::ms_apPedTypesOld, CPedType::ms_apPedTypes); // NOTSA

        for (uint32 pedType = PED_TYPE_CIVMALE; pedType <= PED_TYPE_PROSTITUTE; pedType++) {
            CPedType::SetPedTypeAsAcquaintance(ACQUAINTANCE_HATE, static_cast<ePedType>(pedType), 0xFFFFF);
        }

        for (auto& ped : GetPedPool()->GetAllValid()) {
            if (ped.IsPlayer())
                continue;

            for (uint32 pedType_1 = PED_TYPE_CIVMALE; pedType_1 <= PED_TYPE_PROSTITUTE; ++pedType_1) {
                ped.GetAcquaintance().SetAsAcquaintance(ACQUAINTANCE_HATE, CPedType::GetPedFlag(static_cast<ePedType>(pedType_1)));
            }
            if (CPed* closestPed = ped.GetIntelligence()->GetPedScanner().GetClosestPedInRange()) {
                CEventAcquaintancePedHate event(closestPed);
                event.m_TaskId = TASK_COMPLEX_KILL_PED_ON_FOOT;
                ped.GetEventGroup().Add(&event, false);
            }
        }
    } else {
        // FIX_BUGS https://youtu.be/L97xXbFnFWM?t=90
        // todo: currently fixes only save game
        std::swap(CPedType::ms_apPedTypes, CPedType::ms_apPedTypesOld); // straightforward solution

        for (auto& ped : GetPedPool()->GetAllValid()) {
            if (ped.IsPlayer())
                continue;

            if (CPed* closestPed = ped.GetIntelligence()->GetPedScanner().GetClosestPedInRange()) {
                CEventAcquaintancePedHate event(closestPed);
                event.m_TaskId = TASK_NONE;
                ped.GetEventGroup().Remove(&event);
            }
        }
    }
}

// 0x439510
void CCheat::MidnightCheat() {
    Toggle(MIDNIGHT_CHEAT);
    if (IsActive(MIDNIGHT_CHEAT)) {
        CClock::ms_nGameClockHours = 0;
        CClock::ms_nGameClockMinutes = 0;
        CClock::ms_nGameClockSeconds = 0;
    }
}

// 0x438e40
void CCheat::MoneyArmourHealthCheat() {
    CPlayerPed* player = FindPlayerPed();
    CPlayerInfo* playerInfo = player->GetPlayerInfoForThisPlayerPed();

    playerInfo->m_nMoney += 250000;
    player->m_fArmour = playerInfo->m_nMaxArmour;
    HealthCheat();
}

// 0x439150
void CCheat::MuscleCheat() {
    CPlayerPed* player = FindPlayerPed();

    CStats::SetStatValue(STAT_MUSCLE, 1000.0f);
    if (player->m_nPedState != PEDSTATE_DRIVING) {
        CClothes::RebuildPlayer(player, false);
    }
}

// 0x439e50
void CCheat::NinjaCheat() {
    Toggle(NINJA_CHEAT);
    if (IsActive(NINJA_CHEAT)) {
        if (IsActive(BLACKCARS_CHEAT)) {
            Disable(BLACKCARS_CHEAT);
        }
    } else {
        int32 peds[] = {
            MODEL_TRIADA, MODEL_TRIADB, MODEL_TRIBOSS, UNLOAD_MODEL,
            UNLOAD_MODEL, UNLOAD_MODEL, UNLOAD_MODEL, UNLOAD_MODEL,
        };
        CStreaming::StreamPedsIntoRandomSlots(peds);

        if (IsActive(BEACHPARTY_CHEAT)) {
            BeachPartyCheat();
        }
        if (IsActive(ALLCARSARESHIT_CHEAT)) {
            Disable(ALLCARSARESHIT_CHEAT);
        }
        if (IsActive(ALLCARSAREGREAT_CHEAT)) {
            Disable(ALLCARSAREGREAT_CHEAT);
        }
        if (IsActive(FUNHOUSE_CHEAT)) {
            FunhouseCheat();
        }
        if (IsActive(COUNTRYSIDEINVASION_CHEAT)) {
            CountrysideInvasionCheat();
        }
        CStreaming::ReclassifyLoadedCars();
        if (!IsActive(BLACKCARS_CHEAT)) {
            Enable(BLACKCARS_CHEAT);
            Disable(PINKCARS_CHEAT);
        }

        CStreaming::RequestModel(MODEL_KATANA, STREAMING_GAME_REQUIRED);
        CStreaming::LoadAllRequestedModels(false);

        CPlayerPed* player = FindPlayerPed();
        player->GiveWeapon(WEAPON_KATANA, 0, true);
        player->SetCurrentWeapon(WEAPON_KATANA);
    }
}

// 0x4396c0
void CCheat::NotWantedCheat() {
    CPlayerPed* player = FindPlayerPed();
    player->CheatWantedLevel(eWantedLevel::WANTED_CLEAN);
    player->bWantedByPolice = false;
    Toggle(NOTWANTED_CHEAT);
}

// 0x4395b0
void CCheat::ParachuteCheat() {
    CStreaming::RequestModel(MODEL_GUN_PARA, STREAMING_GAME_REQUIRED);
    CStreaming::LoadAllRequestedModels(false);
    CPlayerPed* player = FindPlayerPed();
    player->GiveWeapon(WEAPON_PARACHUTE, 0, true);
    player->SetCurrentWeapon(WEAPON_PARACHUTE);
}

// 0x4390d0
void CCheat::PinkCarsCheat() {
    Toggle(PINKCARS_CHEAT);
    if (IsActive(PINKCARS_CHEAT)) {
        Disable(BLACKCARS_CHEAT);
    }
}

// 0x439710
void CCheat::RiotCheat() {
    Toggle(RIOT_CHEAT);
}

// 0x439190
void CCheat::SkinnyCheat() {
    CStats::SetStatValue(STAT_FAT, 0.0f);
    CStats::SetStatValue(STAT_MUSCLE, 0.0f);

    CPlayerPed* player = FindPlayerPed();
    if (player->m_nPedState != PEDSTATE_DRIVING) {
        CClothes::RebuildPlayer(player, false);
    }
}

// 0x439930
void CCheat::StaminaCheat() {
    CStats::SetStatValue(STAT_STAMINA, 1000.0f);
}

// 0x439570
void CCheat::StormCheat() {
    RainyWeatherCheat();
    CWeather::Rain = 1.0f;
    CWeather::Wind = 1.0f;
}

// 0x43a570
void CCheat::TankerCheat() {
    CVehicle* vehicle = VehicleCheat(MODEL_PETRO);
    if (!vehicle)
        return;

    CStreaming::RequestModel(MODEL_PETROTR, 0);
    CStreaming::LoadAllRequestedModels(false);

    if (!CStreaming::GetInfo(MODEL_PETROTR).IsLoaded())
        return;

    auto* trailer = new CTrailer(MODEL_PETROTR, RANDOM_VEHICLE);
    trailer->SetPosn(vehicle->GetPosition());
    trailer->SetOrientation(0.0f, 0.0f, DegreesToRadians(200));
    trailer->SetStatus(STATUS_ABANDONED);
    CWorld::Add(trailer);
    trailer->SetTowLink(vehicle, true);
}

// 0x43A0B0
CVehicle* CCheat::VehicleCheat(eModelID modelId) {
    const auto player = FindPlayerPed();
    if (player->GetAreaCode() != AREA_CODE_NORMAL_WORLD) {
        return nullptr;
    }

    // 0x43A0E2 - Don't let the cheat flood the world with vehicles
    if (GetVehiclePool()->GetNoOfUsedSpaces() > 50) {
        return nullptr;
    }

    CStreaming::RequestModel(modelId, STREAMING_GAME_REQUIRED);
    CStreaming::LoadAllRequestedModels(false);
    if (!CStreaming::IsModelLoaded(modelId)) {
        return nullptr;
    }

    if (!CStreaming::GetInfo(modelId).IsGameRequired()) {
        CStreaming::SetModelIsDeletable(modelId);
        CStreaming::SetModelTxdIsDeletable(modelId);
    }
    
    auto* const vehicle = CCarGenerator::CreateVehicle(modelId, RANDOM_VEHICLE); // NB(NOTSA): Made this function instead of having shit here (Also fixes train spawning!)
    if (!vehicle) {
        return nullptr;
    }

    const float radius = vehicle->GetModelInfo()->GetColModel()->GetBoundRadius();
    const auto  rotZ   = player->m_fCurrentRotation + HALF_PI;
    const auto  pos    = player->GetPosition() + (radius + 2.0f) * player->GetForward();

    vehicle->SetPosn(pos);
    vehicle->SetOrientation(0.0f, 0.0f, rotZ);
    vehicle->SetStatus(STATUS_ABANDONED);
    vehicle->m_nDoorLock = CARLOCK_UNLOCKED;
    CWorld::Add(vehicle);
    CTheScripts::ClearSpaceForMissionEntity(pos, vehicle);

    switch (vehicle->GetBaseVehicleType()) {
    case VEHICLE_TYPE_TRAIN: {
        const auto train = vehicle->AsTrain();
        train->FindPositionOnTrackFromCoors();
        break;
    }
    case VEHICLE_TYPE_BOAT:
        break; /* nop */
    case VEHICLE_TYPE_BIKE:
        vehicle->AsBike()->PlaceOnRoadProperly();
        break;
    default:
        // todo: CAutomobile::PlaceOnRoadProperly Places skimmer incorrectly, it's placed at millions units underground
        vehicle->AsAutomobile()->PlaceOnRoadProperly();
        break;
    }

    return vehicle;
}

// 0x43a550
void CCheat::ApacheCheat() {
    VehicleCheat(MODEL_HUNTER);
}

// 0x43a660
void CCheat::DozerCheat() {
    VehicleCheat(MODEL_DOZER);
}

// 0x43a530
void CCheat::FlyboyCheat() {
    VehicleCheat(MODEL_HYDRA);
}

// 0x43a520
void CCheat::GolfcartCheat() {
    VehicleCheat(MODEL_CADDY);
}

// 0x43a4f0
void CCheat::HearseCheat() {
    VehicleCheat(MODEL_ROMERO);
}

// 0x43a500
void CCheat::LovefistCheat() {
    VehicleCheat(MODEL_STRETCH);
}

// 0x43a680
void CCheat::MonsterTruckCheat() {
    VehicleCheat(MODEL_MONSTERA);
}

// unused
// 0x4395a0
void CCheat::PredatorCheat() {
    VehicleCheat(MODEL_PREDATOR);
}

// 0x43a560
void CCheat::QuadCheat() {
    VehicleCheat(MODEL_QUAD);
}
// 0x43a4b0
void CCheat::StockCarCheat() {
    VehicleCheat(MODEL_BLOODRA);
}

// 0x43a4c0
void CCheat::StockCar2Cheat() {
    VehicleCheat(MODEL_RNCHLURE);
}

// 0x43a4d0
void CCheat::StockCar3Cheat() {
    VehicleCheat(MODEL_HOTRINA);
}

// 0x43a4e0
void CCheat::StockCar4Cheat() {
    VehicleCheat(MODEL_HOTRINB);
}

// 0x43a670
void CCheat::StuntPlaneCheat() {
    VehicleCheat(MODEL_STUNT);
}

// 0x43a4a0
void CCheat::TankCheat() {
    VehicleCheat(MODEL_RHINO);
}

// 0x43a510
void CCheat::TrashmasterCheat() {
    VehicleCheat(MODEL_TRASH);
}

// 0x43a540
void CCheat::VortexCheat() {
    VehicleCheat(MODEL_VORTEX);
}

// 0x4399d0
void CCheat::VehicleSkillsCheat() {
    CStats::SetStatValue(STAT_DRIVING_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_FLYING_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_BIKE_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_CYCLING_SKILL, 1000.0f);
}

// 0x439c70
void CCheat::VillagePeopleCheat() {
    Toggle(VILLAGEPEOPLE_CHEAT);
    if (IsActive(VILLAGEPEOPLE_CHEAT)) {
        Disable(WEAPONSFORALL_CHEAT);
        EverybodyAttacksPlayerCheat();
    } else {
        int32 peds[] = {
            MODEL_WMYCON, MODEL_CWMYFR, MODEL_ARMY, MODEL_LAPDM1,
            MODEL_BIKERB, UNLOAD_MODEL, UNLOAD_MODEL, UNLOAD_MODEL,
        };
        CStreaming::StreamPedsIntoRandomSlots(peds);
        EverybodyAttacksPlayerCheat();
        Enable(WEAPONSFORALL_CHEAT);
    }
}

// 0x4396f0
void CCheat::WantedCheat() {
    CPlayerPed* player = FindPlayerPed();
    player->CheatWantedLevel(eWantedLevel::WANTED_LEVEL_6);
}

// 0x438f20
void CCheat::WantedLevelDownCheat() {
    CPlayerPed* player = FindPlayerPed();
    player->CheatWantedLevel(eWantedLevel::WANTED_CLEAN);
}

// 0x438e90
void CCheat::WantedLevelUpCheat() {
    CPlayerPed* player = FindPlayerPed();

    if (!player)
        return;

    const auto level = player->GetWantedLevel();
    player->CheatWantedLevel((eWantedLevel)(std::min(+level + +eWantedLevel::WANTED_LEVEL_2, +eWantedLevel::WANTED_LEVEL_6)));
}

// refactored
// 0x4385b0
void CCheat::WeaponCheat1() {
    static constexpr eModelID weapons[] = {
        MODEL_BRASSKNUCKLE, MODEL_BAT,       MODEL_MOLOTOV, MODEL_COLT45,
        MODEL_CHROMEGUN,    MODEL_MICRO_UZI, MODEL_AK47,    MODEL_CUNTGUN,
        MODEL_ROCKETLA,     MODEL_SPRAYCAN
    };
    std::ranges::for_each(weapons, [](auto model) { CStreaming::RequestModel(model, STREAMING_GAME_REQUIRED); });
    CStreaming::LoadAllRequestedModels(false);

    CPlayerPed* player = FindPlayerPed(0);
    player->GiveWeaponSet1();

    CPlayerPed* player1 = FindPlayerPed(1);
    if (player1) {
        player->GiveWeaponSet1();
    }

    std::ranges::for_each(weapons, [](auto model) { CStreaming::SetModelIsDeletable(model); });
}

// refactored
// 0x438890
void CCheat::WeaponCheat2() {
    static constexpr eModelID weapons[] = {
        MODEL_KNIFECUR, MODEL_GRENADE, MODEL_DESERT_EAGLE, MODEL_SAWNOFF,
        MODEL_TEC9,     MODEL_M4,      MODEL_SNIPER,       MODEL_FLAME,
        MODEL_FIRE_EX
    };
    std::ranges::for_each(weapons, [](auto model) { CStreaming::RequestModel(model, STREAMING_GAME_REQUIRED); });
    CStreaming::LoadAllRequestedModels(false);

    CPlayerPed* player = FindPlayerPed(0);
    player->GiveWeaponSet2();

    CPlayerPed* player1 = FindPlayerPed(1);
    if (player1) {
        player1->GiveWeaponSet2();
    }

    std::ranges::for_each(weapons, [](auto model) { CStreaming::SetModelIsDeletable(model); });
}

// refactored
// 0x438b30
void CCheat::WeaponCheat3() {
    static constexpr eModelID weapons[] = {
        MODEL_CHNSAW, MODEL_SILENCED, MODEL_SHOTGSPA, MODEL_MP5LNG,
        MODEL_M4,     MODEL_HEATSEEK, MODEL_SATCHEL,  MODEL_BOMB
    };
    std::ranges::for_each(weapons, [](auto model) { CStreaming::RequestModel(model, STREAMING_GAME_REQUIRED); });
    CStreaming::LoadAllRequestedModels(false);

    CPlayerPed* player = FindPlayerPed(0);
    player->GiveWeaponSet3();

    CPlayerPed* player1 = FindPlayerPed(1);
    if (player1) {
        player1->GiveWeaponSet3();
    }

    std::ranges::for_each(weapons, [](auto model) { CStreaming::SetModelIsDeletable(model); });
}

#ifdef USE_ADDITIONAL_CHEATS
// Android
void CCheat::WeaponCheat4() {
    static bool bNVGoggles = false;

    const eModelID gogglesModel = bNVGoggles ? MODEL_NVGOGGLES : MODEL_IRGOGGLES;
    const eWeaponType gogglesWeapon = bNVGoggles ? WEAPON_NIGHTVISION : WEAPON_INFRARED;

    const eModelID weapons[] = { MODEL_MINIGUN, MODEL_GUN_DILDO2, gogglesModel };
    std::ranges::for_each(weapons, [](auto model) { CStreaming::RequestModel(model, STREAMING_GAME_REQUIRED); });
    CStreaming::LoadAllRequestedModels(false);

    for (int32 playerId : { 0, 1 }) {
        if (auto* player = FindPlayerPed(playerId)) {
            player->GiveWeapon(WEAPON_DILDO2, 1, true);
            player->GiveWeapon(WEAPON_MINIGUN, 500, true);
            player->GiveWeapon(gogglesWeapon, 1, true);
        }
    }

    std::ranges::for_each(weapons, [](auto model) { CStreaming::SetModelIsDeletable(model); });

    bNVGoggles = !bNVGoggles;
}
#endif

// 0x439940
void CCheat::WeaponSkillsCheat() {
    CStats::SetStatValue(STAT_PISTOL_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_SILENCED_PISTOL_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_DESERT_EAGLE_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_SHOTGUN_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_SAWN_OFF_SHOTGUN_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_COMBAT_SHOTGUN_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_MACHINE_PISTOL_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_SMG_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_AK_47_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_M4_SKILL, 1000.0f);
    CStats::SetStatValue(STAT_RIFLE_SKILL, 1000.0f);
}

// 0x438f60
void CCheat::CloudyWeatherCheat() {
    CWeather::ForceWeatherNow(WEATHER_CLOUDY_LA);
}

// 0x438f50
void CCheat::ExtraSunnyWeatherCheat() {
    CWeather::ForceWeatherNow(WEATHER_EXTRASUNNY_LA);
}

// 0x438f80
void CCheat::FoggyWeatherCheat() {
    CWeather::ForceWeatherNow(WEATHER_FOGGY_SF);
}

// 0x438f70
void CCheat::RainyWeatherCheat() {
    CWeather::ForceWeatherNow(WEATHER_RAINY_COUNTRYSIDE);
}

// 0x439590
void CCheat::SandstormCheat() {
    CWeather::ForceWeatherNow(WEATHER_SANDSTORM_DESERT);
}

// 0x438f40
void CCheat::SunnyWeatherCheat() {
    CWeather::ForceWeatherNow(WEATHER_SUNNY_LA);
}

// 0x438ff0
void CCheat::SuicideCheat() {
    CPedDamageResponseCalculator damageCalculator(nullptr, 1000.0f, WEAPON_UNARMED, PED_PIECE_TORSO, false);
    CEventDamage damageEvent(nullptr, CTimer::GetTimeInMS(), WEAPON_UNARMED, PED_PIECE_TORSO, 0, false, false);
    CPlayerPed* player = FindPlayerPed();
    if (damageEvent.AffectsPed(player))
        damageCalculator.ComputeDamageResponse(player, damageEvent.m_damageResponse, true);
    else
        damageEvent.m_damageResponse.m_bDamageCalculated = true;
    player->GetEventGroup().Add(&damageEvent, false);
}

// 0x407410
bool CCheat::IsZoneStreamingAllowed() {
    return IsActive(ELVISLIVES_CHEAT)
        || IsActive(VILLAGEPEOPLE_CHEAT)
        || IsActive(BEACHPARTY_CHEAT)
        || IsActive(GANGS_CHEAT)
        || IsActive(NINJA_CHEAT)
        || IsActive(LOVECONQUERSALL_CHEAT)
        || IsActive(FUNHOUSE_CHEAT)
        || IsActive(COUNTRYSIDEINVASION_CHEAT);
}

#ifdef USE_ADDITIONAL_CHEATS
// Android
void CCheat::TimeTravelCheat() {
    CClock::SetGameClock(CClock::GetGameClockHours() + 4, CClock::GetGameClockMinutes(), CClock::GetGameClockDays());
}

// Android
void CCheat::TheGamblerCheat() {
    CStats::SetStatValue(eStats::STAT_GAMBLING, 1000.0f);
}

void CCheat::BigHeadCheat() {
    Toggle(CHEAT_BIG_HEAD);
}

void CCheat::ThinBodyCheat() {
    Toggle(CHEAT_THIN_BODY);
}

// **** DEBUG STUFF ****

// Android
void CCheat::TogglePlayerInvincibility() {
    CPlayerPed::bDebugPlayerInvincible ^= true;
}

// Android
void CCheat::ToggleShowTargeting() {
    CPlayerPed::bDebugTargeting ^= true;
}

// Android
void CCheat::ToggleShowTapToTarget() {
    CPlayerPed::bDebugTapToTarget ^= true;
}

// Android
void CCheat::ShowMappingsCheat() {
    m_bShowMappings ^= true;
}

// Android
void CCheat::ScriptBypassCheat() {
    m_nLastScriptBypasstime = CTimer::GetTimeInMS();
}

void CCheat::ProcessAllCheats() {
    ProcessDebugCarCheats();
    ProcessDebugMissionSkip();
    ProcessCheats();
    ProcessCheatMenu();
    ProcessWeaponSlotCheats();
}

void CCheat::ProcessDebugCarCheats() {

}

void CCheat::ProcessDebugMissionSkip() {

}

void CCheat::ProcessCheats() {

}

void CCheat::ProcessCheatMenu() {

}

enum eCheatWeaponSlot {
    SLOT_MELEE,
    SLOT_HANDGUN,
    SLOT_SMG,
    SLOT_SHOTGUN,
    SLOT_ASSAULT_RIFLES,
    SLOT_LONG_RIFLES,
    SLOT_THROWN,
    SLOT_HEAVY_ARTILLERY,
    SLOT_EQUIPMENT,
    SLOT_OTHER,
};

static int32 g_CheatWeaponSlot = -1;
static bool  g_bDisplayCheatWeaponSlot = true;

// Android
void CCheat::ProcessWeaponSlotCheats() {
    if (!g_bDisplayCheatWeaponSlot)
        return;

    CTimer::StartUserPause();
    switch (g_CheatWeaponSlot) {
    case SLOT_MELEE:
        // HandleSlotMelee();
        break;
    case SLOT_HANDGUN:
        // HandleSlotHandguns();
        break;
    case SLOT_SMG:
        // HandleSlotSubmachineGuns();
        break;
    case SLOT_SHOTGUN:
        // HandleSlotShotguns();
        break;
    case SLOT_ASSAULT_RIFLES:
        // HandleSlotAssaultRifles();
        break;
    case SLOT_LONG_RIFLES:
        // HandleSlotLongRifles();
        break;
    case SLOT_THROWN:
        // HandleSlotThrow();
        break;
    case SLOT_HEAVY_ARTILLERY:
        // HandleSlotArtillery();
        break;
    case SLOT_EQUIPMENT:
        // HandleSlotEquipment();
        break;
    case SLOT_OTHER:
        // HandleSlotOther();
        break;
    default:
        break;
    }
    CTimer::EndUserPause();
}

// Can be realized through CMenuSystem
// Android
void CCheat::WeaponSlotCheat() {
    // SLOT_MELEE           "NIGHTSTICK" "BRASS KNUCKLES" "POOL CUE" "BASEBALL BAT" "CHAINSAW" "KATANA" "GOLF CLUB" "SHOVEL" "KNIFE"
    // SLOT_HANDGUN         "DESERT EAGLE" "SLIENCED 9MM"
    // SLOT_SMG             "TEC-9" "MICRO SMG"
    // SLOT_SHOTGUN         "SHOTGUN" "SPAS"
    // SLOT_ASSAULT_RIFLES  "AK47" "M4"
    // SLOT_LONG_RIFLES     "RIFLE" "SNIPER RIFLE"
    // SLOT_THROWN          "GRENADE" "MOLOTOV COCKTAIL" "C4" "TEAR GAS"
    // SLOT_HEAVY_ARTILLERY "MINI GUN" "FLAME THROWER" "HS ROCKET LAUNCHER" "ROCKET LAUNCHER"
    // SLOT_EQUIPMENT       "NIGHT-VISION GOGGLES" "FIRE EXTINGUISHER" "SPRAY CAN" "PARACHUTE" "CAMERA" "THERMAL GOGGLES"
    // SLOT_OTHER           "VIBRA2" "DILDO1" "CANE" "DILDO2" "FLOWERS" "VIBRA1"
}
#endif

void CCheat::InjectHooks() {
    RH_ScopedClass(CCheat);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(AddToCheatString, 0x438480);
    RH_ScopedInstall(HandleSpecialCheats, 0x439A10);
    RH_ScopedInstall(DoCheats, 0x439AF0);
    RH_ScopedInstall(ResetCheats, 0x438450);
    RH_ScopedInstall(IsZoneStreamingAllowed, 0x407410);
    RH_ScopedInstall(ApplyCheat, 0x438370);
    RH_ScopedInstall(VehicleCheat, 0x43A0B0);

    RH_ScopedInstall(WeaponCheat1, 0x4385B0);
    RH_ScopedInstall(WeaponCheat2, 0x438890);
    RH_ScopedInstall(WeaponCheat3, 0x438B30);
    RH_ScopedInstall(MoneyArmourHealthCheat, 0x438E40);
    RH_ScopedInstall(WantedLevelUpCheat, 0x438E90);
    RH_ScopedInstall(WantedLevelDownCheat, 0x438F20);
    RH_ScopedInstall(SunnyWeatherCheat, 0x438F40);
    RH_ScopedInstall(ExtraSunnyWeatherCheat, 0x438F50);
    RH_ScopedInstall(CloudyWeatherCheat, 0x438F60);
    RH_ScopedInstall(RainyWeatherCheat, 0x438F70);
    RH_ScopedInstall(FoggyWeatherCheat, 0x438F80);
    RH_ScopedInstall(FastTimeCheat, 0x438F90);
    RH_ScopedInstall(SlowTimeCheat, 0x438FC0);
    RH_ScopedInstall(SuicideCheat, 0x438FF0);
    RH_ScopedInstall(PinkCarsCheat, 0x4390D0);
    RH_ScopedInstall(BlackCarsCheat, 0x4390F0);
    RH_ScopedInstall(FatCheat, 0x439110);
    RH_ScopedInstall(MuscleCheat, 0x439150);
    RH_ScopedInstall(SkinnyCheat, 0x439190);
    RH_ScopedInstall(ElvisLivesCheat, 0x4391D0);
    RH_ScopedInstall(BeachPartyCheat, 0x439230);
    RH_ScopedInstall(GangsCheat, 0x439360);
    RH_ScopedInstall(GangLandCheat, 0x4393D0);
    RH_ScopedInstall(LoveConquersAllCheat, 0x4393F0);
    RH_ScopedInstall(AllCarsAreShitCheat, 0x4394B0);
    RH_ScopedInstall(AllCarsAreGreatCheat, 0x4394E0);
    RH_ScopedInstall(MidnightCheat, 0x439510);
    RH_ScopedInstall(DuskCheat, 0x439540);
    RH_ScopedInstall(StormCheat, 0x439570);
    RH_ScopedInstall(SandstormCheat, 0x439590);
    RH_ScopedInstall(PredatorCheat, 0x4395A0);
    RH_ScopedInstall(ParachuteCheat, 0x4395B0);
    RH_ScopedInstall(JetpackCheat, 0x439600);
    RH_ScopedInstall(NotWantedCheat, 0x4396C0);
    RH_ScopedInstall(WantedCheat, 0x4396F0);
    RH_ScopedInstall(RiotCheat, 0x439710);
    RH_ScopedInstall(FunhouseCheat, 0x439720);
    RH_ScopedInstall(AdrenalineCheat, 0x439880);
    RH_ScopedInstall(DrivebyCheat, 0x4398D0);
    RH_ScopedInstall(StaminaCheat, 0x439930);
    RH_ScopedInstall(WeaponSkillsCheat, 0x439940);
    RH_ScopedInstall(VehicleSkillsCheat, 0x4399D0);
    RH_ScopedInstall(MayhemCheat, 0x439B20);
    RH_ScopedInstall(EverybodyAttacksPlayerCheat, 0x439C70);
    RH_ScopedInstall(BlowUpCarsCheat, 0x439D80);
    RH_ScopedInstall(VillagePeopleCheat, 0x439DD0);
    RH_ScopedInstall(NinjaCheat, 0x439E50);
    RH_ScopedInstall(CountrysideInvasionCheat, 0x439F60);
    RH_ScopedInstall(TankCheat, 0x43A4A0);
    RH_ScopedInstall(StockCarCheat, 0x43A4B0);
    RH_ScopedInstall(StockCar2Cheat, 0x43A4C0);
    RH_ScopedInstall(StockCar3Cheat, 0x43A4D0);
    RH_ScopedInstall(StockCar4Cheat, 0x43A4E0);
    RH_ScopedInstall(HearseCheat, 0x43A4F0);
    RH_ScopedInstall(LovefistCheat, 0x43A500);
    RH_ScopedInstall(TrashmasterCheat, 0x43A510);
    RH_ScopedInstall(GolfcartCheat, 0x43A520);
    RH_ScopedInstall(FlyboyCheat, 0x43A530);
    RH_ScopedInstall(VortexCheat, 0x43A540);
    RH_ScopedInstall(ApacheCheat, 0x43A550);
    RH_ScopedInstall(QuadCheat, 0x43A560);
    RH_ScopedInstall(TankerCheat, 0x43A570);
    RH_ScopedInstall(DozerCheat, 0x43A660);
    RH_ScopedInstall(StuntPlaneCheat, 0x43A670);
    RH_ScopedInstall(MonsterTruckCheat, 0x43A680);
}

