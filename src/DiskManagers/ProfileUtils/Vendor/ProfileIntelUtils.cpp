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
#include "ProfileIntelUtils.h"

namespace PWTD {
    void serializeIntelData(QDataStream &ds, const PWTS::ClientPacket &packet) {
        static constexpr int intelDataVersion = 1;

        ds << intelDataVersion <<
            packet.intelData->pkgPowerLimit <<
            packet.intelData->vrCurrentCfg <<
            packet.intelData->pp1CurrentCfg <<
            packet.intelData->turboPowerCurrentLimit <<
            packet.intelData->turboRatioLimit <<
            packet.intelData->miscProcFeatures <<
            packet.intelData->powerCtl <<
            packet.intelData->miscPwrMgmt <<
            packet.intelData->hwpRequestPkg <<
            packet.intelData->undervoltData <<
            packet.intelData->pp0Priority <<
            packet.intelData->pp1Priority <<
            packet.intelData->energyPerfBias <<
            packet.intelData->hwpEnable <<
            packet.intelData->hwpPkgCtlPolarity <<
            packet.intelData->mchbarPkgRaplLimit;

        ds << packet.intelData->coreData.size() <<
            packet.intelData->threadData.size();

        for (const auto &coreData: packet.intelData->coreData)
            serializeIntelCoreData(ds, coreData);

        for (const auto &thdData: packet.intelData->threadData)
            serializeIntelThreadData(ds, thdData);
    }

    void serializeIntelCoreData(QDataStream &ds, const PWTS::Intel::IntelCoreData &coreData) {
        ds << coreData.pkgCstConfigControl;
    }

    void serializeIntelThreadData(QDataStream &ds, const PWTS::Intel::IntelThreadData &thdData) {
        ds << thdData.hwpRequest;
    }

    QSharedPointer<PWTS::Intel::IntelData> deserializeIntelData(QDataStream &ds) {
        QSharedPointer<PWTS::Intel::IntelData> data = QSharedPointer<PWTS::Intel::IntelData>::create();
        qsizetype numCores, numThreads;
        int version;

        ds >> version >>
            data->pkgPowerLimit >>
            data->vrCurrentCfg >>
            data->pp1CurrentCfg >>
            data->turboPowerCurrentLimit >>
            data->turboRatioLimit >>
            data->miscProcFeatures >>
            data->powerCtl >>
            data->miscPwrMgmt >>
            data->hwpRequestPkg >>
            data->undervoltData >>
            data->pp0Priority >>
            data->pp1Priority >>
            data->energyPerfBias >>
            data->hwpEnable >>
            data->hwpPkgCtlPolarity >>
            data->mchbarPkgRaplLimit;

        ds >> numCores >> numThreads;

        for (qsizetype i=0; i<numCores; ++i)
            deserializeIntelCoreData(ds, data, version);

        for (qsizetype i=0; i<numThreads; ++i)
            deserializeIntelThreadData(ds, data, version);

        return data;
    }

    void deserializeIntelCoreData(QDataStream &ds, const QSharedPointer<PWTS::Intel::IntelData> &data, const int version) {
        PWTS::Intel::IntelCoreData coreData {};

        ds >> coreData.pkgCstConfigControl;

        data->coreData.append(coreData);
    }

    void deserializeIntelThreadData(QDataStream &ds, const QSharedPointer<PWTS::Intel::IntelData> &data, const int version) {
        PWTS::Intel::IntelThreadData thdData {};

        ds >> thdData.hwpRequest;

        data->threadData.append(thdData);
    }

    void loadIntelProfileToDaemonPacket(const QSharedPointer<PWTS::Intel::IntelData> &profile, const QSharedPointer<PWTS::Intel::IntelData> &packet) {
        packet->pkgPowerLimit = profile->pkgPowerLimit;
        packet->vrCurrentCfg = profile->vrCurrentCfg;
        packet->pp1CurrentCfg = profile->pp1CurrentCfg;
        packet->turboPowerCurrentLimit = profile->turboPowerCurrentLimit;
        packet->turboRatioLimit = profile->turboRatioLimit;
        packet->miscProcFeatures = profile->miscProcFeatures;
        packet->powerCtl = profile->powerCtl;
        packet->miscPwrMgmt = profile->miscPwrMgmt;
        packet->hwpRequestPkg = profile->hwpRequestPkg;
        packet->undervoltData = profile->undervoltData;
        packet->pp0Priority = profile->pp0Priority;
        packet->pp1Priority = profile->pp1Priority;
        packet->energyPerfBias = profile->energyPerfBias;
        packet->hwpEnable = profile->hwpEnable;
        packet->hwpPkgCtlPolarity = profile->hwpPkgCtlPolarity;
        packet->mchbarPkgRaplLimit = profile->mchbarPkgRaplLimit;

        for (qsizetype i=0,l=profile->coreData.size(); i<l; ++i) {
            packet->coreData[i].pkgCstConfigControl = profile->coreData[i].pkgCstConfigControl;
        }

        for (qsizetype i=0,l=profile->threadData.size(); i<l; ++i) {
            packet->threadData[i].hwpRequest = profile->threadData[i].hwpRequest;
        }
    }
}
