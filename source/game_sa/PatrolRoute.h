#pragma once

#include "Vector.h"
#include "AnimNameDescriptor.h"
#include <utility>


class CPatrolRoute {
public:
    enum {
        MAX_NUM_ROUTE_ELEMENTS = 8
    };

    CPatrolRoute() {
        Clear();
    }

    CPatrolRoute(const CPatrolRoute& o) {
        From(o);
    }

    ~CPatrolRoute() = default;

    CPatrolRoute* Destructor() {
        this->CPatrolRoute::~CPatrolRoute();
        return this;
    }

    CPatrolRoute& operator=(const CPatrolRoute& o) {
        if (this != &o) {
            From(o);
        }
        return *this;
    }

    // 0x41B810
    static void* operator new(size_t size);

    // 0x41B820
    static void operator delete(void* ptr, size_t sz);

    bool operator==(const CPatrolRoute& o) const {
        if (m_iRouteSize != o.m_iRouteSize)
            return false;
        for (int32 i = 0; i < m_iRouteSize; ++i) {
            if (m_routePoints[i] != o.m_routePoints[i] || m_anims[i] != o.m_anims[i])
                return false;
        }
        return true;
    }

    bool operator!=(const CPatrolRoute& o) const {
        return !(*this == o);
    }

    void From(const CPatrolRoute& o) {
        m_iRouteSize = o.m_iRouteSize;
        for (int32 i = 0; i < m_iRouteSize; ++i) {
            m_routePoints[i] = o.m_routePoints[i];
            m_anims[i] = o.m_anims[i];
        }
    }

    void Clear() {
        m_iRouteSize = 0;
    }

    // 0x46AE80
    bool Add(const CVector& point, const CAnimNameDescriptor& animDesc);

    const CVector& Get(int32 i) const { return m_routePoints[i]; }
    const CAnimNameDescriptor& GetAnim(int32 i) const { return m_anims[i]; }
    int32 GetSize() const { return m_iRouteSize; }
    void SetSize(int32 size) { m_iRouteSize = size; }

    void Reverse() {
        for (int32 i0 = 0, i1 = m_iRouteSize - 1; i0 < i1; ++i0, --i1) {
            std::swap(m_routePoints[i0], m_routePoints[i1]);
            std::swap(m_anims[i0], m_anims[i1]);
        }
    }

    void Remove(int32 index) {
        for (int32 i = index; i < m_iRouteSize - 1; ++i) {
            m_routePoints[i] = m_routePoints[i + 1];
            m_anims[i] = m_anims[i + 1];
        }
        --m_iRouteSize;
    }

    static void InjectHooks();

private:
    int32 m_iRouteSize{0};
    CAnimNameDescriptor m_anims[MAX_NUM_ROUTE_ELEMENTS];
    CVector m_routePoints[MAX_NUM_ROUTE_ELEMENTS];
};
VALIDATE_SIZE(CPatrolRoute, 0x1A4);
