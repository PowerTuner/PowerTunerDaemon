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

#include "Memory.h"

namespace PWTD {
    class MemoryNull final: public Memory {
    public:
        [[nodiscard]] bool isAccessible() const override { return false; }
        [[nodiscard]] bool readMem8(uint64_t &val, const uint64_t addr) const override { return false; }
        [[nodiscard]] bool readMem16(uint64_t &val, const uint64_t addr) const override { return false; }
        [[nodiscard]] bool readMem32(uint64_t &val, const uint64_t addr) const override { return false; }
        [[nodiscard]] bool readMem64(uint64_t &val, const uint64_t addr) const override { return false; }
        [[nodiscard]] bool writeMem8(uint64_t &val, const uint64_t addr) const override { return false; }
        [[nodiscard]] bool writeMem16(uint64_t &val, const uint64_t addr) const override { return false; }
        [[nodiscard]] bool writeMem32(uint64_t &val, const uint64_t addr) const override { return false; }
        [[nodiscard]] bool writeMem64(uint64_t &val, const uint64_t addr) const override { return false; }
    };
}
