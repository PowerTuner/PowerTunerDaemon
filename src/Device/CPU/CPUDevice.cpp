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
#include "CPUDevice.h"
#include "../../Utils/DaemonUtils.h"

namespace PWTD {
    CPUDevice::CPUDevice(const QSharedPointer<cpu_id_t> &cpuid, const QSharedPointer<cpu_raw_data_t> &cpuRawData) {
        logger = FileLogger::getInstance();
        cpuidRaw = cpuRawData;
        cpuInfo = QSharedPointer<PWTS::CpuInfo>::create();

        const int l1Data = cpuid->l1_data_cache > 0 ? (cpuid->l1_data_cache * 1024) : 0;
        const int l1Ins = cpuid->l1_instruction_cache > 0 ? (cpuid->l1_instruction_cache * 1024) : 0;
        const int l2 = cpuid->l2_cache > 0 ? (cpuid->l2_cache * 1024) : 0;
        const int l3 = cpuid->l3_cache > 0 ? (cpuid->l3_cache * 1024) : 0;
        const QString l1DSize = l1Data > 0 ? getMemorySizeStr(l1Data) : "0";
        const QString l1DTotSize = l1Data > 0 ? getMemorySizeStr(cpuid->num_cores * l1Data) : "0";
        const QString l1ISize = l1Ins > 0 ? getMemorySizeStr(l1Ins) : "0";
        const QString l1ITotSize = l1Ins > 0 ? getMemorySizeStr(cpuid->num_cores * l1Ins) : "0";
        const QString l2Size = l2 > 0 ? getMemorySizeStr(l2) : "0";
        const QString l2TotSize = l2 > 0 ? getMemorySizeStr(cpuid->num_cores * l2) : "0";
        const QString l3Size = l3 > 0 ? getMemorySizeStr(l3) : "0";

        cpuInfo->brand = QString(cpuid->brand_str).trimmed();
        cpuInfo->codename = cpuid->cpu_codename;
        cpuInfo->techNode = cpuid->technology_node;
        cpuInfo->vendor = PWTS::CPUVendor::Unknown;
        cpuInfo->vendorString = cpuid->vendor_str;
        cpuInfo->model = cpuid->x86.model;
        cpuInfo->extModel = cpuid->x86.ext_model;
        cpuInfo->family = cpuid->x86.family;
        cpuInfo->extFamily = cpuid->x86.ext_family;
        cpuInfo->numCores = cpuid->num_cores;
        cpuInfo->numLogicalCpus = cpuid->num_logical_cpus;
        cpuInfo->stepping = cpuid->x86.stepping;
        cpuInfo->l1DCache = QString("%1 %2x%3  %4-way").arg(l1DTotSize).arg(cpuid->num_cores).arg(l1DSize).arg(cpuid->l1_data_assoc);
        cpuInfo->l1ICache = QString("%1 %2x%3  %4-way").arg(l1ITotSize).arg(cpuid->num_cores).arg(l1ISize).arg(cpuid->l1_instruction_assoc);
        cpuInfo->l2Cache = QString("%1 %2x%3  %4-way").arg(l2TotSize).arg(cpuid->num_cores).arg(l2Size).arg(cpuid->l2_assoc);
        cpuInfo->l3Cache = QString("%1  %2-way").arg(l3Size).arg(cpuid->l3_assoc);

        if (cpuid->l4_cache > 0) {
            const int l4 = cpuid->l4_cache > 0 ? (cpuid->l4_cache * 1024) : 0;
            const QString l4Size = l4 > 0 ? getMemorySizeStr(l4) : "0";

            cpuInfo->l4Cache = QString("%1  %2-way").arg(l4Size).arg(cpuid->l4_assoc);
        }
    }
}
