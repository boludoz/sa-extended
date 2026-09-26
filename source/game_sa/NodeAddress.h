/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

class CNodeAddress {
public:
    uint16 Region{ (uint16)-1 };
    uint16 Index{ (uint16)-1 };

    constexpr CNodeAddress() = default;
    constexpr CNodeAddress(uint16 areaId, uint16 nodeId) : Region(areaId), Index(nodeId) {}

    bool operator==(CNodeAddress const&) const = default;
    bool operator!=(CNodeAddress const&) const = default;

    //! 0x420980 - Ordering by area first, then node. Used to give a link between two nodes a
    //! canonical direction, independent of which end you look at it from.
    auto operator<=>(CNodeAddress const&) const = default;

    void ResetAreaId() { Region = UINT16_MAX; }
    void ResetNodeId() { Index = UINT16_MAX; }

    [[nodiscard]] bool IsAreaValid() const { return Region != (uint16)-1; }
    [[nodiscard]] bool IsValid() const { return IsAreaValid() && Index != UINT16_MAX; }
    [[nodiscard]] bool IsEmpty() const { return !IsValid(); }

    operator bool() const { return IsValid(); }

    uint32 GetRegion() const { return Region; }
    uint32 GetIndex() const { return Index; }
    void Set(uint32 region, uint32 index) {
        Region = (uint16)region;
        Index  = (uint16)index;
    }
    void SetEmpty() { Region = UINT16_MAX; }
};
VALIDATE_SIZE(CNodeAddress, 0x4);
