#pragma once

enum class eWantedLevel : uint32 {
    WANTED_CLEAN = 0,
    WANTED_LEVEL_1,
    WANTED_LEVEL_2,
    WANTED_LEVEL_3,
    WANTED_LEVEL_4,
    WANTED_LEVEL_5,
    WANTED_LEVEL_6
};

//! Scoped and fixed-underlying-type enums are not implicitly formattable, so give
//! fmt a conversion for the `NOTSA_LOG_*` call sites that print one.
inline auto format_as(eWantedLevel e) { return static_cast<std::underlying_type_t<eWantedLevel>>(e); }
