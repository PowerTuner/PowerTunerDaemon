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

namespace PWTD::WIN {
    MemoryWindows::MemoryWindows() {
        inpOutDll = LoadLibrary(L"inpoutx64.dll");

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
        FreeLibrary(inpOutDll);
    }

    bool MemoryWindows::isAccessible() const {
        return inpOutDll != nullptr;
    }

    bool MemoryWindows::rwMem(uint64_t &data, const uint64_t addr, const size_t size, const bool write) const {
        if (inpOutDll == nullptr)
            return false;

        HANDLE physMemoryHandle;
        uint32_t *linAddr = reinterpret_cast<uint32_t *>(mapMem(addr, size, &physMemoryHandle));

        if (linAddr == nullptr)
            return false;

        if (write) {
            std::memcpy(linAddr, &data, size);

        } else {
            uint64_t ret = 0;

            std::memcpy(&ret, linAddr, size);
            data = ret;
        }

        unmapMem(physMemoryHandle, *linAddr);
        return true;
    }
}
