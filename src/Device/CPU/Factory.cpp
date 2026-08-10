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
#include "Factory.h"
#ifdef WITH_INTEL
#include "Intel/IntelCPU.h"
#endif
#ifdef WITH_AMD
#include "AMD/AMDCPU.h"
#endif

namespace PWTD::CPU {
    std::unique_ptr<CPUDevice> factory() {
        if (!cpuid_present())
            return {};

        const std::shared_ptr<cpu_id_t> cpuID = std::make_shared<cpu_id_t>();
        const std::shared_ptr<cpu_raw_data_t> rawData = std::make_shared<cpu_raw_data_t>();

        if (cpuid_get_raw_data(rawData.get()) < 0 || cpu_identify(rawData.get(), cpuID.get()) < 0)
            return {};

        switch (cpuID->vendor) {
#ifdef WITH_INTEL
            case VENDOR_INTEL:
                return std::make_unique<Intel::IntelCPU>(cpuID, rawData);
#endif
#ifdef WITH_AMD
            case VENDOR_AMD:
                return std::make_unique<AMD::AMDCPU>(cpuID, rawData);
#endif
            default:
                break;
        }

        return {};
    }
}
