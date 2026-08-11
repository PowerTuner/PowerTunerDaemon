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
#include "AMD/AMDGPU.h"
#include "Intel/IntelGPU.h"
#include "NVIDIA/NVIDIAGPU.h"

namespace PWTD::GPU {
    std::shared_ptr<GPUDevice> factory(const int index, const std::shared_ptr<OS> &os) {
        switch (os->getGPUVendor(index)) {
            case PWTS::GPUVendor::AMD:
                return std::make_shared<AMD::AMDGPU>(index, os);
            case PWTS::GPUVendor::Intel:
                return std::make_shared<Intel::IntelGPU>(index, os);
            case PWTS::GPUVendor::NVIDIA:
                return std::make_shared<NVIDIA::NVIDIAGPU>(index, os);
            default:
                return {};
        }
    }
}
