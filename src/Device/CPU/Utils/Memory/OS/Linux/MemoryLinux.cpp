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
#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>

#include "MemoryLinux.h"

namespace PWTD::LNX {
    bool MemoryLinux::isAccessible() const {
        const int fd = open("/dev/mem", O_RDWR | O_SYNC);

        if (fd < 0)
            return false;

        close(fd);
        return true;
    }

    bool MemoryLinux::rwMem(uint64_t &data, const uint64_t addr, const size_t size, const bool write) const {
        const off_t paOfft = addr & ~(sysconf(_SC_PAGE_SIZE) - 1);
        const size_t mapSize = size + addr - paOfft;
        unsigned char *mapAdr;
        int memfd;

        memfd = open("/dev/mem", O_RDWR | O_SYNC);
        if (memfd < 0)
            return false;

        mapAdr = static_cast<unsigned char *>(mmap(nullptr, mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, paOfft));

        close(memfd);

        if (mapAdr == MAP_FAILED)
            return false;

        if (write) {
            std::memcpy(mapAdr + mapSize - size, &data, size);

        } else {
            uint64_t ret = 0;

            std::memcpy(&ret, mapAdr + mapSize - size, size);
            data = ret;
        }

        munmap(mapAdr, mapSize);
        return true;
    }
}
