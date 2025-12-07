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

#include <QFileSystemWatcher>
#include <QTimer>
#include <QHash>

#include "pwtShared/Include/Packets/ClientPacket.h"
#include "pwtShared/Include/Packets/DaemonPacket.h"
#include "../Utils/FileLogger/FileLogger.h"

namespace PWTD {
    class ProfileDiskManager final: public QObject {
        Q_OBJECT

    private:
        struct DiskData final {
#ifdef __linux__
            QSharedPointer<PWTS::LNX::LinuxData> linuxD;
#elif defined(_WIN32)
            QSharedPointer<PWTS::WIN::WindowsData> windowsD;
#endif
#ifdef WITH_INTEL
            QSharedPointer<PWTS::Intel::IntelData> intelD;
#endif
#ifdef WITH_AMD
            QSharedPointer<PWTS::AMD::AMDData> amdD;
#ifdef __linux__
            QSharedPointer<PWTS::LNX::AMD::LinuxAMDData> linuxAmdD;
#endif
#endif
            QMap<QString, PWTS::FanData> fanD;
        };

        static constexpr char ext[] = "pwt";
        const QString signature = "PWTPF";
        static constexpr int fileVersion = 1;
        static constexpr int linuxDataVersion = 1;
        static constexpr int windowsDataVersion = 1;
        static constexpr int intelDataVersion = 1;
        static constexpr int amdDataVersion = 1;
        static constexpr int linuxAmdDataVersion = 1;
        static constexpr int fanDataVersion = 1;
        QScopedPointer<QFileSystemWatcher> fsWatcher;
        QScopedPointer<QTimer> fsWatcherEvtTimer;
        QSharedPointer<FileLogger> logger;
        PWTS::CPUVendor cpuVendor;
        QByteArray deviceHash;
        QString path;

        [[nodiscard]] QString getFilePath(const QString &name) const { return QString("%1/%2.%3").arg(path, name, ext); }

#ifdef __linux__
        void serializeLinuxData(QDataStream &ds, const PWTS::ClientPacket &packet) const;
        void serializeLinuxThreadData(QDataStream &ds, const PWTS::LNX::LinuxThreadData &thdData) const;
        void deserializeLinuxData(QDataStream &ds, DiskData &profile) const;
        void deserializeLinuxThreadData(QDataStream &ds, const DiskData &profile, int version) const;
        void loadLinuxProfileToDaemonPacket(const QSharedPointer<PWTS::LNX::LinuxData> &profile, const QSharedPointer<PWTS::LNX::LinuxData> &packet) const;
#elif defined(_WIN32)
        void serializeWindowsData(QDataStream &ds, const PWTS::ClientPacket &packet) const;
        void deserializeWindowsData(QDataStream &ds, DiskData &profile) const;
        void loadWindowsProfileToDaemonPacket(const QSharedPointer<PWTS::WIN::WindowsData> &profile, const QSharedPointer<PWTS::WIN::WindowsData> &packet) const;
#endif
#ifdef WITH_INTEL
        void serializeIntelData(QDataStream &ds, const PWTS::ClientPacket &packet) const;
        void serializeIntelCoreData(QDataStream &ds, const PWTS::Intel::IntelCoreData &coreData) const;
        void serializeIntelThreadData(QDataStream &ds, const PWTS::Intel::IntelThreadData &thdData) const;
        void deserializeIntelData(QDataStream &ds, DiskData &profile) const;
        void deserializeIntelCoreData(QDataStream &ds, const DiskData &profile, int version) const;
        void deserializeIntelThreadData(QDataStream &ds, const DiskData &profile, int version) const;
        void loadIntelProfileToDaemonPacket(const QSharedPointer<PWTS::Intel::IntelData> &profile, const QSharedPointer<PWTS::Intel::IntelData> &packet) const;
#endif
#ifdef WITH_AMD
        void serializeAMDData(QDataStream &ds, const PWTS::ClientPacket &packet) const;
        void serializeAMDCoreData(QDataStream &ds, const PWTS::AMD::AMDCoreData &coreData) const;
        void serializeAMDThreadData(QDataStream &ds, const PWTS::AMD::AMDThreadData &thdData) const;
        void deserializeAMDData(QDataStream &ds, DiskData &profile) const;
        void deserializeAMDCoreData(QDataStream &ds, const DiskData &profile, int version) const;
        void deserializeAMDThreadData(QDataStream &ds, const DiskData &profile, int version) const;
        void loadAMDProfileToDaemonPacket(const QSharedPointer<PWTS::AMD::AMDData> &profile, const QSharedPointer<PWTS::AMD::AMDData> &packet) const;
#ifdef __linux__
        void serializeLinuxAMDData(QDataStream &ds, const PWTS::ClientPacket &packet) const;
        void serializeLinuxAMDThreadData(QDataStream &ds, const PWTS::LNX::AMD::LinuxAMDThreadData &thdData) const;
        void deserializeLinuxAMDData(QDataStream &ds, DiskData &profile) const;
        void deserializeLinuxAMDThreadData(QDataStream &ds, const DiskData &profile, int version) const;
        void loadLinuxAMDProfileToDaemonPacket(const QSharedPointer<PWTS::LNX::AMD::LinuxAMDData> &profile, const QSharedPointer<PWTS::LNX::AMD::LinuxAMDData> &packet) const;
#endif
#endif
        void serializeFanData(QDataStream &ds, const PWTS::ClientPacket &packet) const;
        void deserializeFanData(QDataStream &ds, DiskData &profile) const;
        [[nodiscard]] QByteArray getProfileData(const QString &profile) const;
        [[nodiscard]] DiskData getDiskData(QByteArray &data) const;
        [[nodiscard]] QByteArray createProfileFromPacket(const PWTS::ClientPacket &packet) const;

    public:
        ProfileDiskManager(const QByteArray &hash, PWTS::CPUVendor vendor);

        [[nodiscard]] QString getPath() const { return path; }

        [[nodiscard]] QList<QString> getProfilesList() const;
        [[nodiscard]] QHash<QString, QByteArray> exportProfiles(const QString &name) const;
        [[nodiscard]] bool importProfile(const QString &name, const QByteArray &data) const;
        [[nodiscard]] bool load(const QString &name, PWTS::ClientPacket &packet) const;
        [[nodiscard]] bool load(const QString &name, PWTS::DaemonPacket &packet) const;
        [[nodiscard]] bool save(const QString &name, const PWTS::ClientPacket &packet) const;
        [[nodiscard]] bool destroy(const QString &name) const;

    private slots:
        void onDirChanged(const QString &fsPath) const;
        void onFsWatcherTimerTimeout();

    signals:
        void profileDiskChanged(const QList<QString> &list);
    };
}
