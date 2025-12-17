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
        QScopedPointer<QFileSystemWatcher> fsWatcher;
        QScopedPointer<QTimer> fsWatcherEvtTimer;
        QSharedPointer<FileLogger> logger;
        PWTS::CPUVendor cpuVendor;
        QByteArray deviceHash;
        QString path;

        [[nodiscard]] QString getFilePath(const QString &name) const { return QString("%1/%2.%3").arg(path, name, ext); }

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
