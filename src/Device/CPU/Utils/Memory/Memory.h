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

namespace PWTD::MEM {
    class Memory {
    public:
        virtual ~Memory() = default;

        [[nodiscard]] virtual bool forbidden() const { return true; }
        [[nodiscard]] virtual bool read8(const uint64_t addr, uint8_t &out) const { return false; }
        [[nodiscard]] virtual bool read16(const uint64_t addr, uint16_t &out) const { return false; }
        [[nodiscard]] virtual bool read32(const uint64_t addr, uint32_t &out) const { return false; }
        [[nodiscard]] virtual bool read64(const uint64_t addr, uint64_t &out) const { return false; }
        [[nodiscard]] virtual bool write8(const uint8_t val, const uint64_t addr) const { return false; }
        [[nodiscard]] virtual bool write16(const uint16_t val, const uint64_t addr) const { return false; }
        [[nodiscard]] virtual bool write32(const uint32_t val, const uint64_t addr) const { return false; }
        [[nodiscard]] virtual bool write64(const uint64_t val, const uint64_t addr) const { return false; }
    };
}
