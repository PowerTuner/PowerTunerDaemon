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
#include "ProfileAMDUtils.h"

namespace PWTD {
    void serializeAMDData(QDataStream &ds, const PWTS::ClientPacket &packet) {
        static constexpr int amdDataVersion = 1;

        ds << amdDataVersion <<
            packet.amdData->stapmLimit <<
            packet.amdData->fastLimit <<
            packet.amdData->slowLimit <<
            packet.amdData->tctlTemp <<
            packet.amdData->apuSlow <<
            packet.amdData->apuSkinTemp <<
            packet.amdData->dgpuSkinTemp <<
            packet.amdData->vrmCurrent <<
            packet.amdData->vrmSocCurrent <<
            packet.amdData->vrmMaxCurrent <<
            packet.amdData->vrmSocMaxCurrent <<
            packet.amdData->staticGfxClock <<
            packet.amdData->minGfxClock <<
            packet.amdData->maxGfxClock <<
            packet.amdData->powerProfile <<
            packet.amdData->cppcEnableBit;

        ds << packet.amdData->coreData.size() <<
            packet.amdData->threadData.size();

        for (const auto &coreData: packet.amdData->coreData)
            serializeAMDCoreData(ds, coreData);

        for (const auto &thdData: packet.amdData->threadData)
            serializeAMDThreadData(ds, thdData);
    }

    void serializeAMDCoreData(QDataStream &ds, const PWTS::AMD::AMDCoreData &coreData) {
        ds << coreData.curveOptimizer;
    }

    void serializeAMDThreadData(QDataStream &ds, const PWTS::AMD::AMDThreadData &thdData) {
        ds << thdData.cppcRequest <<
            thdData.pstateCmd <<
            thdData.corePerfBoost;
    }

    QSharedPointer<PWTS::AMD::AMDData> deserializeAMDData(QDataStream &ds) {
        QSharedPointer<PWTS::AMD::AMDData> data = QSharedPointer<PWTS::AMD::AMDData>::create();
        qsizetype numCores, numThreads;
        int version;

        ds >> version >>
            data->stapmLimit >>
            data->fastLimit >>
            data->slowLimit >>
            data->tctlTemp >>
            data->apuSlow >>
            data->apuSkinTemp >>
            data->dgpuSkinTemp >>
            data->vrmCurrent >>
            data->vrmSocCurrent >>
            data->vrmMaxCurrent >>
            data->vrmSocMaxCurrent >>
            data->staticGfxClock >>
            data->minGfxClock >>
            data->maxGfxClock >>
            data->powerProfile >>
            data->cppcEnableBit;

        ds >> numCores >> numThreads;

        for (qsizetype i=0; i<numCores; ++i)
            deserializeAMDCoreData(ds, data, version);

        for (qsizetype i=0; i<numThreads; ++i)
            deserializeAMDThreadData(ds, data, version);

        return data;
    }

    void deserializeAMDCoreData(QDataStream &ds, const QSharedPointer<PWTS::AMD::AMDData> &data, const int version) {
        PWTS::AMD::AMDCoreData coreData {};

        ds >> coreData.curveOptimizer;

        data->coreData.append(coreData);
    }

    void deserializeAMDThreadData(QDataStream &ds, const QSharedPointer<PWTS::AMD::AMDData> &data, const int version) {
        PWTS::AMD::AMDThreadData thdData {};

        ds >> thdData.cppcRequest >>
            thdData.pstateCmd >>
            thdData.corePerfBoost;

        data->threadData.append(thdData);
    }

    void loadAMDProfileToDaemonPacket(const QSharedPointer<PWTS::AMD::AMDData> &profile, const QSharedPointer<PWTS::AMD::AMDData> &packet) {
        packet->stapmLimit = profile->stapmLimit;
        packet->fastLimit = profile->fastLimit;
        packet->slowLimit = profile->slowLimit;
        packet->tctlTemp = profile->tctlTemp;
        packet->apuSlow = profile->apuSlow;
        packet->apuSkinTemp = profile->apuSkinTemp;
        packet->dgpuSkinTemp = profile->dgpuSkinTemp;
        packet->vrmCurrent = profile->vrmCurrent;
        packet->vrmSocCurrent = profile->vrmSocCurrent;
        packet->vrmMaxCurrent = profile->vrmMaxCurrent;
        packet->vrmSocMaxCurrent = profile->vrmSocMaxCurrent;
        packet->staticGfxClock = profile->staticGfxClock;
        packet->minGfxClock = profile->minGfxClock;
        packet->maxGfxClock = profile->maxGfxClock;
        packet->powerProfile = profile->powerProfile;
        packet->cppcEnableBit = profile->cppcEnableBit;

        for (qsizetype i=0,l=profile->coreData.size(); i<l; ++i) {
            packet->coreData[i].curveOptimizer = profile->coreData[i].curveOptimizer;
        }

        for (qsizetype i=0,l=profile->threadData.size(); i<l; ++i) {
            packet->threadData[i].cppcRequest = profile->threadData[i].cppcRequest;
            packet->threadData[i].pstateCmd = profile->threadData[i].pstateCmd;
            packet->threadData[i].corePerfBoost = profile->threadData[i].corePerfBoost;
        }
    }
}
