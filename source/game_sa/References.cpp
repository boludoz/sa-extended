#include "StdInc.h"

#include "References.h"

void CReferences::InjectHooks() {
    RH_ScopedClass(CReferences);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Init, 0x5719B0);
    RH_ScopedInstall(ListSize, 0x5719E0);
    RH_ScopedInstall(RemoveReferencesToPlayer, 0x571AD0);
    RH_ScopedInstall(PruneAllReferencesInWorld, 0x571CC0);
}

// 0x5719B0
void CReferences::Init() {
    pEmptyList = &aRefs[0];

    for (int32 i = 0; i < MAX_NUM_REFERENCES; ++i) {
        aRefs[i].m_pNext = &aRefs[i + 1];
        aRefs[i].m_ppEntity = nullptr;
    }

    aRefs[MAX_NUM_REFERENCES - 1].m_pNext = nullptr;
}

uint32 CReferences::ListSize(CReference* ref) {
    CReference* pRef = ref;
    uint32 size = 0;

    while (pRef != nullptr) {
        ++size;
        pRef = pRef->m_pNext;
    }

    return size;
}

void CReferences::RemoveReferencesToPlayer() {
    auto vehicle = FindPlayerVehicle();
    if (vehicle)
        vehicle->ResolveReferences();

    auto player = FindPlayerPed();
    if (player) {
        player->ResolveReferences();
        auto& group = CPedGroups::GetGroup(player->GetPlayerData()->m_nPlayerGroup);
        group.GetMembership().SetLeader(player);
        group.Process();
    }
}

void CReferences::PruneAllReferencesInWorld() {
    auto* pedPool = GetPedPool();
    for (int32 i = pedPool->GetSize() - 1; i >= 0; --i) {
        CPed* pPed = pedPool->GetAt(i);
        if (pPed != nullptr) {
            pPed->PruneReferences();
        }
    }

    auto* vehiclePool = GetVehiclePool();
    for (int32 i = vehiclePool->GetSize() - 1; i >= 0; --i) {
        CVehicle* pVeh = vehiclePool->GetAt(i);
        if (pVeh != nullptr) {
            pVeh->PruneReferences();
        }
    }

    auto* objectPool = GetObjectPool();
    for (int32 i = objectPool->GetSize() - 1; i >= 0; --i) {
        CObject* pObj = objectPool->GetAt(i);
        if (pObj != nullptr) {
            pObj->PruneReferences();
        }
    }
}
