#include "StdInc.h"
#include "PatrolRoute.h"


void CPatrolRoute::InjectHooks() {
    RH_ScopedClass(CPatrolRoute);
    RH_ScopedCategory("Core");

    RH_ScopedInstall(operator new, 0x41B810);
    RH_ScopedInstall(operator delete, 0x41B820);
    RH_ScopedInstall(Destructor, 0x6677A0);
    RH_ScopedInstall(Add, 0x46AE80);
}

// 0x41B810
void* CPatrolRoute::operator new(size_t size) {
    return GetPatrolRoutePool()->New();
}

// 0x41B820
void CPatrolRoute::operator delete(void* ptr, size_t sz) {
    GetPatrolRoutePool()->Delete(reinterpret_cast<CPatrolRoute*>(ptr));
}

// 0x46AE80
bool CPatrolRoute::Add(const CVector& point, const CAnimNameDescriptor& animDesc) {
    if (m_iRouteSize >= MAX_NUM_ROUTE_ELEMENTS) {
        return false;
    }

    m_routePoints[m_iRouteSize] = point;
    m_anims[m_iRouteSize] = animDesc;
    ++m_iRouteSize;
    return true;
}
