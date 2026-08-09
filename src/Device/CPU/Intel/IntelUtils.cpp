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
#include <cmath>

#include "IntelUtils.h"

namespace PWTD::Intel {
    // from https://patchwork.kernel.org/project/xen-devel/patch/20210308210210.116278-13-jandryuk@gmail.com
    std::pair<int, int> USecToRawHWPActivityWindow(int us) {
        int exponent = 0;
        int mantissa = 0;

        /* looking for 7 bits of mantissa and 3 bits of exponent */
        while (us > 127) {
            us /= 10;
            ++exponent;
        }

        exponent &= 0x7;
        mantissa = us & 0x7f;

        return std::make_pair(exponent, mantissa);
    }

    std::optional<std::pair<int, int>> getRawTimeWindow(const double seconds, const double timeUnit) {
        if (seconds == 0)
            return {};

        const double raw = seconds / timeUnit;

        for (int z=0; z<=3; ++z) {
            const double lhsMultiplier = 1 + (static_cast<double>(z) / 4.0);
            const double y = std::log2(raw / lhsMultiplier); // log rhs

            if ((std::pow(2, y) * lhsMultiplier) == raw)
                return std::make_pair(static_cast<int>(y), z);
        }

        return {};
    }
}
