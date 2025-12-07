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
    class Memory {
    public:
        virtual ~Memory() = default;

        [[nodiscard]] virtual bool isAccessible() const = 0;
        [[nodiscard]] virtual bool readMem8(uint64_t &val, uint64_t addr) const = 0;
        [[nodiscard]] virtual bool readMem16(uint64_t &val, uint64_t addr) const = 0;
        [[nodiscard]] virtual bool readMem32(uint64_t &val, uint64_t addr) const = 0;
        [[nodiscard]] virtual bool readMem64(uint64_t &val, uint64_t addr) const = 0;
        [[nodiscard]] virtual bool writeMem8(uint64_t &val, uint64_t addr) const = 0;
        [[nodiscard]] virtual bool writeMem16(uint64_t &val, uint64_t addr) const = 0;
        [[nodiscard]] virtual bool writeMem32(uint64_t &val, uint64_t addr) const = 0;
        [[nodiscard]] virtual bool writeMem64(uint64_t &val, uint64_t addr) const = 0;
    };
}
