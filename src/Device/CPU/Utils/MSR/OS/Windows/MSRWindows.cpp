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
#include "pwtWin32/win.h"
#include "libryzenadj/win32/OlsDef.h"
#include "libryzenadj/win32/OlsApi.h"
#include "MSRWindows.h"

namespace PWTD::WIN {
    bool MSRWindows::openMsrFd([[maybe_unused]] const int cpu) {
        isDriverOpen = GetDllStatus() == OLS_DLL_NO_ERROR && IsMsr();

        return isDriverOpen;
    }

    void MSRWindows::closeMsrFd([[maybe_unused]] const int cpu) {}

    bool MSRWindows::readMSR(uint32_t &ret, const uint32_t adr, const int cpu) const {
        if (!isDriverOpen)
            return false;

        DWORD eax = 0;
        DWORD edx = 0;

        if (!RdmsrTx(adr, &eax, &edx, 1ULL << cpu))
            return false;

        ret = eax;
        return true;
    }

    bool MSRWindows::readMSR(uint64_t &ret, const uint32_t adr, const int cpu) const {
        if (!isDriverOpen)
            return false;

        DWORD eax = 0;
        DWORD edx = 0;

        if (!RdmsrTx(adr, &eax, &edx, 1ULL << cpu))
            return false;

        ret = (static_cast<uint64_t>(edx) << 32) | eax;
        return true;
    }

    bool MSRWindows::writeMSR(const uint32_t value, const uint32_t adr, const int cpu) const {
        return isDriverOpen && WrmsrTx(adr, value, 0, 1ULL << cpu);
    }

    bool MSRWindows::writeMSR(const uint64_t value, const uint32_t adr, const int cpu) const {
        return isDriverOpen && WrmsrTx(adr, value & 0x00000000ffffffff, value >> 32, 1ULL << cpu);
    }
}
