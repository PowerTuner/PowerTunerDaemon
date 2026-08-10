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
#include <optional>
#include <utility>

#include "../../../Memory/Memory.h"

namespace PWTD::MEM {
    class MemoryLinux final: public Memory {
    private:
        [[nodiscard]] std::optional<std::pair<unsigned char *, size_t>> mapMemory(uint64_t addr, size_t size, int memFlags, int mapFlags) const;
        [[nodiscard]] std::optional<uint64_t> read(uint64_t addr, size_t size) const;
        [[nodiscard]] bool write(uint64_t data, uint64_t addr, size_t size) const;

    public:
        [[nodiscard]] bool forbidden() const override;
        [[nodiscard]] bool read8(uint64_t addr, uint8_t &out) const override;
        [[nodiscard]] bool read16(uint64_t addr, uint16_t &out) const override;
        [[nodiscard]] bool read32(uint64_t addr, uint32_t &out) const override;
        [[nodiscard]] bool read64(uint64_t addr, uint64_t &out) const override;
        [[nodiscard]] bool write8(uint8_t val, uint64_t addr) const override;
        [[nodiscard]] bool write16(uint16_t val, uint64_t addr) const override;
        [[nodiscard]] bool write32(uint32_t val, uint64_t addr) const override;
        [[nodiscard]] bool write64(uint64_t val, uint64_t addr) const override;
    };
}
