#pragma once
#include <cassert>
#include "app_debug.h"
#include "AudioEngine.h"
#include "Common.h"
#include "FileMgr.h"
#include "GenericGameStorage.h"
#include "MenuManager.h"

#include "extensions/Configuration.hpp"

inline struct MiscConfig {
    INI_CONFIG_SECTION("Misc");

    std::optional<std::string> SaveDirectoryPath{};

    //! Defaults to `true` because the original game doesn't version-check saves at all - see the
    //! `SIMPLE_VARIABLES` block in `CGenericGameStorage::GenericLoad`.
    bool LoadSavesWithMismatchingVersion{ true };

    void Load() {
        STORE_INI_CONFIG_VALUE_OPT(SaveDirectoryPath);

        if (SaveDirectoryPath.has_value() && SaveDirectoryPath->size() >= 255) {
            SaveDirectoryPath->resize(255);
            NOTSA_LOG_WARN("Custom save directory path is too long! Truncated to: '{}'", *SaveDirectoryPath);
        }

        STORE_INI_CONFIG_VALUE(LoadSavesWithMismatchingVersion, true);
    }
} g_MiscConfig{};
