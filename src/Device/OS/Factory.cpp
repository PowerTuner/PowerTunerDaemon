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
#include "Factory.h"
#ifdef __linux__
#include "Linux/OSLinux.h"
#elifdef _WIN32
#include "Windows/OSWindows.h"
#endif

namespace PWTD::Sys {
    std::shared_ptr<OS> factory() {
#ifdef __linux__
        return std::make_shared<OSLinux>();
#elifdef _WIN32
        return std::make_shared<OSWindows>();
#else
        return {};
#endif
    }
}
