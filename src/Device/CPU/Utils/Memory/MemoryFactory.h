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

#include <QSharedPointer>

#ifdef __linux__
#include "OS/Linux/MemoryLinux.h"
#elif defined(_WIN32)
#include "OS/Windows/MemoryWindows.h"
#else
#include "MemoryNull.h"
#endif

namespace PWTD {
    class MemoryFactory final {
    protected:
        inline static QSharedPointer<Memory> instance;

        MemoryFactory() = default;

    public:
        MemoryFactory(const MemoryFactory &) = delete;
        MemoryFactory &operator=(const MemoryFactory &) = delete;

        static QSharedPointer<Memory> getInstance() {
            if (!instance.isNull())
                return instance;

#ifdef __linux__
            instance = QSharedPointer<LNX::MemoryLinux>::create();
#elif defined(_WIN32)
            instance = QSharedPointer<WIN::MemoryWindows>::create();
#else
            instance = QSharedPointer<MemoryNull>::create();
#endif
            return instance;
        }
    };
}
