/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

class CNodeAddress {
public:
    uint16 m_wAreaId{ (uint16)-1 };
    uint16 m_wNodeId{ (uint16)-1 };

    constexpr CNodeAddress() = default;
    constexpr CNodeAddress(uint16 areaId, uint16 nodeId) : m_wAreaId(areaId), m_wNodeId(nodeId) {}

    bool operator==(CNodeAddress const&) const = default;
    bool operator!=(CNodeAddress const&) const = default;

    //! 0x420980 - Ordering by area first, then node. Used to give a link between two nodes a
    //! canonical direction, independent of which end you look at it from.
    auto operator<=>(CNodeAddress const&) const = default;

    void ResetAreaId() { m_wAreaId = UINT16_MAX; }
    void ResetNodeId() { m_wNodeId = UINT16_MAX; }

    [[nodiscard]] bool IsAreaValid() const { return m_wAreaId != (uint16)-1; }
    [[nodiscard]] bool IsValid() const { return IsAreaValid() && m_wNodeId != UINT16_MAX; }

    operator bool() const { return IsValid(); }
};
VALIDATE_SIZE(CNodeAddress, 0x4);
