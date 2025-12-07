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

#include "Utils/MSR/MSRFactory.h"
#include "../../Utils/FileLogger/FileLogger.h"

namespace PWTD {
    class CPURegister {
    protected:
        QSharedPointer<MSR> msrUtils;
        QSharedPointer<FileLogger> logger;
        uint32_t addr;

    public:
        CPURegister() {
            msrUtils = MSRFactory::getMSRInstance();
            logger = FileLogger::getInstance();
        }

        virtual ~CPURegister() = default;
    };
}
