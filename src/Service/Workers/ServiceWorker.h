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

#include <QTcpSocket>
#include <QTcpServer>
#include <QPointer>

#include "pwtShared/Include/Packets/DeviceInfoPacket.h"
#include "pwtShared/Include/Packets/DaemonPacket.h"
#include "pwtShared/Include/DaemonCMD.h"
#include "pwtShared/Include/LogLevel.h"

namespace PWTD {
    class ServiceWorker final: public QObject {
        Q_OBJECT

    private:
        QScopedPointer<QTcpServer> server;
        QPointer<QTcpSocket> sock;
        QDataStream sockStreamIn;

        [[nodiscard]] bool isSockOpen() const { return !sock.isNull() && sock->isOpen(); }

        [[nodiscard]] QByteArray packErrorList(const QSet<PWTS::DError> &errors);
        void sendData(const QList<QVariant> &args);

    public:
        ~ServiceWorker() override;

    private slots:
        void onNewConnection();
        void onDisconnected();
        void onReadyRead();

    public slots:
        void init();
        void startServer(const QHostAddress &adr, quint16 port);
        void restartServer(const QHostAddress &adr, quint16 port);
        void stopServer() const;
        void getNextPendingConnection();
        void sendError(PWTS::DError error);
        void sendCMDFail(PWTS::DCMD failedCMD);
        void sendDeviceInfoPacket(const PWTS::DeviceInfoPacket &packet);
        void sendDaemonPacket(const PWTS::DaemonPacket &packet);
        void sendSettingsApplyResult(PWTS::DCMD cmd, const QSet<PWTS::DError> &errors, const QString &profileName = "");
        void sendLoadedProfile(const PWTS::DaemonPacket &packet, const QString &name);
        void sendExportedProfiles(const QHash<QString, QByteArray> &profiles);
        void sendProfileList(const QList<QString> &list);
        void sendCmdResult(PWTS::DCMD cmd, bool result);
        void sendByteArray(PWTS::DCMD cmd, const QByteArray &data);

    signals:
        void logMessageSent(const QString &msg, PWTS::LogLevel lvl);
        void cmdReceived(const QList<QVariant> &args);
    };
}
