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
#pragma once
#include "../../../CPURegister.h"
#include "pwtShared/Include/CPU/Intel/FIVRControlUV.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class MSR_UNK_FIVR_CONTROL: public CPURegister {
    private:
        static constexpr unsigned addr = 0x150;

        // from intel-undervolt: https://github.com/kitsunyan/intel-undervolt, not sure if correct but kinda works
        [[nodiscard]]
        std::pair<uint64_t, uint64_t> prepareUVData(const int index, const int value) const {
            const uint64_t uvint = (static_cast<uint64_t>(0x800 - (value < 0 ? -(value) : value) * 1.024f + 0.5f) << 21) & 0xffffffff;
            const uint64_t rdval = 0x8000001000000000 | (static_cast<uint64_t>(index) << 40);
            const uint64_t wrval = rdval | 0x100000000 | uvint;

            return std::make_pair(rdval, wrval);
        }

        // from intel-undervolt: https://github.com/kitsunyan/intel-undervolt
        [[nodiscard]]
        bool writeUnderVolt(const std::pair<uint64_t, uint64_t> &uv) const {
            const auto [rd, wr] = uv;
            uint64_t reg = 0;

            if (!msrDev->write(wr, addr, 0) || !msrDev->write(rd, addr, 0) || !msrDev->read(addr, 0, reg))
                return false;

            return (reg & 0xffffffff) == (wr & 0xffffffff);
        }

    protected:
        int cpuIdx = -1;
        int gpuIdx = -1;
        int cpuCacheIdx = -1;
        int unsliceIdx = -1;
        int sysAgentIdx = -1;

    public:
        [[nodiscard]] bool hasCPU() const { return cpuIdx != -1; }
        [[nodiscard]] bool hasGPU() const { return gpuIdx != -1; }
        [[nodiscard]] bool hasCPUCache() const { return cpuCacheIdx != -1; }
        [[nodiscard]] bool hasUnslice() const { return unsliceIdx != -1; }
        [[nodiscard]] bool hasSysAgent() const { return sysAgentIdx != -1; }

        [[nodiscard]]
        bool setCPU(const PWTS::RWData<PWTS::Intel::FIVRControlUV> &data) const {
            if (!data.isValid() || cpuIdx == -1)
                return false;

            return writeUnderVolt(prepareUVData(cpuIdx, data.getValue().cpu));
        }

        [[nodiscard]]
        bool setGPU(const PWTS::RWData<PWTS::Intel::FIVRControlUV> &data) const {
            if (!data.isValid() || gpuIdx == -1)
                return false;

            return writeUnderVolt(prepareUVData(gpuIdx, data.getValue().gpu));
        }

        [[nodiscard]]
        bool setCPUCache(const PWTS::RWData<PWTS::Intel::FIVRControlUV> &data) const {
            if (!data.isValid() || cpuCacheIdx == -1)
                return false;

            return writeUnderVolt(prepareUVData(cpuCacheIdx, data.getValue().cpuCache));
        }

        [[nodiscard]]
        bool setUnslice(const PWTS::RWData<PWTS::Intel::FIVRControlUV> &data) const {
            if (!data.isValid() || unsliceIdx == -1)
                return false;

            return writeUnderVolt(prepareUVData(unsliceIdx, data.getValue().unslice));
        }

        [[nodiscard]]
        bool setSysAgent(const PWTS::RWData<PWTS::Intel::FIVRControlUV> &data) const {
            if (!data.isValid() || sysAgentIdx == -1)
                return false;

            return writeUnderVolt(prepareUVData(sysAgentIdx, data.getValue().sa));
        }
    };
}
