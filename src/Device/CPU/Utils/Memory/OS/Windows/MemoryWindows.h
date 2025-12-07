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

#include "pwtWin32/win.h"
#include "../../Memory.h"

typedef BOOL (__stdcall IsInpOutDriverOpen)(void);
typedef PBYTE (__stdcall MapPhysToLin)(uintptr_t pbPhysAddr, size_t dwPhysSize, HANDLE *pPhysicalMemoryHandle);
typedef BOOL (__stdcall UnmapPhysicalMemory)(HANDLE PhysicalMemoryHandle, uintptr_t pbLinAddr); // return type is probably wrong

namespace PWTD::WIN {
    class MemoryWindows final: public Memory {
    private:
        IsInpOutDriverOpen *isDriverOpen = nullptr;
        MapPhysToLin *mapMem = nullptr;
        UnmapPhysicalMemory *unmapMem = nullptr;
        HINSTANCE inpOutDll = nullptr;

        [[nodiscard]] bool rwMem(uint64_t &data, uint64_t addr, size_t size, bool write) const;

    public:
        MemoryWindows();
        ~MemoryWindows() override;

        [[nodiscard]] bool isAccessible() const override;
        [[nodiscard]] bool readMem8(uint64_t &val, const uint64_t addr) const override { return rwMem(val, addr, sizeof(uint8_t), false); }
        [[nodiscard]] bool readMem16(uint64_t &val, const uint64_t addr) const override { return rwMem(val, addr, sizeof(uint16_t), false); }
        [[nodiscard]] bool readMem32(uint64_t &val, const uint64_t addr) const override { return rwMem(val, addr, sizeof(uint32_t), false); }
        [[nodiscard]] bool readMem64(uint64_t &val, const uint64_t addr) const override { return rwMem(val, addr, sizeof(uint64_t), false); }
        [[nodiscard]] bool writeMem8(uint64_t &val, const uint64_t addr) const override { return rwMem(val, addr, sizeof(uint8_t), true); }
        [[nodiscard]] bool writeMem16(uint64_t &val, const uint64_t addr) const override { return rwMem(val, addr, sizeof(uint16_t), true); }
        [[nodiscard]] bool writeMem32(uint64_t &val, const uint64_t addr) const override { return rwMem(val, addr, sizeof(uint32_t), true); }
        [[nodiscard]] bool writeMem64(uint64_t &val, const uint64_t addr) const override { return rwMem(val, addr, sizeof(uint64_t), true); }
    };
}
