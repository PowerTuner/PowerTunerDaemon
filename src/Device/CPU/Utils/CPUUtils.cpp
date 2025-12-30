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
#include <stdexcept>
#include <format>

#include "CPUUtils.h"

namespace PWTD {
    // from rdmsr: https://github.com/intel/msr-tools/blob/master/rdmsr.c
    static uint64_t _getBitfield(const unsigned maxBit, const unsigned highbit, const unsigned lowbit, uint64_t data) {
        const unsigned bits = highbit - lowbit + 1;

        if (highbit > maxBit || lowbit > highbit)
            throw std::invalid_argument(std::format("invalid usage: high {}, low {}, max bit {}", highbit, lowbit, maxBit));

        if (bits < (maxBit + 1)) {
            /* Show only part of register */
            data >>= lowbit;
            data &= (1ULL << bits) - 1;
        }

        return data;
    }

    static uint64_t _setBitfield(const unsigned maxBit, const unsigned highbit, const unsigned lowbit, const uint64_t value, uint64_t data) {
        const unsigned highDiscard = maxBit - highbit;
        uint64_t mask = 0xffffffffffffffff;
        uint64_t vmask;

        if (highbit > maxBit || lowbit > highbit)
            throw std::invalid_argument(std::format("invalid usage: high {}, low {}, max bit {}", highbit, lowbit, maxBit));

        mask = ~( (((mask >> lowbit) << lowbit) << highDiscard) >> highDiscard );
        vmask = ~mask;

        data &= mask;
        data |= (value << lowbit) & vmask; // paranoid, clear unneded bits

        return data;
    }

    uint64_t getBitfield(const unsigned highbit, const unsigned lowbit, const uint64_t data) {
        return _getBitfield(63, highbit, lowbit, data);
    }

    uint32_t getBitfield(const unsigned highbit, const unsigned lowbit, const uint32_t data) {
        return static_cast<uint32_t>(_getBitfield(31, highbit, lowbit, data));
    }

    void setBitfield(const unsigned highbit, const unsigned lowbit, const uint64_t value, uint64_t &data) {
        data = _setBitfield(63, highbit, lowbit, value, data);
    }

    void setBitfield(const unsigned highbit, const unsigned lowbit, const uint32_t value, uint32_t &data) {
        data = static_cast<uint32_t>(_setBitfield(31, highbit, lowbit, value, data));
    }
}
