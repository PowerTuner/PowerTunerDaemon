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
#include "ServiceWorker.h"
#include "pwtShared/Utils.h"

namespace PWTD {
    ServiceWorker::~ServiceWorker() {
        if (!sock.isNull())
            sock->abort();

        if (!server.isNull())
            server->close();
    }

    void ServiceWorker::init() {
        server.reset(new QTcpServer);
    }

    QByteArray ServiceWorker::packErrorList(const QSet<PWTS::DError> &errors) {
        QByteArray data;

        if (!PWTS::packData<QSet<PWTS::DError>>(errors, data)) {
            emit logMessageSent(QStringLiteral("Failed to pack error list"), PWTS::LogLevel::Error);
            return {};
        }

        return data;
    }

    void ServiceWorker::sendData(const QList<QVariant> &args) {
        QByteArray data;

        if (!PWTS::packData<QList<QVariant>>(args, data)) {
            emit logMessageSent(QString("sendData: failed to pack data for cmd %1").arg(args[0].toInt()), PWTS::LogLevel::Error);
            return;
        }

        sock->write(data);
        sock->flush();
    }

    void ServiceWorker::startServer(const QHostAddress &adr, const quint16 port) {
        const bool res = server->listen(adr, port);
        const QString sadr = server->serverAddress().toString();

        if (res) {
            QObject::connect(server.get(), &QTcpServer::newConnection, this, &ServiceWorker::onNewConnection);

            emit logMessageSent(QString("Listening on %1:%2").arg(sadr).arg(port), PWTS::LogLevel::Service);

        } else {
            emit logMessageSent(QString("Failed to listen on %1:%2").arg(sadr).arg(port), PWTS::LogLevel::Error);
        }
    }

    void ServiceWorker::restartServer(const QHostAddress &adr, const quint16 port) {
        QObject::disconnect(server.get(), &QTcpServer::newConnection, this, &ServiceWorker::onNewConnection);

        if (server->isListening())
            server->close();

        if (!sock.isNull()) {
            if (sock->isOpen())
                sock->abort();

            sock->deleteLater();
        }

        startServer(adr, port);
    }

    void ServiceWorker::stopServer() const {
        QObject::disconnect(server.get(), &QTcpServer::newConnection, this, &ServiceWorker::onNewConnection);

        if (server->isListening())
            server->close();

        if (!sock.isNull()) {
            sock->abort();
            sock->deleteLater();
        }
    }

    void ServiceWorker::getNextPendingConnection() {
        if (!server->hasPendingConnections())
            return;

        sock = server->nextPendingConnection();

        sockStreamIn.setDevice(sock);

        emit logMessageSent(QString("Connected to %1").arg(sock->peerAddress().toString()), PWTS::LogLevel::Info);

        QObject::connect(sock, &QTcpSocket::disconnected, this, &ServiceWorker::onDisconnected);
        QObject::connect(sock, &QTcpSocket::readyRead, this, &ServiceWorker::onReadyRead);
    }

    void ServiceWorker::sendError(const PWTS::DError error) {
        if (!isSockOpen()) {
            emit logMessageSent(QStringLiteral("ServiceWorker::sendError: socket not available"), PWTS::LogLevel::Error);
            return;
        }

        const QList<QVariant> args {static_cast<int>(PWTS::DCMD::PRINT_ERROR), static_cast<int>(error)};

        sendData(args);
    }

    void ServiceWorker::sendCMDFail(const PWTS::DCMD failedCMD) {
        if (!isSockOpen()) {
            emit logMessageSent(QStringLiteral("ServiceWorker::sendCMDFail: socket not available"), PWTS::LogLevel::Error);
            return;
        }

        const QList<QVariant> args {static_cast<int>(PWTS::DCMD::DAEMON_CMD_FAIL), static_cast<int>(failedCMD)};

        sendData(args);
    }

    void ServiceWorker::sendDeviceInfoPacket(const PWTS::DeviceInfoPacket &packet) {
        if (!isSockOpen()) {
            emit logMessageSent(QStringLiteral("ServiceWorker::sendDeviceInfoPacket: socket not available"), PWTS::LogLevel::Error);
            return;
        }

        const QList<QVariant> args {static_cast<int>(PWTS::DCMD::GET_DEVICE_INFO_PACKET), QVariant::fromValue<PWTS::DeviceInfoPacket>(packet)};

        sendData(args);
    }

    void ServiceWorker::sendDaemonPacket(const PWTS::DaemonPacket &packet) {
        if (!isSockOpen()) {
            emit logMessageSent(QStringLiteral("ServiceWorker::sendDaemonPacket: socket not available"), PWTS::LogLevel::Error);
            return;
        }

        const QList<QVariant> args {static_cast<int>(PWTS::DCMD::GET_DAEMON_PACKET), QVariant::fromValue<PWTS::DaemonPacket>(packet)};

        sendData(args);
    }

    void ServiceWorker::sendSettingsApplyResult(const PWTS::DCMD cmd, const QSet<PWTS::DError> &errors, const QString &profileName) {
        if (!isSockOpen()) {
            emit logMessageSent(QStringLiteral("ServiceWorker::sendSettingsApplyResult: socket not available"), PWTS::LogLevel::Error);
            return;
        }

        const QByteArray errorsBA = packErrorList(errors);
        QList<QVariant> args;

        if (errorsBA.isEmpty())
            return;

        args.append(static_cast<int>(cmd));
        args.append(errorsBA);

        if (!profileName.isEmpty())
            args.append(profileName);

        sendData(args);
    }

    void ServiceWorker::sendLoadedProfile(const PWTS::DaemonPacket &packet, const QString &name) {
        if (!isSockOpen()) {
            emit logMessageSent(QStringLiteral("ServiceWorker::sendLoadedProfile: socket not available"), PWTS::LogLevel::Error);
            return;
        }

        const QList<QVariant> args {static_cast<int>(PWTS::DCMD::LOAD_PROFILE), QVariant::fromValue<PWTS::DaemonPacket>(packet), name};

        sendData(args);
    }

    void ServiceWorker::sendExportedProfiles(const QHash<QString, QByteArray> &profiles) {
        if (!isSockOpen()) {
            emit logMessageSent(QStringLiteral("ServiceWorker::sendExportedProfiles: socket not available"), PWTS::LogLevel::Error);
            return;
        }

        QByteArray exportedData;

        if (!PWTS::packData<QHash<QString, QByteArray>>(profiles, exportedData)) {
            emit logMessageSent(QStringLiteral("Failed to pack exported profiles"), PWTS::LogLevel::Error);
            return;
        }

        const QList<QVariant> args {static_cast<int>(PWTS::DCMD::EXPORT_PROFILES), exportedData};

        sendData(args);
    }

    void ServiceWorker::sendProfileList(const QList<QString> &list) {
        if (!isSockOpen()) {
            emit logMessageSent(QStringLiteral("ServiceWorker::sendProfileList: socket not available"), PWTS::LogLevel::Error);
            return;
        }

        const QList<QVariant> args {static_cast<int>(PWTS::DCMD::GET_PROFILE_LIST), list};

        sendData(args);
    }

    void ServiceWorker::sendCmdResult(PWTS::DCMD cmd, const bool result) {
        if (!isSockOpen()) {
            emit logMessageSent(QStringLiteral("ServiceWorker::sendCmdResult: socket not available"), PWTS::LogLevel::Error);
            return;
        }

        const QList<QVariant> args {static_cast<int>(cmd), result};

        sendData(args);
    }

    void ServiceWorker::sendByteArray(const PWTS::DCMD cmd, const QByteArray &data) {
        if (!isSockOpen()) {
            emit logMessageSent(QString("ServiceWorker::sendByteArray: cmd %1: socket not available").arg(static_cast<int>(cmd)), PWTS::LogLevel::Error);
            return;
        }

        const QList<QVariant> args {static_cast<int>(cmd), data};

        sendData(args);
    }

    void ServiceWorker::onNewConnection() {
        if (isSockOpen()) {
            emit logMessageSent(QStringLiteral("New client connection request, previous connection will be closed"), PWTS::LogLevel::Info);
            sock->abort();

        } else {
            getNextPendingConnection();
        }
    }

    void ServiceWorker::onDisconnected() {
        sock->deleteLater();
        emit logMessageSent(QStringLiteral("disconnected from client"), PWTS::LogLevel::Info);
        getNextPendingConnection();
    }

    void ServiceWorker::onReadyRead() {
        QList<QVariant> args;

        while (true) {
            if (!isSockOpen()) {
                emit logMessageSent(QStringLiteral("ServiceWorker::onReadyRead: socket is not available"), PWTS::LogLevel::Error);
                break;
            }

            sockStreamIn.startTransaction();
            sockStreamIn >> args;

            if (!sockStreamIn.commitTransaction())
                break;

            if (args.empty()) {
                sendError(PWTS::DError::CORRUPTED_DATA);
                break;
            }

            emit cmdReceived(args);
        }
    }
}
