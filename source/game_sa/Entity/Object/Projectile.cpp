#include "StdInc.h"

#include "Projectile.h"

void CProjectile::InjectHooks() {
    RH_ScopedVirtualClass(CProjectile, 0x867030, 23);
    RH_ScopedCategory("Entity/Object");

    RH_ScopedInstall(Constructor, 0x5A4030);
    RH_ScopedInstall(Destructor, 0x5A40E0);
}

// 0x5A4030
CProjectile::CProjectile(int32 modelId) : CObject() {
    m_bTunnelTransition = true;
    m_fMass = 1.0F;
    m_fTurnMass = 1.0F;
    m_fAirResistance = 0.99999F;
    m_fElasticity = 0.75F;
    m_fBuoyancyConstant = 1.0F / 1250.0F; // 0.0008F
    CObject::SetIsStatic(false);
    physicalFlags.bExplosionProof = true;
    CEntity::SetModelIndex(modelId);
    m_nObjectType = eObjectType::OBJECT_TYPE_DECORATION;
}

CProjectile* CProjectile::Constructor(int32 modelId) {
    this->CProjectile::CProjectile(modelId);
    return this;
}

// 0x5A40E0
CProjectile* CProjectile::Destructor() {
    this->CProjectile::~CProjectile();
    return this;
}
