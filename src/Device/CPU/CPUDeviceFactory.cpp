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
#include "CPUDeviceFactory.h"
#ifdef WITH_INTEL
#include "Intel/IntelCPU.h"
#endif
#ifdef WITH_AMD
#include "AMD/AMDCPU.h"
#endif

namespace PWTD {
    QSharedPointer<CPUDevice> CPUDeviceFactory::getCpuDevice() {
        if (!cpuid_present())
            return {};

        const QSharedPointer<cpu_raw_data_t> rawData = QSharedPointer<cpu_raw_data_t>::create();
        const QSharedPointer<cpu_id_t> cpuID = QSharedPointer<cpu_id_t>::create();

        if (cpuid_get_raw_data(rawData.get()) < 0 || cpu_identify(rawData.get(), cpuID.get()) < 0)
            return {};

        switch (cpuID->vendor) {
#ifdef WITH_INTEL
            case VENDOR_INTEL:
                return QSharedPointer<Intel::IntelCPU>::create(cpuID, rawData);
#endif
#ifdef WITH_AMD
            case VENDOR_AMD:
                return QSharedPointer<AMD::AMDCPU>::create(cpuID, rawData);
#endif
            default:
                break;
        }

        return {};
    }
}
