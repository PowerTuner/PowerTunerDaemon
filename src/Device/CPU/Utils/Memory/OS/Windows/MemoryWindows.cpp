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
#include "MemoryWindows.h"

#include <cstring>

namespace PWTD::MEM {
    MemoryWindows::MemoryWindows() {
        inpOutDll = LoadLibraryW(L"inpoutx64.dll");

        if (inpOutDll == nullptr)
            return;

        isDriverOpen = reinterpret_cast<IsInpOutDriverOpen *>(GetProcAddress(inpOutDll, "IsInpOutDriverOpen"));
        mapMem = reinterpret_cast<MapPhysToLin *>(GetProcAddress(inpOutDll, "MapPhysToLin"));
        unmapMem = reinterpret_cast<UnmapPhysicalMemory *>(GetProcAddress(inpOutDll, "UnmapPhysicalMemory"));

        if (!isDriverOpen()) {
            FreeLibrary(inpOutDll);

            inpOutDll = nullptr;
            isDriverOpen = nullptr;
            mapMem = nullptr;
            unmapMem = nullptr;
        }
    }

    MemoryWindows::~MemoryWindows() {
        if (inpOutDll != nullptr)
            FreeLibrary(inpOutDll);
    }

    bool MemoryWindows::forbidden() const {
        return inpOutDll == nullptr;
    }

    std::optional<uint64_t> MemoryWindows::read(const uint64_t addr, const size_t size) const {
        if (inpOutDll == nullptr)
            return {};

        HANDLE physMemoryHandle;
        uint32_t *linAddr = reinterpret_cast<uint32_t *>(mapMem(addr, size, &physMemoryHandle));
        uint64_t ret = 0;

        if (linAddr == nullptr)
            return {};

        std::memcpy(&ret, linAddr, size);
        unmapMem(physMemoryHandle, *linAddr);
        return ret;
    }

    bool MemoryWindows::write(const uint64_t val, const uint64_t addr, const size_t size) const {
        if (inpOutDll == nullptr)
            return false;

        HANDLE physMemoryHandle;
        uint32_t *linAddr = reinterpret_cast<uint32_t *>(mapMem(addr, size, &physMemoryHandle));

        if (linAddr == nullptr)
            return false;

        std::memcpy(linAddr, &val, size);
        unmapMem(physMemoryHandle, *linAddr);
        return true;
    }

    bool MemoryWindows::read8(const uint64_t addr, uint8_t &out) const {
        const std::optional<uint64_t> mem = read(addr, sizeof(uint8_t));

        if (mem)
            out = static_cast<uint8_t>(*mem);

        return mem.has_value();
    }

    bool MemoryWindows::read16(const uint64_t addr, uint16_t &out) const {
        const std::optional<uint64_t> mem = read(addr, sizeof(uint16_t));

        if (mem)
            out = static_cast<uint16_t>(*mem);

        return mem.has_value();
    }

    bool MemoryWindows::read32(const uint64_t addr, uint32_t &out) const {
        const std::optional<uint64_t> mem = read(addr, sizeof(uint32_t));

        if (mem)
            out = static_cast<uint32_t>(*mem);

        return mem.has_value();
    }

    bool MemoryWindows::read64(const uint64_t addr, uint64_t &out) const {
        const std::optional<uint64_t> mem = read(addr, sizeof(uint64_t));

        if (mem)
            out = *mem;

        return mem.has_value();
    }

    bool MemoryWindows::write8(const uint8_t val, const uint64_t addr) const {
        return write(val, addr, sizeof(uint8_t));
    }

    bool MemoryWindows::write16(const uint16_t val, const uint64_t addr) const {
        return write(val, addr, sizeof(uint16_t));
    }

    bool MemoryWindows::write32(const uint32_t val, const uint64_t addr) const {
        return write(val, addr, sizeof(uint32_t));
    }

    bool MemoryWindows::write64(const uint64_t val, const uint64_t addr) const {
        return write(val, addr, sizeof(uint64_t));
    }
}
