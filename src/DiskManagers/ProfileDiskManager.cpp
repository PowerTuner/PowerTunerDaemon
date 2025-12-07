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
#include <QDirListing>
#include <QCryptographicHash>
#include <QDir>

#include "ProfileDiskManager.h"
#include "../Utils/AppDataPath.h"
#include "pwtShared/Utils.h"

namespace PWTD {
    ProfileDiskManager::ProfileDiskManager(const QByteArray &hash, const PWTS::CPUVendor vendor) {
        const QDir qdir;

        path = AppDataPath::appDataLocation();
        logger = FileLogger::getInstance();
        cpuVendor = vendor;
        deviceHash = hash;

        if (path.isEmpty())
            return;

        path.append("/profiles");

        if (!qdir.exists(path) && !qdir.mkdir(path)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("Failed to create profiles folder, cannot create/load profiles!"));

            path.clear();
            return;
        }

        fsWatcherEvtTimer.reset(new QTimer);
        fsWatcherEvtTimer->setSingleShot(true);
        fsWatcherEvtTimer->setInterval(1200);
        fsWatcher.reset(new QFileSystemWatcher);
        fsWatcher->addPath(path);

        QObject::connect(fsWatcher.get(), &QFileSystemWatcher::directoryChanged, this, &ProfileDiskManager::onDirChanged);
        QObject::connect(fsWatcherEvtTimer.get(), &QTimer::timeout, this, &ProfileDiskManager::onFsWatcherTimerTimeout);
    }

#ifdef __linux__
    void ProfileDiskManager::serializeLinuxData(QDataStream &ds, const PWTS::ClientPacket &packet) const {
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

    void ProfileDiskManager::serializeLinuxThreadData(QDataStream &ds, const PWTS::LNX::LinuxThreadData &thdData) const {
        ds << thdData.cpuFrequency <<
            thdData.scalingGovernor <<
            thdData.cpuOnlineStatus;
    }

    void ProfileDiskManager::deserializeLinuxData(QDataStream &ds, DiskData &profile) const {
        qsizetype numThreads;
        int version;

        profile.linuxD = QSharedPointer<PWTS::LNX::LinuxData>::create();

        ds >> version >>
            profile.linuxD->smtState >>
            profile.linuxD->cpuIdleGovernor >>
            profile.linuxD->blockDevicesQueSched >>
            profile.linuxD->miscPMDevices >>
            profile.linuxD->intelGpuData >>
            profile.linuxD->amdGpuData;

        ds >> numThreads;

        for (qsizetype i=0; i<numThreads; ++i)
            deserializeLinuxThreadData(ds, profile, version);
    }

    void ProfileDiskManager::deserializeLinuxThreadData(QDataStream &ds, const DiskData &profile, const int version) const {
        PWTS::LNX::LinuxThreadData thdData {};

        ds >> thdData.cpuFrequency >>
            thdData.scalingGovernor >>
            thdData.cpuOnlineStatus;

        profile.linuxD->threadData.append(thdData);
    }

    void ProfileDiskManager::loadLinuxProfileToDaemonPacket(const QSharedPointer<PWTS::LNX::LinuxData> &profile, const QSharedPointer<PWTS::LNX::LinuxData> &packet) const {
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
                if (logger->isLevel(PWTS::LogLevel::Warning))
                    logger->write(QString("Cannot load Intel GPU data for card %1, missing on device").arg(gpuIdx));

                continue;
            }

            packet->intelGpuData[gpuIdx].frequency = gpuData.frequency;
            packet->intelGpuData[gpuIdx].boostFrequency = gpuData.boostFrequency;
        }

        for (const auto &[gpuIdx, gpuData]: profile->amdGpuData.asKeyValueRange()) {
            if (!packet->amdGpuData.contains(gpuIdx)) {
                if (logger->isLevel(PWTS::LogLevel::Warning))
                    logger->write(QString("Cannot load AMD GPU data for card %1, missing on device").arg(gpuIdx));

                continue;
            }

            packet->amdGpuData[gpuIdx].dpmForcePerfLevel = gpuData.dpmForcePerfLevel;
            packet->amdGpuData[gpuIdx].powerDpmState = gpuData.powerDpmState;
        }

        for (int i=0,l=profile->threadData.size(); i<l; ++i) {
            packet->threadData[i].cpuFrequency = profile->threadData[i].cpuFrequency;
            packet->threadData[i].scalingGovernor = profile->threadData[i].scalingGovernor;
            packet->threadData[i].cpuOnlineStatus = profile->threadData[i].cpuOnlineStatus;
        }
    }
#elif defined(_WIN32)
    void ProfileDiskManager::serializeWindowsData(QDataStream &ds, const PWTS::ClientPacket &packet) const {
        ds << windowsDataVersion <<
            packet.windowsData->activeScheme <<
            packet.windowsData->schemes <<
            packet.windowsData->replaceDefaultSchemes <<
            packet.windowsData->resetSchemesDefault;
    }

    void ProfileDiskManager::deserializeWindowsData(QDataStream &ds, DiskData &profile) const {
        profile.windowsD = QSharedPointer<PWTS::WIN::WindowsData>::create();
        int version;

        ds >> version >>
            profile.windowsD->activeScheme >>
            profile.windowsD->schemes >>
            profile.windowsD->replaceDefaultSchemes >>
            profile.windowsD->resetSchemesDefault;
    }

    void ProfileDiskManager::loadWindowsProfileToDaemonPacket(const QSharedPointer<PWTS::WIN::WindowsData> &profile, const QSharedPointer<PWTS::WIN::WindowsData> &packet) const {
        packet->activeScheme = profile->activeScheme;
        packet->schemes = profile->schemes;
        packet->replaceDefaultSchemes = profile->replaceDefaultSchemes;
        packet->resetSchemesDefault = profile->resetSchemesDefault;
    }
#endif
#ifdef WITH_INTEL
    void ProfileDiskManager::serializeIntelData(QDataStream &ds, const PWTS::ClientPacket &packet) const {
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

    void ProfileDiskManager::serializeIntelCoreData(QDataStream &ds, const PWTS::Intel::IntelCoreData &coreData) const {
        ds << coreData.pkgCstConfigControl;
    }

    void ProfileDiskManager::serializeIntelThreadData(QDataStream &ds, const PWTS::Intel::IntelThreadData &thdData) const {
        ds << thdData.hwpRequest;
    }

    void ProfileDiskManager::deserializeIntelData(QDataStream &ds, DiskData &profile) const {
        qsizetype numCores, numThreads;
        int version;

        profile.intelD = QSharedPointer<PWTS::Intel::IntelData>::create();

        ds >> version >>
            profile.intelD->pkgPowerLimit >>
            profile.intelD->vrCurrentCfg >>
            profile.intelD->pp1CurrentCfg >>
            profile.intelD->turboPowerCurrentLimit >>
            profile.intelD->turboRatioLimit >>
            profile.intelD->miscProcFeatures >>
            profile.intelD->powerCtl >>
            profile.intelD->miscPwrMgmt >>
            profile.intelD->hwpRequestPkg >>
            profile.intelD->undervoltData >>
            profile.intelD->pp0Priority >>
            profile.intelD->pp1Priority >>
            profile.intelD->energyPerfBias >>
            profile.intelD->hwpEnable >>
            profile.intelD->hwpPkgCtlPolarity >>
            profile.intelD->mchbarPkgRaplLimit;

        ds >> numCores >> numThreads;

        for (qsizetype i=0; i<numCores; ++i)
            deserializeIntelCoreData(ds, profile, version);

        for (qsizetype i=0; i<numThreads; ++i)
            deserializeIntelThreadData(ds, profile, version);
    }

    void ProfileDiskManager::deserializeIntelCoreData(QDataStream &ds, const DiskData &profile, int version) const {
        PWTS::Intel::IntelCoreData coreData {};

        ds >> coreData.pkgCstConfigControl;

        profile.intelD->coreData.append(coreData);
    }

    void ProfileDiskManager::deserializeIntelThreadData(QDataStream &ds, const DiskData &profile, const int version) const {
        PWTS::Intel::IntelThreadData thdData {};

        ds >> thdData.hwpRequest;

        profile.intelD->threadData.append(thdData);
    }

    void ProfileDiskManager::loadIntelProfileToDaemonPacket(const QSharedPointer<PWTS::Intel::IntelData> &profile, const QSharedPointer<PWTS::Intel::IntelData> &packet) const {
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

        for (int i=0,l=profile->coreData.size(); i<l; ++i) {
            packet->coreData[i].pkgCstConfigControl = profile->coreData[i].pkgCstConfigControl;
        }

        for (int i=0,l=profile->threadData.size(); i<l; ++i) {
            packet->threadData[i].hwpRequest = profile->threadData[i].hwpRequest;
        }
    }
#endif
#ifdef WITH_AMD
    void ProfileDiskManager::serializeAMDData(QDataStream &ds, const PWTS::ClientPacket &packet) const {
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

    void ProfileDiskManager::serializeAMDCoreData(QDataStream &ds, const PWTS::AMD::AMDCoreData &coreData) const {
        ds << coreData.curveOptimizer;
    }

    void ProfileDiskManager::serializeAMDThreadData(QDataStream &ds, const PWTS::AMD::AMDThreadData &thdData) const {
        ds << thdData.cppcRequest <<
            thdData.pstateCmd <<
            thdData.corePerfBoost;
    }

    void ProfileDiskManager::deserializeAMDData(QDataStream &ds, DiskData &profile) const {
        qsizetype numCores, numThreads;
        int version;

        profile.amdD = QSharedPointer<PWTS::AMD::AMDData>::create();

        ds >> version >>
            profile.amdD->stapmLimit >>
            profile.amdD->fastLimit >>
            profile.amdD->slowLimit >>
            profile.amdD->tctlTemp >>
            profile.amdD->apuSlow >>
            profile.amdD->apuSkinTemp >>
            profile.amdD->dgpuSkinTemp >>
            profile.amdD->vrmCurrent >>
            profile.amdD->vrmSocCurrent >>
            profile.amdD->vrmMaxCurrent >>
            profile.amdD->vrmSocMaxCurrent >>
            profile.amdD->staticGfxClock >>
            profile.amdD->minGfxClock >>
            profile.amdD->maxGfxClock >>
            profile.amdD->powerProfile >>
            profile.amdD->cppcEnableBit;

        ds >> numCores >> numThreads;

        for (qsizetype i=0; i<numCores; ++i)
            deserializeAMDCoreData(ds, profile, version);

        for (qsizetype i=0; i<numThreads; ++i)
            deserializeAMDThreadData(ds, profile, version);
    }

    void ProfileDiskManager::deserializeAMDCoreData(QDataStream &ds, const DiskData &profile, const int version) const {
        PWTS::AMD::AMDCoreData coreData {};

        ds >> coreData.curveOptimizer;

        profile.amdD->coreData.append(coreData);
    }

    void ProfileDiskManager::deserializeAMDThreadData(QDataStream &ds, const DiskData &profile, const int version) const {
        PWTS::AMD::AMDThreadData thdData {};

        ds >> thdData.cppcRequest >>
            thdData.pstateCmd >>
            thdData.corePerfBoost;

        profile.amdD->threadData.append(thdData);
    }

    void ProfileDiskManager::loadAMDProfileToDaemonPacket(const QSharedPointer<PWTS::AMD::AMDData> &profile, const QSharedPointer<PWTS::AMD::AMDData> &packet) const {
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

        for (int i=0,l=profile->coreData.size(); i<l; ++i) {
            packet->coreData[i].curveOptimizer = profile->coreData[i].curveOptimizer;
        }

        for (int i=0,l=profile->threadData.size(); i<l; ++i) {
            packet->threadData[i].cppcRequest = profile->threadData[i].cppcRequest;
            packet->threadData[i].pstateCmd = profile->threadData[i].pstateCmd;
            packet->threadData[i].corePerfBoost = profile->threadData[i].corePerfBoost;
        }
    }
#ifdef __linux__
    void ProfileDiskManager::serializeLinuxAMDData(QDataStream &ds, const PWTS::ClientPacket &packet) const {
        ds << linuxAmdDataVersion <<
            packet.linuxAmdData->pstateStatus;

        ds << packet.linuxAmdData->threadData.size();

        for (const auto &thdData: packet.linuxAmdData->threadData)
            serializeLinuxAMDThreadData(ds, thdData);
    }

    void ProfileDiskManager::serializeLinuxAMDThreadData(QDataStream &ds, const PWTS::LNX::AMD::LinuxAMDThreadData &thdData) const {
        ds << thdData.epp;
    }

    void ProfileDiskManager::deserializeLinuxAMDData(QDataStream &ds, DiskData &profile) const {
        qsizetype numLAThreads;
        int version;

        profile.linuxAmdD = QSharedPointer<PWTS::LNX::AMD::LinuxAMDData>::create();

        ds >> version >>
            profile.linuxAmdD->pstateStatus;

        ds >> numLAThreads;

        for (qsizetype i=0; i<numLAThreads; ++i)
            deserializeLinuxAMDThreadData(ds, profile, version);
    }

    void ProfileDiskManager::deserializeLinuxAMDThreadData(QDataStream &ds, const DiskData &profile, const int version) const {
        PWTS::LNX::AMD::LinuxAMDThreadData thdData {};

        ds >> thdData.epp;

        profile.linuxAmdD->threadData.append(thdData);
    }

    void ProfileDiskManager::loadLinuxAMDProfileToDaemonPacket(const QSharedPointer<PWTS::LNX::AMD::LinuxAMDData> &profile, const QSharedPointer<PWTS::LNX::AMD::LinuxAMDData> &packet) const {
        packet->pstateStatus = profile->pstateStatus;

        for (int i=0,l=profile->threadData.size(); i<l; ++i) {
            packet->threadData[i].epp = profile->threadData[i].epp;
        }
    }
#endif
#endif

    void ProfileDiskManager::serializeFanData(QDataStream &ds, const PWTS::ClientPacket &packet) const {
        ds << fanDataVersion <<
            packet.fanData;
    }

    void ProfileDiskManager::deserializeFanData(QDataStream &ds, DiskData &profile) const {
        int version;

        ds >> version >>
            profile.fanD;
    }

    QByteArray ProfileDiskManager::getProfileData(const QString &profile) const {
        QFile profileF {profile};
        QDataStream ds(&profileF);
        QString psignature;
        int fversion;
        QByteArray hash;
        QByteArray checksum;
        QByteArray data;

        if (!profileF.open(QFile::ReadOnly)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("%1: failed to open file: %2").arg(profileF.fileName(), profileF.errorString()));

            return {};
        }

        ds >> psignature;
        if (psignature != signature) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("%1: failed to load, invalid signature").arg(profileF.fileName()));

            return {};
        }

        ds >> fversion;
        if (fversion > fileVersion) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("%1: failed to load, future version %2 vs %3").arg(profileF.fileName()).arg(fversion).arg(fileVersion));

            return {};
        }

        ds >> hash;
        if (hash != deviceHash) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("%1: device hash mismatch, cannot load").arg(profileF.fileName()));

            return {};
        }

        ds >> checksum >> data;
        if (checksum != QCryptographicHash::hash(data, QCryptographicHash::Sha256)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("%1: checksum failed, cannot load").arg(profileF.fileName()));

            return {};
        }

        return data;
    }

    ProfileDiskManager::DiskData ProfileDiskManager::getDiskData(QByteArray &data) const {
        QDataStream ds(&data, QIODevice::ReadOnly);
        DiskData profile {};

#ifdef __linux__
        deserializeLinuxData(ds, profile);
#elif defined(_WIN32)
        deserializeWindowsData(ds, profile);
#endif
        deserializeFanData(ds, profile);

        switch (cpuVendor) {
#ifdef WITH_INTEL
            case PWTS::CPUVendor::Intel:
                deserializeIntelData(ds, profile);
                break;
#endif
#ifdef WITH_AMD
            case PWTS::CPUVendor::AMD: {
                deserializeAMDData(ds, profile);
#ifdef __linux__
                deserializeLinuxAMDData(ds, profile);
#endif
            }
                break;
#endif
            default:
                break;
        }

        return profile;
    }

    QByteArray ProfileDiskManager::createProfileFromPacket(const PWTS::ClientPacket &packet) const {
        QByteArray data;
        QByteArray file;
        QDataStream ds(&data, QIODevice::WriteOnly);
        QDataStream fds(&file, QIODevice::WriteOnly);

#ifdef __linux__
        serializeLinuxData(ds, packet);
#elif defined(_WIN32)
        serializeWindowsData(ds, packet);
#endif
        serializeFanData(ds, packet);

        switch (cpuVendor) {
#ifdef WITH_INTEL
            case PWTS::CPUVendor::Intel:
                serializeIntelData(ds, packet);
                break;
#endif
#ifdef WITH_AMD
            case PWTS::CPUVendor::AMD: {
                serializeAMDData(ds, packet);
#ifdef __linux__
                serializeLinuxAMDData(ds, packet);
#endif
            }
                break;
#endif
            default:
                break;
        }

        fds << signature <<
                fileVersion <<
                deviceHash <<
                QCryptographicHash::hash(data, QCryptographicHash::Sha256) <<
                data;

        return file;
    }

    bool ProfileDiskManager::load(const QString &name, PWTS::ClientPacket &packet) const {
        if (path.isEmpty()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("profiles folder is not available, cannot load %1").arg(name));

            return false;
        }

        QByteArray data = getProfileData(getFilePath(name));
        DiskData profile;

        if (data.isEmpty())
            return false;

        profile = getDiskData(data);

#ifdef __linux__
        packet.linuxData = profile.linuxD;
#elif defined(_WIN32)
        packet.windowsData = profile.windowsD;
#endif
#ifdef WITH_INTEL
        packet.intelData = profile.intelD;
#endif
#ifdef WITH_AMD
        packet.amdData = profile.amdD;
#ifdef __linux__
        packet.linuxAmdData = profile.linuxAmdD;
#endif
#endif
        packet.fanData = profile.fanD;

        return true;
    }

    bool ProfileDiskManager::load(const QString &name, PWTS::DaemonPacket &packet) const {
        if (path.isEmpty()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("profiles folder is not available, cannot load %1").arg(name));

            return false;
        }

        QByteArray data = getProfileData(getFilePath(name));
        DiskData profile;

        if (data.isEmpty())
            return false;

        profile = getDiskData(data);

        packet.fanData = profile.fanD;

#ifdef __linux__
        if (profile.linuxD->threadData.size() != packet.linuxData->threadData.size()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("%1: linux thread data count mismatch").arg(name));

            return false;
        }

        loadLinuxProfileToDaemonPacket(profile.linuxD, packet.linuxData);
#elif defined(_WIN32)
        loadWindowsProfileToDaemonPacket(profile.windowsD, packet.windowsData);
#endif

        switch (cpuVendor) {
#ifdef WITH_INTEL
            case PWTS::CPUVendor::Intel: {
                if (profile.intelD->coreData.size() != packet.intelData->coreData.size()) {
                    if (logger->isLevel(PWTS::LogLevel::Error))
                        logger->write(QString("%1: intel core data count mismatch").arg(name));

                    return false;
                }

                if (profile.intelD->threadData.size() != packet.intelData->threadData.size()) {
                    if (logger->isLevel(PWTS::LogLevel::Error))
                        logger->write(QString("%1: intel thread data count mismatch").arg(name));

                    return false;
                }

                loadIntelProfileToDaemonPacket(profile.intelD, packet.intelData);
            }
                break;
#endif
#ifdef WITH_AMD
            case PWTS::CPUVendor::AMD: {
                if (profile.amdD->coreData.size() != packet.amdData->coreData.size()) {
                    if (logger->isLevel(PWTS::LogLevel::Error))
                        logger->write(QString("%1: amd core data count mismatch").arg(name));

                    return false;
                }

                if (profile.amdD->threadData.size() != packet.amdData->threadData.size()) {
                    if (logger->isLevel(PWTS::LogLevel::Error))
                        logger->write(QString("%1: amd thread data count mismatch").arg(name));

                    return false;
                }

                loadAMDProfileToDaemonPacket(profile.amdD, packet.amdData);
#ifdef __linux__
                if (profile.linuxAmdD->threadData.size() != packet.linuxAmdData->threadData.size()) {
                    if (logger->isLevel(PWTS::LogLevel::Error))
                        logger->write(QString("%1: linux amd thread data count mismatch").arg(name));

                    return false;
                }

                loadLinuxAMDProfileToDaemonPacket(profile.linuxAmdD, packet.linuxAmdData);
#endif
            }
                break;
#endif
            default:
                break;
        }

        return true;
    }

    bool ProfileDiskManager::save(const QString &name, const PWTS::ClientPacket &packet) const {
        if (path.isEmpty())
            return false;

        return PWTS::writeFile(getFilePath(name), createProfileFromPacket(packet));
    }

    bool ProfileDiskManager::destroy(const QString &name) const {
        if (path.isEmpty())
            return false;

        return QFile(getFilePath(name)).remove();
    }

    QList<QString> ProfileDiskManager::getProfilesList() const {
        if (path.isEmpty())
            return {};

        const QDirListing dit(path, {QString("*.%1").arg(ext)}, QDirListing::IteratorFlag::FilesOnly);
        QList<QString> list;

        for (const QDirListing::DirEntry &entry: dit)
            list.append(entry.baseName());

        return list;
    }

    QHash<QString, QByteArray> ProfileDiskManager::exportProfiles(const QString &name) const {
        if (path.isEmpty())
            return {};

        QHash<QString, QByteArray> exported;

        if (name.toLower() != "all") {
            QFile profile {getFilePath(name)};

            if (!profile.open(QFile::ReadOnly))
                return {};

            exported.insert(QString("%1.%2").arg(name, ext), profile.readAll());

        } else {
            const QDirListing dit(path, {QString("*.%1").arg(ext)}, QDirListing::IteratorFlag::FilesOnly);

            for (const QDirListing::DirEntry &entry: dit) {
                QFile profile {entry.filePath()};

                if (profile.open(QFile::ReadOnly))
                    exported.insert(entry.fileName(), profile.readAll());
            }
        }

        return exported;
    }

    bool ProfileDiskManager::importProfile(const QString &name, const QByteArray &data) const {
        if (path.isEmpty() || name.isEmpty()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("%1: failed to import: invalid file name or no profiles folder").arg(name));

            return false;
        }

        QFile profile {getFilePath(name)};
        QDataStream ds(data);
        QString psignature;
        qint64 written;

        ds >> psignature;
        if (psignature != signature) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("%1: failed to import: invalid signature").arg(name));

            return false;
        }

        if (!profile.open(QFile::WriteOnly | QFile::Truncate)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("%1: failed to import: %2").arg(name, profile.errorString()));

            return false;
        }

        written = profile.write(data);

        return written != -1 && written == data.size();
    }

    void ProfileDiskManager::onDirChanged(const QString &fsPath) const {
        if (fsPath == path)
            fsWatcherEvtTimer->start();
    }

    void ProfileDiskManager::onFsWatcherTimerTimeout() {
        emit profileDiskChanged(getProfilesList());
    }
}
