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
#include "ProfileUtils/FAN/ProfileFanUtils.h"
#ifdef __linux__
#include "ProfileUtils/OS/ProfileLinuxUtils.h"
#elif defined(_WIN32)
#include "ProfileUtils/OS/ProfileWindowsUtils.h"
#endif
#ifdef WITH_INTEL
#include "ProfileUtils/Vendor/ProfileIntelUtils.h"
#endif
#ifdef WITH_AMD
#include "ProfileUtils/Vendor/ProfileAMDUtils.h"
#ifdef __linux__
#include "ProfileUtils/OS/ProfileLinuxAMDUtils.h"
#endif
#endif

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
        profile.linuxD = deserializeLinuxData(ds);
#elif defined(_WIN32)
        profile.windowsD = deserializeWindowsData(ds);
#endif
        profile.fanD = deserializeFanData(ds);

        switch (cpuVendor) {
#ifdef WITH_INTEL
            case PWTS::CPUVendor::Intel:
                profile.intelD = deserializeIntelData(ds);
                break;
#endif
#ifdef WITH_AMD
            case PWTS::CPUVendor::AMD: {
                profile.amdD = deserializeAMDData(ds);
#ifdef __linux__
                profile.linuxAmdD = deserializeLinuxAMDData(ds);
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
