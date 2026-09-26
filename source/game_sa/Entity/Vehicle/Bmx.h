/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Bike.h"
#include "AnimBlendAssociation.h"

enum eBmxNodes {
    BMX_NULL,
    BMX_CHASSIS,
    BMX_FORKS,
    BMX_SWINGARM,
    BMX_WHEEL_F,
    BMX_WHEEL_R,
    BMX_HANDLEBARS,
    BMX_CHAINSET,
    BMX_PEDAL_L,
    BMX_PEDAL_R,
    MAX_BMX_NODES,
};

class NOTSA_EXPORT_VTABLE CBmx : public CBike {
public:
    float m_fControlJump;
    float m_fControlPedaling;
    float m_fSprintLeanAngle;
    float m_fCrankAngle;
    float m_fPedalAngleL;
    float m_fPedalAngleR;
    float m_fMidWheelDistY;
    float m_fMidWheelFracY;
    bool  m_bIsFreewheeling;

    static constexpr auto Type = VEHICLE_TYPE_BMX;

public:
    CBmx(int32 modelIndex, eVehicleCreatedBy createdBy);
    ~CBmx() override;

    void BlowUpCar(CEntity* damager, bool bHideExplosion) override;
    bool SetUpWheelColModel(CColModel* wheelCol) override;
    bool BurstTyre(uint8 tyreComponentId, bool bPhysicalEffect) override;
    void ProcessControl() override;
    void ProcessDrivingAnims(CPed* driver, bool blend) override;
    float FindWheelWidth(bool bRear) override;
    void PreRender() override;
    bool ProcessAI(uint32& extraHandlingFlags) override;

    void GetFrameOffset(float& fZOffset, float& fAngleOffset);
    void ProcessBunnyHop();
    static void LaunchBunnyHopCB(CAnimBlendAssociation* blendAssoc, void* data);

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CBmx* Constructor(int32 modelIndex, eVehicleCreatedBy createdBy) { this->CBmx::CBmx(modelIndex, createdBy); return this; }
    CBmx* Destructor() { this->CBmx::~CBmx(); return this; }

                                };
VALIDATE_SIZE(CBmx, 0x838);
