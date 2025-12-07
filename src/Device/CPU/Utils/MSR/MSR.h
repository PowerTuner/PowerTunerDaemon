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

#include <cstdint>

namespace PWTD {
    class MSR {
    public:
        virtual ~MSR() = default;

        [[nodiscard]] virtual bool openMsrFd(int cpu) = 0;
        [[nodiscard]] virtual bool readMSR(uint64_t &ret, uint32_t adr, int cpu) const = 0;
        [[nodiscard]] virtual bool readMSR(uint32_t &ret, uint32_t adr, int cpu) const = 0;
        [[nodiscard]] virtual bool writeMSR(uint64_t value, uint32_t adr, int cpu) const = 0;
        [[nodiscard]] virtual bool writeMSR(uint32_t value, uint32_t adr, int cpu) const = 0;
        virtual void closeMsrFd(int cpu) = 0;
    };
}
