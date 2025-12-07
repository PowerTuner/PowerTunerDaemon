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

#include "MSRLinux.h"

namespace PWTD::LNX {
    void MSRLinux::addSlotForCPU(const int cpu) {
        if (msrFDMap.contains(cpu))
            return;

        msrFDMap.insert(cpu, {.fd = -1, .openCount = 0});
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

    bool MSRLinux::openMsrFd(const int cpu) {
        if (!loadMsrModule())
            return false;

        addSlotForCPU(cpu);

        if (msrFDMap[cpu].fd >= 0) {
            ++msrFDMap[cpu].openCount;
            return true;
        }

        const std::string path {QString("/dev/cpu/%1/msr").arg(cpu).toStdString()};

        msrFDMap[cpu].fd = open(path.c_str(), O_RDWR | O_SYNC);

        if (msrFDMap[cpu].fd >= 0) {
            ++msrFDMap[cpu].openCount;
            return true;
        }

        return false;
    }

    void MSRLinux::closeMsrFd(const int cpu) {
        if (msrFDMap[cpu].openCount == 0) [[unlikely]]
            return;

        --msrFDMap[cpu].openCount;

        if (msrFDMap[cpu].openCount > 0)
            return;

        close(msrFDMap[cpu].fd);

        msrFDMap[cpu].fd = -1;
        msrFDMap[cpu].openCount = 0;
    }

    bool MSRLinux::readMSR(uint64_t &ret, const uint32_t adr, const int cpu) const {
        if (msrFDMap[cpu].openCount == 0) [[unlikely]]
            return false;

        return pread(msrFDMap[cpu].fd, &ret, sizeof ret, adr) == sizeof ret;
    }

    bool MSRLinux::readMSR(uint32_t &ret, const uint32_t adr, const int cpu) const {
        if (msrFDMap[cpu].openCount == 0) [[unlikely]]
            return false;

        return pread(msrFDMap[cpu].fd, &ret, sizeof ret, adr) == sizeof ret;
    }

    bool MSRLinux::writeMSR(const uint64_t value, const uint32_t adr, const int cpu) const {
        if (msrFDMap[cpu].openCount == 0) [[unlikely]]
            return false;

        return pwrite(msrFDMap[cpu].fd, &value, sizeof value, adr) == sizeof value;
    }

    bool MSRLinux::writeMSR(const uint32_t value, const uint32_t adr, const int cpu) const {
        if (msrFDMap[cpu].openCount == 0) [[unlikely]]
            return false;

        return pwrite(msrFDMap[cpu].fd, &value, sizeof value, adr) == sizeof value;
    }
}
