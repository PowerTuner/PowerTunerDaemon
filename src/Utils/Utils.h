/*
 * This file is part of PowerTunerDaemon.
 * Copyright (C) 2025 kylon
 *
 * PowerTunerDaemon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PowerTunerDaemon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include "pwtShared/Include/OSType.h"

namespace PWTD {
    [[nodiscard]] consteval PWTS::OSType getOS() {
#ifdef __linux__
        return PWTS::OSType::Linux;
#elifdef _WIN32
        return PWTS::OSType::Windows;
#else
        return PWTS::OSType::Unknown;
#endif
    }

    [[nodiscard]] consteval bool isLinux() { return getOS() == PWTS::OSType::Linux; }
    [[nodiscard]] consteval bool isWindows() { return getOS() == PWTS::OSType::Windows; }
    [[nodiscard]] consteval bool isUnknownOS() { return getOS() == PWTS::OSType::Unknown; }

    [[nodiscard]] QString getMemorySizeStr(quint64 size);
    [[nodiscard]] uint64_t getBitfield(unsigned high, unsigned low, uint64_t data);
    [[nodiscard]] uint32_t getBitfield(unsigned high, unsigned low, uint32_t data);
    [[nodiscard]] uint64_t setBitfield(unsigned high, unsigned low, uint64_t value, uint64_t data);
    [[nodiscard]] uint32_t setBitfield(unsigned high, unsigned low, uint32_t value, uint32_t data);
}
