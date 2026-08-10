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
#define _FILE_OFFSET_BITS 64
#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>

#include "MemoryLinux.h"

namespace PWTD::MEM {
    bool MemoryLinux::forbidden() const {
        const int fd = open("/dev/mem", O_RDWR | O_SYNC);

        if (fd < 0)
            return true;

        close(fd);
        return false;
    }

    std::optional<std::pair<unsigned char *, size_t>> MemoryLinux::mapMemory(const uint64_t addr, const size_t size, const int memFlags, const int mapFlags) const {
        const long pageSz = sysconf(_SC_PAGE_SIZE);

        if (pageSz <= 0)
            return {};

        const off_t offset = static_cast<off_t>(addr & ~static_cast<uint64_t>(pageSz - 1));
        const size_t mapSz = size + addr - offset;
        const int memfd = open("/dev/mem", memFlags | O_SYNC);
        unsigned char *mapAdr;

        if (memfd < 0)
            return {};

        mapAdr = static_cast<unsigned char *>(mmap(nullptr, mapSz, mapFlags, MAP_SHARED, memfd, offset));
        close(memfd);

        if (mapAdr == MAP_FAILED)
            return {};

        return std::make_pair(mapAdr, mapSz);
    }

    std::optional<uint64_t> MemoryLinux::read(const uint64_t addr, const size_t size) const {
        const std::optional<std::pair<unsigned char *, size_t>> memMap = mapMemory(addr, size, O_RDONLY, PROT_READ);
        uint64_t ret = 0;

        if (!memMap)
            return {};

        const auto [mapAdr, mapSz] = memMap.value();

        std::memcpy(&ret, mapAdr + mapSz - size, size);
        munmap(mapAdr, mapSz);
        return ret;
    }

    bool MemoryLinux::write(const uint64_t data, const uint64_t addr, const size_t size) const {
        const std::optional<std::pair<unsigned char *, size_t>> memMap = mapMemory(addr, size, O_WRONLY, PROT_WRITE);

        if (!memMap)
            return false;

        const auto [mapAdr, mapSz] = memMap.value();

        std::memcpy(mapAdr + mapSz - size, &data, size);
        munmap(mapAdr, mapSz);
        return true;
    }

    bool MemoryLinux::read8(const uint64_t addr, uint8_t &out) const {
        const std::optional<uint64_t> mem = read(addr, sizeof(uint8_t));

        if (mem)
            out = static_cast<uint8_t>(*mem);

        return mem.has_value();
    }

    bool MemoryLinux::read16(const uint64_t addr, uint16_t &out) const {
        const std::optional<uint64_t> mem = read(addr, sizeof(uint16_t));

        if (mem)
            out = static_cast<uint16_t>(*mem);

        return mem.has_value();
    }

    bool MemoryLinux::read32(const uint64_t addr, uint32_t &out) const {
        const std::optional<uint64_t> mem = read(addr, sizeof(uint32_t));

        if (mem)
            out = static_cast<uint32_t>(*mem);

        return mem.has_value();
    }

    bool MemoryLinux::read64(const uint64_t addr, uint64_t &out) const {
        const std::optional<uint64_t> mem = read(addr, sizeof(uint64_t));

        if (mem)
            out = *mem;

        return mem.has_value();
    }

    bool MemoryLinux::write8(const uint8_t val, const uint64_t addr) const {
        return write(val, addr, sizeof(uint8_t));
    }

    bool MemoryLinux::write16(const uint16_t val, const uint64_t addr) const {
        return write(val, addr, sizeof(uint16_t));
    }

    bool MemoryLinux::write32(const uint32_t val, const uint64_t addr) const {
        return write(val, addr, sizeof(uint32_t));
    }

    bool MemoryLinux::write64(const uint64_t val, const uint64_t addr) const {
        return write(val, addr, sizeof(uint64_t));
    }
}
