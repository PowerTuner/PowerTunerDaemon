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
#include "MCHBARUtils.h"
#include "../../../../Utils/FileLogger/FileLogger.h"
#ifdef __linux__
extern "C" {
#include <pci/pci.h>
}
#elif defined(_WIN32)
#include "pwtWin32/win.h"
#include "../../../../external/winring0/OlsApi.h"
#include "../../../../external/winring0/OlsDef.h"
#include "../Include/CPUModel.h"
#endif

namespace PWTD::Intel {
#ifdef __linux__
    [[nodiscard]]
    static uint32_t getMCHBARBaseAddressLinux() {
        const QSharedPointer<FileLogger> logger = FileLogger::getInstance();
        struct pci_access *pacc = pci_alloc();
        struct pci_filter filter {};
        bool foundDev = false;
        struct pci_dev *dev;
        uint16_t hi, lo;

        if (pacc == nullptr) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("pci alloc fail"));

            return -1;
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
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("no pci device found"));

            pci_cleanup(pacc);
            return -1;
        }

        hi = pci_read_word(dev, 0x4a);
        lo = pci_read_word(dev, 0x48);

        pci_cleanup(pacc);

        if (hi == 0) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("failed to read MCHBAR base address"));

            return -1;

        } else if (!(lo & 0x0001)) {
            if (logger->isLevel(PWTS::LogLevel::Info))
                logger->write(QStringLiteral("MCHBAR is disabled"));

            return -1;
        }

        return (hi << 16);
    }
#elif defined(_WIN32)
    // (hopefully temp)
    // workaround for devices that can't get base addr due to SECURE_PCI_CONFIG_SPACE_ACCESS_VIOLATION
    [[nodiscard]]
    static uint32_t getMCHBARBaseAddressWindowsWk(const int cpuFamily) {
        switch (cpuFamily) {
            case LunarLake:
                return 0xfedc;
            default:
                break;
        }

        return 0;
    }

    [[nodiscard]]
    static uint32_t getMCHBARBaseAddressWindows(const int cpuFamily) {
        const uint32_t wkAddr = getMCHBARBaseAddressWindowsWk(cpuFamily);

        if (wkAddr != 0)
            return wkAddr;

        const QSharedPointer<FileLogger> logger = FileLogger::getInstance();
        const DWORD dev = PciBusDevFunc(0, 0, 0);
        const DWORD ringDLLStatus = GetDllStatus();
        WORD hi, lo;

        if (ringDLLStatus != OLS_DLL_NO_ERROR) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("winring dll error, code: %1").arg(ringDLLStatus));

            return -1;
        }

        if (ReadPciConfigWordEx(dev, 0x4a, &hi) == FALSE || ReadPciConfigWordEx(dev, 0x48, &lo) == FALSE) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("failed to read MCHBAR"));

            return -1;
        }

        if (!(lo & 0x0001)) {
            if (logger->isLevel(PWTS::LogLevel::Info))
                logger->write(QStringLiteral("MCHBAR is disabled"));

            return -1;
        }

        return (hi << 16);
    }
#endif

    uint32_t getMCHBARBaseAddress(const int cpuFamily) {
#ifdef __linux__
        return getMCHBARBaseAddressLinux();
#elif defined(_WIN32)
        return getMCHBARBaseAddressWindows(cpuFamily);
#else
        return -1;
#endif
    }
}
