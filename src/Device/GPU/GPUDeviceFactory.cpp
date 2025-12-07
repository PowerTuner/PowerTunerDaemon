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
#include "GPUDeviceFactory.h"
#include "AMD/AMDGPU.h"
#include "Intel/IntelGPU.h"
#include "NVIDIA/NVIDIAGPU.h"

namespace PWTD {
    QSharedPointer<GPUDevice> GPUDeviceFactory::getGPUDevice(const int index, const QSharedPointer<OS> &os) {
        switch (os->getGPUVendor(index)) {
            case PWTS::GPUVendor::AMD:
                return QSharedPointer<AMD::AMDGPU>::create(index, os);
            case PWTS::GPUVendor::Intel:
                return QSharedPointer<Intel::IntelGPU>::create(index, os);
            case PWTS::GPUVendor::NVIDIA:
                return QSharedPointer<NVIDIA::NVIDIAGPU>::create(index, os);
            default:
                break;
        }

        return {};
    }
}
