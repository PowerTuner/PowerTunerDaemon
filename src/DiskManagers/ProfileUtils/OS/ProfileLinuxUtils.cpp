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
#include "ProfileLinuxUtils.h"
#include "../../../Utils/FileLogger/FileLogger.h"

namespace PWTD {
    void serializeLinuxData(QDataStream &ds, const PWTS::ClientPacket &packet) {
        static constexpr int linuxDataVersion = 1;

        ds << linuxDataVersion <<
            packet.linuxData->smtState <<
            packet.linuxData->cpuIdleGovernor <<
            packet.linuxData->blockDevicesQueSched <<
            packet.linuxData->miscPMDevices <<
            packet.linuxData->intelGpuData <<
            packet.linuxData->amdGpuData;

        ds << packet.linuxData->threadData.size();

        for (const auto &thdData: packet.linuxData->threadData)
            serializeLinuxThreadData(ds, thdData);
    }

    void serializeLinuxThreadData(QDataStream &ds, const PWTS::LNX::LinuxThreadData &thdData) {
        ds << thdData.cpuFrequency <<
            thdData.scalingGovernor <<
            thdData.cpuOnlineStatus;
    }

    QSharedPointer<PWTS::LNX::LinuxData> deserializeLinuxData(QDataStream &ds) {
        QSharedPointer<PWTS::LNX::LinuxData> data = QSharedPointer<PWTS::LNX::LinuxData>::create();
        qsizetype numThreads;
        int version;

        ds >> version >>
            data->smtState >>
            data->cpuIdleGovernor >>
            data->blockDevicesQueSched >>
            data->miscPMDevices >>
            data->intelGpuData >>
            data->amdGpuData;

        ds >> numThreads;

        for (qsizetype i=0; i<numThreads; ++i)
            deserializeLinuxThreadData(ds, data, version);

        return data;
    }

    void deserializeLinuxThreadData(QDataStream &ds, const QSharedPointer<PWTS::LNX::LinuxData> &data, const int version) {
        PWTS::LNX::LinuxThreadData thdData {};

        ds >> thdData.cpuFrequency >>
            thdData.scalingGovernor >>
            thdData.cpuOnlineStatus;

        data->threadData.append(thdData);
    }

    void loadLinuxProfileToDaemonPacket(const QSharedPointer<PWTS::LNX::LinuxData> &profile, const QSharedPointer<PWTS::LNX::LinuxData> &packet) {
        packet->smtState = profile->smtState;
        packet->cpuIdleGovernor = profile->cpuIdleGovernor;
        packet->miscPMDevices = profile->miscPMDevices;

        for (const auto &[dev, data]: profile->blockDevicesQueSched.asKeyValueRange()) {
            if (!packet->blockDevicesQueSched.contains(dev))
                continue;

            packet->blockDevicesQueSched[dev].scheduler = data.scheduler;
        }

        for (const auto &[gpuIdx, gpuData]: profile->intelGpuData.asKeyValueRange()) {
            if (!packet->intelGpuData.contains(gpuIdx)) {
                if (FileLogger::getInstance()->isLevel(PWTS::LogLevel::Warning))
                    FileLogger::getInstance()->write(QString("Cannot load Intel GPU data for card %1, missing on device").arg(gpuIdx));

                continue;
            }

            packet->intelGpuData[gpuIdx].frequency = gpuData.frequency;
            packet->intelGpuData[gpuIdx].boostFrequency = gpuData.boostFrequency;
        }

        for (const auto &[gpuIdx, gpuData]: profile->amdGpuData.asKeyValueRange()) {
            if (!packet->amdGpuData.contains(gpuIdx)) {
                if (FileLogger::getInstance()->isLevel(PWTS::LogLevel::Warning))
                    FileLogger::getInstance()->write(QString("Cannot load AMD GPU data for card %1, missing on device").arg(gpuIdx));

                continue;
            }

            packet->amdGpuData[gpuIdx].dpmForcePerfLevel = gpuData.dpmForcePerfLevel;
            packet->amdGpuData[gpuIdx].powerDpmState = gpuData.powerDpmState;
        }

        for (qsizetype i=0,l=profile->threadData.size(); i<l; ++i) {
            packet->threadData[i].cpuFrequency = profile->threadData[i].cpuFrequency;
            packet->threadData[i].scalingGovernor = profile->threadData[i].scalingGovernor;
            packet->threadData[i].cpuOnlineStatus = profile->threadData[i].cpuOnlineStatus;
        }
    }
}
