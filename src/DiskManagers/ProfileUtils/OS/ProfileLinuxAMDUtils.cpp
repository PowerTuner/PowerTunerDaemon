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
#include "ProfileLinuxAMDUtils.h"

namespace PWTD {
    void serializeLinuxAMDData(QDataStream &ds, const PWTS::ClientPacket &packet) {
        static constexpr int linuxAmdDataVersion = 1;

        ds << linuxAmdDataVersion <<
            packet.linuxAmdData->pstateStatus;

        ds << packet.linuxAmdData->threadData.size();

        for (const auto &thdData: packet.linuxAmdData->threadData)
            serializeLinuxAMDThreadData(ds, thdData);
    }

    void serializeLinuxAMDThreadData(QDataStream &ds, const PWTS::LNX::AMD::LinuxAMDThreadData &thdData) {
        ds << thdData.epp;
    }

    QSharedPointer<PWTS::LNX::AMD::LinuxAMDData> deserializeLinuxAMDData(QDataStream &ds) {
        QSharedPointer<PWTS::LNX::AMD::LinuxAMDData> data = QSharedPointer<PWTS::LNX::AMD::LinuxAMDData>::create();
        qsizetype numLAThreads;
        int version;

        ds >> version >>
            data->pstateStatus;

        ds >> numLAThreads;

        for (qsizetype i=0; i<numLAThreads; ++i)
            deserializeLinuxAMDThreadData(ds, data, version);

        return data;
    }

    void deserializeLinuxAMDThreadData(QDataStream &ds, const QSharedPointer<PWTS::LNX::AMD::LinuxAMDData> &data, const int version) {
        PWTS::LNX::AMD::LinuxAMDThreadData thdData {};

        ds >> thdData.epp;

        data->threadData.append(thdData);
    }

    void loadLinuxAMDProfileToDaemonPacket(const QSharedPointer<PWTS::LNX::AMD::LinuxAMDData> &profile, const QSharedPointer<PWTS::LNX::AMD::LinuxAMDData> &packet) {
        packet->pstateStatus = profile->pstateStatus;

        for (qsizetype i=0,l=profile->threadData.size(); i<l; ++i) {
            packet->threadData[i].epp = profile->threadData[i].epp;
        }
    }
}
