#pragma once

class CVehicle;

class CRemote {
public:
    static void TakeRemoteControlledCarFromPlayer(bool bCreateRemoteVehicleExplosion);
    static void GivePlayerRemoteControlledCar(CVector pos, float rotation, int16 modelId);
    static void TakeRemoteControlOfCar(CVehicle* vehicle);
};
