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
#include <QString>
#include <array>

#include "Utils.h"

namespace PWTD {
    QString getMemorySizeStr(const quint64 size) {
        static constexpr std::array<std::string_view, 6> memUnits {"Bytes", "KB", "MB", "GB", "PB", "TB"};
        double sz = static_cast<double>(size);
        auto unitIt = memUnits.cbegin();

        if (size == 0)
            return "";

        while (static_cast<quint64>(sz / 1024) > 0) {
            sz /= 1024;
            ++unitIt;

            if (unitIt == memUnits.cend())
                return "Unknown";
        }

        return QString("%1 %2").arg(QString::number(sz, 'g', 3), unitIt->data());
    }

    // from rdmsr: https://github.com/intel/msr-tools/blob/master/rdmsr.c
    static uint64_t _getBitfield(const unsigned size, const unsigned high, const unsigned low, uint64_t data) {
        const unsigned bits = high - low + 1;

        if (bits < size) {
            /* Show only part of register */
            data >>= low;
            data &= (1ULL << bits) - 1;
        }

        return data;
    }

    uint64_t getBitfield(const unsigned high, const unsigned low, const uint64_t data) {
        return _getBitfield(64, high, low, data);
    }

    uint32_t getBitfield(const unsigned high, const unsigned low, const uint32_t data) {
        return static_cast<uint32_t>(_getBitfield(32, high, low, data));
    }

    // from rdmsr: https://github.com/intel/msr-tools/blob/master/rdmsr.c
    static uint64_t _setBitfield(const unsigned size, const unsigned high, const unsigned low, const uint64_t value, uint64_t data) {
        const unsigned highDiscard = (size - 1) - high;
        const uint64_t mask = ~( (((0xffffffffffffffffULL >> low) << low) << highDiscard) >> highDiscard );

        data &= mask;
        data |= (value << low) & (~mask); // paranoid, clear unneded bits

        return data;
    }

    uint64_t setBitfield(const unsigned high, const unsigned low, const uint64_t value, const uint64_t data) {
        return _setBitfield(64, high, low, value, data);
    }

    uint32_t setBitfield(const unsigned high, const unsigned low, const uint32_t value, const uint32_t data) {
        return static_cast<uint32_t>(_setBitfield(32, high, low, value, data));
    }
}
