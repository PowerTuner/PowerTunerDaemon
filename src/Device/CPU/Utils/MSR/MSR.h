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

namespace PWTD::MSR {
    class MSR {
    public:
        virtual ~MSR() = default;

        [[nodiscard]] virtual bool openFd(const int cpu) { return false; }
        [[nodiscard]] virtual bool read(const uint32_t adr, const int cpu, uint64_t &out) const { return false; }
        [[nodiscard]] virtual bool write(const uint64_t value, const uint32_t adr, const int cpu) const { return false; }
        virtual void closeFd(const int cpu) {}
    };
}
