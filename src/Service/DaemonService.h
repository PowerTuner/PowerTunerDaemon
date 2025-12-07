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

#include <QTimer>
#include <QThread>

#include "Workers/ServiceWorker.h"
#include "../Device/Device.h"
#include "../DiskManagers/ProfileDiskManager.h"
#include "../DiskManagers/DaemonSettingDiskManager.h"
#include "PowerNotifications/PowerNotifications.h"
#include "pwtShared/DaemonSettings.h"

namespace PWTD {
    class DaemonService final: public QObject {
        Q_OBJECT

    private:
        mutable std::optional<PWTS::ClientPacket> lastClientPacket;
        mutable QString activeProfile;
        QSharedPointer<FileLogger> logger;
        QSharedPointer<Device> device;
        QScopedPointer<ProfileDiskManager> profileDiskMan;
        QScopedPointer<PWTS::DaemonSettings> daemonSettings;
        QSharedPointer<DaemonSettingDiskManager> daemonSettingDiskMan;
		QSharedPointer<PowerNotifications> powerNotifications;
        mutable QScopedPointer<QTimer> applyTimer;
        QThread *serviceThread = nullptr;
        ServiceWorker *serviceWorker = nullptr;

        [[nodiscard]] QHostAddress getListenAddress(const QString &adr) const;
        [[nodiscard]] quint16 getServerPort(quint16 port) const;
        [[nodiscard]] bool hasValidMessageArgs(const QList<QVariant> &args) const;
        [[nodiscard]] bool isValidClientPacket(const PWTS::ClientPacket &packet) const;
        void setApplyTimer(int interval) const;
        void stopApplyTimer() const;
        void startApplyTimer() const;
        void writeErrorsToLog(const QSet<PWTS::DError> &errors) const;
        PWTS::DeviceInfoPacket createDeviceInfoPacket() const;
        PWTS::DaemonPacket createDaemonPacket() const;
        void applyClientSettings(const PWTS::ClientPacket &packet);
        [[nodiscard]] QSet<PWTS::DError> applyProfileSettings(const QString &name) const;
        void loadProfile(const QString &name);
        void importProfiles(const QByteArray &profilesData);
        void applyDaemonSettings(const QByteArray &data);

    public:
        DaemonService();
        ~DaemonService() override;

        void start(bool hasServer, const QString &adr, quint16 port);
        void reload(bool hasServer, const QString &adr, quint16 port);

    private slots:
        void onLogMessageSent(const QString &msg, PWTS::LogLevel lvl) const;
        void onCmdReceived(const QList<QVariant> &args);
        void onApplyTimerTimeout();
        void onBatteryStatusChanged(bool onBattery);
        void onPrepareForSleepEventTriggered() const;
        void onWakeFromSleepEventTriggered();

    signals:
        void sendError(PWTS::DError error);
        void sendCMDFail(PWTS::DCMD failedCMD);
        void connectService(const QHostAddress &adr, quint16 port);
        void restartService(const QHostAddress &adr, quint16 port);
        void stopService();
        void sendDeviceInfoPacket(const PWTS::DeviceInfoPacket &packet);
        void sendDaemonPacket(const PWTS::DaemonPacket &packet);
        void sendLoadedProfile(const PWTS::DaemonPacket &packet, const QString &name);
        void sendSettingsApplyResult(PWTS::DCMD cmd, const QSet<PWTS::DError> &errors, const QString &profileName = "");
        void sendExportedProfiles(const QHash<QString, QByteArray> &profiles);
        void sendProfileList(const QList<QString> &list);
        void sendCmdResult(PWTS::DCMD cmd, bool result);
        void sendByteArray(PWTS::DCMD cmd, const QByteArray &data);
    };
}
