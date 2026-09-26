#include "StdInc.h"

#include "Remote.h"

void CRemote::InjectHooks() {
    RH_ScopedClass(CRemote);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(TakeRemoteControlledCarFromPlayer, 0x45AE80);
}

// 0x45AE80
// ASM Match: not measured
void CRemote::TakeRemoteControlledCarFromPlayer(bool bCreateExplosion)
{
    assert(CWorld::Players[CWorld::PlayerInFocus].pRemoteVehicle != nullptr);

    if (CWorld::Players[CWorld::PlayerInFocus].pRemoteVehicle->GetCreatedBy() == MISSION_VEHICLE)
    {
        CWorld::Players[CWorld::PlayerInFocus].pRemoteVehicle->SetVehicleCreatedBy(RANDOM_VEHICLE);

        int32 VehiclePoolIndex = (*GetVehiclePool()).GetIndex(CWorld::Players[CWorld::PlayerInFocus].pRemoteVehicle);

        CTheScripts::MissionCleanUp.RemoveEntityFromList(VehiclePoolIndex, MISSION_CLEANUP_ENTITY_TYPE_VEHICLE);
    }

    CWorld::Players[CWorld::PlayerInFocus].pRemoteVehicle->vehicleFlags.bIsLocked = false;

    CWorld::Players[CWorld::PlayerInFocus].TimeOfRemoteVehicleExplosion = CTimer::GetTimeInMS();
    CWorld::Players[CWorld::PlayerInFocus].bAfterRemoteVehicleExplosion = true;
    CWorld::Players[CWorld::PlayerInFocus].bCreateRemoteVehicleExplosion = bCreateExplosion;
    CWorld::Players[CWorld::PlayerInFocus].bFadeAfterRemoteVehicleExplosion = true;
}

void CRemote::GivePlayerRemoteControlledCar(CVector pos, float rotation, int16 modelId)
{
    return plugin::Call<0x45AB10, CVector, float, int16>(pos, rotation, modelId);
}

void CRemote::TakeRemoteControlOfCar(CVehicle* vehicle)
{
    return plugin::Call<0x45AD40, CVehicle*>(vehicle);
}
