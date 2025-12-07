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
#include "pwtShared/Include/Types/ROData.h"
#include "pwtShared/Include/Types/RWData.h"

namespace PWTD::Intel {
    class MSR_UNK_FIVR_CONTROL: public CPURegister {
    private:
        struct planeIndex final {
            int cpu;
            int gpu;
            int cpuCache;
            int unslice;
            int sysAgent;
        };

        struct uvData final {
            uint64_t rd = 0;
            uint64_t wr = 0;

            uvData(const uint64_t r, const uint64_t w): rd(r), wr(w) {}
        };

        // from intel-undervolt: https://github.com/kitsunyan/intel-undervolt, not sure if correct but kinda works
        [[nodiscard]]
        uvData prepareUVData(const int index, const int value) const {
            const uint64_t uvint = (static_cast<uint64_t>(0x800 - (value < 0 ? -(value) : value) * 1.024f + 0.5f) << 21) & 0xffffffff;
            const uint64_t rdval = 0x8000001000000000 | (static_cast<uint64_t>(index) << 40);
            const uint64_t wrval = rdval | 0x100000000 | uvint;

            return {rdval, wrval};
        }

        // from intel-undervolt: https://github.com/kitsunyan/intel-undervolt
        [[nodiscard]]
        bool writeUnderVolt(const uvData &data) const {
            uint64_t cur = 0;

            if (!msrUtils->writeMSR(data.wr, addr, 0) || !msrUtils->writeMSR(data.rd, addr, 0) || !msrUtils->readMSR(cur, addr, 0))
                return false;

            return (cur & 0xffffffff) == (data.wr & 0xffffffff);
        }

    protected:
        planeIndex indexes {};

    public:
        struct [[nodiscard]] FIVRWriteResult final {
            bool cpu = false;
            bool gpu = false;
            bool cpuCache = false;
            bool unslice = false;
            bool sysAgent = false;
        };

        struct [[nodiscard]] FIVRCapabilities final {
            bool cpu = false;
            bool gpu = false;
            bool cpuCache = false;
            bool unslice = false;
            bool sysAgent = false;
        };

        MSR_UNK_FIVR_CONTROL() {
            addr = 0x150;
        }

        PWTS::ROData<FIVRCapabilities> getFIVRCapabilities() const {
            return PWTS::ROData<FIVRCapabilities>({
                .cpu = indexes.cpu != -1,
                .gpu = indexes.gpu != -1,
                .cpuCache = indexes.cpuCache != -1,
                .unslice = indexes.unslice != -1,
                .sysAgent = indexes.sysAgent != -1
            }, true);
        }

        FIVRWriteResult setFIVRControl(const PWTS::RWData<PWTS::Intel::FIVRControlUV> &data) const {
            if (!data.isValid())
                return {};

            const PWTS::Intel::FIVRControlUV uv = data.getValue();
            FIVRWriteResult res;

            if (indexes.cpu != -1)
                res.cpu = writeUnderVolt(prepareUVData(indexes.cpu, uv.cpu));

            if (indexes.gpu != -1)
                res.gpu = writeUnderVolt(prepareUVData(indexes.gpu, uv.gpu));

            if (indexes.cpuCache != -1)
                res.cpuCache = writeUnderVolt(prepareUVData(indexes.cpuCache, uv.cpuCache));

            if (indexes.sysAgent != -1)
                res.sysAgent = writeUnderVolt(prepareUVData(indexes.sysAgent, uv.sa));

            if (indexes.unslice != -1)
                res.unslice = writeUnderVolt(prepareUVData(indexes.unslice, uv.unslice));

            return res;
        }
    };
}
