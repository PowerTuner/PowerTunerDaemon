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
#ifdef __linux__
extern "C" {
#include <pci/pci.h>
}
#elifdef _WIN32
#include "../Include/CPUModel.h"
#endif
#include <cmath>

#include "IntelUtils.h"
#include "../../Utils/FileLogger/FileLogger.h"

namespace PWTD::Intel {
    using namespace Qt::StringLiterals;

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

#ifdef __linux__
    [[nodiscard]]
    static uint32_t getMCHBARBaseAddressLinux() {
        FileLogger &logger = FileLogger::get();
        struct pci_access *pacc = pci_alloc();
        struct pci_filter filter {};
        bool foundDev = false;
        struct pci_dev *dev;
        uint16_t hi, lo;

        if (pacc == nullptr) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(u"pci alloc fail"_s);

            return 0;
        }

        pci_init(pacc);
        pci_scan_bus(pacc);
        pci_filter_init(pacc, &filter);

        filter.domain = 0;
        filter.bus = 0;
        filter.slot = 0;
        filter.func = 0;

        for (dev=pacc->devices; dev; dev=dev->next) {
            if (pci_filter_match(&filter, dev)) {
                foundDev = true;
                break;
            }
        }

        if (!foundDev) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(u"no pci device found"_s);

            pci_cleanup(pacc);
            return 0;
        }

        hi = pci_read_word(dev, 0x4a);
        lo = pci_read_word(dev, 0x48);

        pci_cleanup(pacc);

        if (hi == 0) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(u"failed to read MCHBAR base address"_s);

            return 0;

        } else if (!(lo & 0x0001)) {
            if (logger.isLevel(PWTS::LogLevel::Info))
                logger.write(u"MCHBAR is disabled"_s);

            return 0;
        }

        return (hi << 16);
    }
#elifdef _WIN32
    // no way to get mchbar on new devices, also winring is to be removed, so...
    [[nodiscard]]
    static uint32_t getMCHBARBaseAddressWindows(const int cpuModel) {
        switch (cpuModel) {
            case SandyBridge:
            case IvyBridge:
            case IceLakeU:
                return 0xfed1;
            case TigerLakeU:
            case AlderLakeN:
            case LunarLake:
                return 0xfedc;
            default:
                return 0;
        }
    }
#endif

    uint32_t getMCHBARBaseAddress(const int cpuModel) {
#ifdef __linux__
        return getMCHBARBaseAddressLinux();
#elifdef _WIN32
        return getMCHBARBaseAddressWindows(cpuModel);
#else
        return 0;
#endif
    }
}
