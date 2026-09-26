#include "StdInc.h"

#include "ObjectSaveStructure.h"

void CObjectSaveStructure::InjectHooks()
{
    RH_ScopedClass(CObjectSaveStructure);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Construct, 0x5D2160);
    RH_ScopedInstall(Extract, 0x5D2220);
}

void CObjectSaveStructure::Construct(CObject* obj)
{
    m_matrix.CompressFromFullMatrix(*obj->m_matrix);
    m_nBonusValue  = obj->m_nBonusValue;
    m_nCostValue   = obj->m_wCostValue;
    m_nRemovalTime = obj->m_nRemovalTime;
    m_nEntityFlags = obj->m_nFlags;
    m_nObjectFlags = obj->m_nObjectFlagsRaw;
    m_nObjectType  = obj->m_nObjectType;
    m_nColDamageEffect = obj->m_nColDamageEffect;
    m_nSpecialColResponseCase = obj->m_nSpecialColResponseCase;
    m_nUnused = 0;

    m_nPhysicalFlags = 0;

    if (obj->m_nPhysicalFlags.bInfiniteMass)
        m_bDisableCollisionForce = true;

    if (obj->m_nPhysicalFlags.bCoorsFrozenByScript)
        m_bDontApplySpeed = true;

    if (obj->m_nPhysicalFlags.bNotDamagedByBullets)
        m_bBulletProof = true;

    if (obj->m_nPhysicalFlags.bNotDamagedByFlames)
        m_bFireProof = true;

    if (obj->m_nPhysicalFlags.bNotDamagedByCollisions)
        m_bCollisionProof = true;

    if (obj->m_nPhysicalFlags.bNotDamagedByMelee)
        m_bMeleeProof = true;

    if (obj->m_nPhysicalFlags.bIgnoresExplosions)
        m_bExplosionProof = true;
}

void CObjectSaveStructure::Extract(CObject* obj)
{
    m_matrix.DecompressIntoFullMatrix(*obj->m_matrix);
    obj->m_nBonusValue = m_nBonusValue;
    obj->m_wCostValue = m_nCostValue;
    obj->m_nRemovalTime = m_nRemovalTime;
    obj->m_nFlags = m_nEntityFlags;
    obj->m_nObjectFlagsRaw = m_nObjectFlags;
    obj->m_nObjectType = m_nObjectType;
    obj->m_nColDamageEffect = m_nColDamageEffect;
    obj->m_nSpecialColResponseCase = m_nSpecialColResponseCase;

    obj->m_nPhysicalFlags.bInfiniteMass = m_bDisableCollisionForce;
    obj->m_nPhysicalFlags.bCoorsFrozenByScript = m_bDontApplySpeed;
    obj->m_nPhysicalFlags.bNotDamagedByBullets = m_bBulletProof;
    obj->m_nPhysicalFlags.bNotDamagedByFlames = m_bFireProof;
    obj->m_nPhysicalFlags.bNotDamagedByCollisions = m_bCollisionProof;
    obj->m_nPhysicalFlags.bNotDamagedByMelee = m_bMeleeProof;
    obj->m_nPhysicalFlags.bIgnoresExplosions = m_bExplosionProof;
}
