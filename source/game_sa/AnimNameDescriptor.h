#pragma once

#include <cstring>
#include <Base.h>

class CAnimNameDescriptor {
public:
    CAnimNameDescriptor() {
        Clear();
    }

    // 0x46A340
    CAnimNameDescriptor(const char* animName, const char* animGroupName) {
        if (animName) {
            strncpy(m_animName, animName, sizeof(m_animName) - 1);
            m_animName[sizeof(m_animName) - 1] = '\0';
        } else {
            m_animName[0] = '\0';
        }

        if (animGroupName) {
            strncpy(m_animGroupName, animGroupName, sizeof(m_animGroupName) - 1);
            m_animGroupName[sizeof(m_animGroupName) - 1] = '\0';
        } else {
            m_animGroupName[0] = '\0';
        }
    }

    CAnimNameDescriptor(const CAnimNameDescriptor& o) {
        memcpy(m_animName, o.m_animName, sizeof(m_animName));
        memcpy(m_animGroupName, o.m_animGroupName, sizeof(m_animGroupName));
    }

    ~CAnimNameDescriptor() = default;

    CAnimNameDescriptor& operator=(const CAnimNameDescriptor& o) {
        if (this != &o) {
            memcpy(m_animName, o.m_animName, sizeof(m_animName));
            memcpy(m_animGroupName, o.m_animGroupName, sizeof(m_animGroupName));
        }
        return *this;
    }

    bool operator==(const CAnimNameDescriptor& o) const {
        return strcmp(m_animName, o.m_animName) == 0 && strcmp(m_animGroupName, o.m_animGroupName) == 0;
    }

    bool operator!=(const CAnimNameDescriptor& o) const {
        return !(*this == o);
    }

    void Clear() {
        memset(m_animName, 0, sizeof(m_animName));
        memset(m_animGroupName, 0, sizeof(m_animGroupName));
    }

    bool IsEmpty() const { return m_animName[0] == '\0'; }

    const char* GetName() const { return m_animName; }
    const char* GetGroupName() const { return m_animGroupName; }

    static void InjectHooks();

private:
    CAnimNameDescriptor* Constructor(const char* animName, const char* animGroupName) {
        this->CAnimNameDescriptor::CAnimNameDescriptor(animName, animGroupName);
        return this;
    }

private:
    char m_animName[24]{};
    char m_animGroupName[16]{};
};
VALIDATE_SIZE(CAnimNameDescriptor, 0x28);
