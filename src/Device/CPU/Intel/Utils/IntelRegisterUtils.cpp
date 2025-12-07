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

#include "IntelRegisterUtils.h"

namespace PWTD::Intel {
    // from https://patchwork.kernel.org/project/xen-devel/patch/20210308210210.116278-13-jandryuk@gmail.com
    HWPActivityWindowBits getHWPActivityWindowBitsFromMicroSecond(int us) {
        HWPActivityWindowBits acwBits {};

        acwBits.exponent = 0;

        /* looking for 7 bits of mantissa and 3 bits of exponent */
        while (us > 127) {
            us /= 10;
            ++acwBits.exponent;
        }

        acwBits.exponent &= 0x7;
        acwBits.mantissa = us & 0x7f;

        return acwBits;
    }

    PowerLimitRawTimeWindow getRawTimeWindow(const float seconds, const double timeUnit) {
        if (seconds == 0)
            return {.y = 0, .z = 0};

        const double raw = seconds / timeUnit;
        PowerLimitRawTimeWindow rtm {};

        for (int z=0; z<=3; ++z) {
            const double lhsMultiplier = 1.f + (static_cast<float>(z) / 4.f);
            const double y = std::log2(raw / lhsMultiplier); // log rhs

            if ((std::pow(2, y) * lhsMultiplier) == raw) {
                rtm.y = static_cast<int>(y);
                rtm.z = z;
                break;
            }
        }

        return rtm;
    }
}
