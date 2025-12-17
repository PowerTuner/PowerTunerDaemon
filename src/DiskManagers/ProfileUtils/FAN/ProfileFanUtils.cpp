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
#include "ProfileFanUtils.h"

namespace PWTD {
    void serializeFanData(QDataStream &ds, const PWTS::ClientPacket &packet) {
        static constexpr int fanDataVersion = 1;

        ds << fanDataVersion <<
            packet.fanData;
    }

    QMap<QString, PWTS::FanData> deserializeFanData(QDataStream &ds) {
        QMap<QString, PWTS::FanData> data;
        int version;

        ds >> version >> data;

        return data;
    }
}
