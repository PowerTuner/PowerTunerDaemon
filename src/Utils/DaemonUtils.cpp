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
#include <QString>
#include <array>

#include "DaemonUtils.h"

namespace PWTD {
    [[nodiscard]]
    QString getMemorySizeStr(const quint64 size) {
        static constexpr std::array<std::string_view, 6> memUnits {"Bytes", "KB", "MB", "GB", "PB", "TB"};
        double sz = static_cast<double>(size);
        int sizeIdx = 0;

        if (size == 0)
            return "";

        while (static_cast<quint64>(sz / 1024) > 0) {
            sz /= 1024;
            ++sizeIdx;

            if (sizeIdx >= memUnits.size())
                return "Unknown";
        }

        return QString("%1 %2").arg(QString::number(sz, 'g', 3), memUnits[sizeIdx].data());
    }
}
