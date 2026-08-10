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
#include <libkmod.h>
#include <fcntl.h>
#include <unistd.h>
#include <format>

#include "MSRLinux.h"

namespace PWTD::MSR {
    void MSRLinux::addSlotForCPU(const int cpu) {
        if (fdMap.contains(cpu))
            return;

        fdMap[cpu] = {.fd = -1, .count = 0};
    }

    bool MSRLinux::loadMsrModule() {
        if (moduleLoaded)
            return true;

        struct kmod_module *mod = nullptr;
        struct kmod_ctx *ctx = kmod_new(nullptr, nullptr);
        int ret;

        if (ctx == nullptr)
            return false;

        ret = kmod_module_new_from_name_lookup(ctx, "msr", &mod);
        if (ret < 0) {
            kmod_unref(ctx);
            return false;
        }

        ret = kmod_module_probe_insert_module(mod, KMOD_PROBE_FAIL_ON_LOADED, nullptr, nullptr, nullptr, nullptr);
        moduleLoaded = !(ret < 0 && ret != -EEXIST);

        kmod_module_unref(mod);
        kmod_unref(ctx);
        return moduleLoaded;
    }

    bool MSRLinux::openFd(const int cpu) {
        if (!loadMsrModule())
            return false;

        addSlotForCPU(cpu);

        msrFD &msr = fdMap[cpu];

        if (msr.fd >= 0) {
            ++msr.count;
            return true;
        }

        const std::string path = std::format("/dev/cpu/{}/msr", cpu);

        msr.fd = open(path.c_str(), O_RDWR | O_SYNC);

        if (msr.fd >= 0) {
            ++msr.count;
            return true;
        }

        return false;
    }

    void MSRLinux::closeFd(const int cpu) {
        msrFD &msr = fdMap[cpu];

        if (msr.count == 0) [[unlikely]]
            return;

        --msr.count;

        if (msr.count > 0)
            return;

        close(msr.fd);

        msr.fd = -1;
        msr.count = 0;
    }

    bool MSRLinux::read(const uint32_t adr, const int cpu, uint64_t &out) const {
        constexpr size_t outSz = sizeof(uint64_t);
        const msrFD &msr = fdMap.at(cpu);

        if (msr.count == 0 || pread(msr.fd, &out, outSz, adr) != outSz) [[unlikely]]
            return false;

        return pread(msr.fd, &out, outSz, adr) == outSz;
    }

    bool MSRLinux::write(const uint64_t value, const uint32_t adr, const int cpu) const {
        constexpr size_t valueSz = sizeof(uint64_t);
        const msrFD &msr = fdMap.at(cpu);

        if (msr.count == 0) [[unlikely]]
            return false;

        return pwrite(msr.fd, &value, valueSz, adr) == valueSz;
    }
}
