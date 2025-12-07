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

#include <QStandardPaths>
#include <QLockFile>

namespace PWTD {
    class ProcessStatus final {
    private:
        const QString tmpPath {QStandardPaths::writableLocation(QStandardPaths::TempLocation)};
        QLockFile flock {QString("%1/pwtd.lock").arg(tmpPath)};

    public:
        ProcessStatus() { flock.setStaleLockTime(0); }
        ~ProcessStatus() { flock.unlock(); }

        [[nodiscard]]
        QLockFile::LockError isAlreadyRunning() {
            if (tmpPath.isEmpty())
                return QLockFile::UnknownError;
            else if (!flock.tryLock(700))
                return flock.error();

            return QLockFile::NoError;
        }
    };
}
