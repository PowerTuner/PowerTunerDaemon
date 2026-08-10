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
#include <memory>

#ifdef __linux__
#include "OS/Linux/MSRLinux.h"
#elifdef _WIN32
#include "OS/Windows/MSRWindows.h"
#endif

namespace PWTD::MSR {
    [[nodiscard]]
    inline std::shared_ptr<MSR> factory() {
#ifdef __linux__
        static std::shared_ptr<MSR> instance = std::make_shared<MSRLinux>();
#elifdef _WIN32
        static std::shared_ptr<MSR> instance = std::make_shared<MSRWindows>();
#else
        static std::shared_ptr<MSR> instance = std::make_shared<MSR>();
#endif
        return instance;
    }
}