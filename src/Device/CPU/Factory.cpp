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
    std::unique_ptr<CPUDevice> factory(LIBPWT::DeviceData &devData) {
        cpu_raw_data_t rawData = {};
        cpu_id_t cpuID = {};

        if (!cpuid_present() || cpuid_get_raw_data(&rawData) < 0 || cpu_identify(&rawData, &cpuID) < 0)
            return {};

        const int l1Data = cpuID.l1_data_cache > 0 ? (cpuID.l1_data_cache * 1024) : 0;
        const int l1Ins = cpuID.l1_instruction_cache > 0 ? (cpuID.l1_instruction_cache * 1024) : 0;
        const int l2 = cpuID.l2_cache > 0 ? (cpuID.l2_cache * 1024) : 0;
        const int l3 = cpuID.l3_cache > 0 ? (cpuID.l3_cache * 1024) : 0;
        const int l4 = cpuID.l4_cache > 0 ? (cpuID.l4_cache * 1024) : 0;
        const std::string l1DSize = l1Data > 0 ? getMemorySizeStr(l1Data) : "0";
        const std::string l1DTotSize = l1Data > 0 ? getMemorySizeStr(cpuID.num_cores * l1Data) : "0";
        const std::string l1ISize = l1Ins > 0 ? getMemorySizeStr(l1Ins) : "0";
        const std::string l1ITotSize = l1Ins > 0 ? getMemorySizeStr(cpuID.num_cores * l1Ins) : "0";
        const std::string l2Size = l2 > 0 ? getMemorySizeStr(l2) : "0";
        const std::string l2TotSize = l2 > 0 ? getMemorySizeStr(cpuID.num_cores * l2) : "0";
        const std::string l3Size = l3 > 0 ? getMemorySizeStr(l3) : "0";
        const std::string l4Size = l4 > 0 ? getMemorySizeStr(l4) : "0";

        devData.cpuInfo = LIBPWT::CPUInfo{
            .brand = cpuID.brand_str,
            .codename = cpuID.cpu_codename,
            .techNode = cpuID.technology_node,
            .CPUString = cpuID.vendor_str,
            .model = cpuID.x86.model,
            .extModel = cpuID.x86.ext_model,
            .family = cpuID.x86.family,
            .extFamily = cpuID.x86.ext_family,
            .cores = cpuID.num_cores,
            .threads = cpuID.num_logical_cpus,
            .stepping = cpuID.x86.stepping,
            .l1DCache = std::format("{} {}x{}  {}-way", l1DTotSize, cpuID.num_cores, l1DSize, cpuID.l1_data_assoc),
            .l1ICache = std::format("{} {}x{}  {}-way", l1ITotSize, cpuID.num_cores, l1ISize, cpuID.l1_instruction_assoc),
            .l2Cache = std::format("{} {}x{}  {}-way", l2TotSize, cpuID.num_cores, l2Size, cpuID.l2_assoc),
            .l3Cache = std::format("{}  {}-way", l3Size, cpuID.l3_assoc),
            .l4Cache = l4 > 0 ? std::format("{}  {}-way", l4Size, cpuID.l4_assoc) : ""
        };

        switch (cpuID.vendor) {
#ifdef WITH_INTEL
            case VENDOR_INTEL:
                return std::make_unique<Intel::IntelCPU>(rawData, devData);
#endif
#ifdef WITH_AMD
            case VENDOR_AMD:
                return std::make_unique<AMD::AMDCPU>(rawData, devData);
#endif
            default:
                break;
        }

        return {};
    }
}
