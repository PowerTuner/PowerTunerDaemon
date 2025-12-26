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
#include "../../version.h"
#include "DaemonService.h"
#include "../Utils/DaemonUtils.h"
#include "../Utils/AppDataPath.h"
#include "PowerNotifications/PowerNotificationsFactory.h"
#include "pwtShared/Utils.h"

namespace PWTD {
    DaemonService::DaemonService() {
        device = Device::getDevice();
        logger = FileLogger::getInstance();
		powerNotifications = PowerNotificationsFactory::getPowerNotifications();
        daemonSettingDiskMan = DaemonSettingDiskManager::getInstance();

        profileDiskMan.reset(new ProfileDiskManager(device->getDeviceHash(), device->getCPUVendor()));
        daemonSettings.reset(new PWTS::DaemonSettings);
    }

    DaemonService::~DaemonService() {
        stopApplyTimer();
        serviceThread->quit();
        serviceThread->wait();
        delete serviceThread;
    }

    QHostAddress DaemonService::getListenAddress(const QString &adr) const {
        if (adr.isEmpty())
            return QHostAddress(daemonSettings->getAddress());

        const QHostAddress addr = PWTS::getHostAddress(adr);

        return addr.isNull() ? QHostAddress::Any : addr;
    }

    quint16 DaemonService::getServerPort(const quint16 port) const {
        if (port == 0)
            return daemonSettings->getSocketTcpPort();

        return PWTS::isValidPort(port) ? port : PWTS::DaemonSettings::DefaultTCPPort;
    }

    bool DaemonService::hasValidMessageArgs(const QList<QVariant> &args) const {
        if (args.isEmpty())
            return false;

        switch (static_cast<PWTS::DCMD>(args[0].toInt())) {
            case PWTS::DCMD::APPLY_CLIENT_SETTINGS:
            case PWTS::DCMD::APPLY_PROFILE:
            case PWTS::DCMD::DELETE_PROFILE:
            case PWTS::DCMD::LOAD_PROFILE:
            case PWTS::DCMD::EXPORT_PROFILES:
            case PWTS::DCMD::IMPORT_PROFILES:
            case PWTS::DCMD::APPLY_DAEMON_SETT: {
                if (args.size() < 2)
                    return false;
            }
                break;
            case PWTS::DCMD::WRITE_PROFILE: {
                if (args.size() < 3)
                    return false;
            }
                break;
            case PWTS::DCMD::GET_DAEMON_SETTS: {
                if (args.size() > 1)
                    return false;
            }
                break;
            default:
                break;
        }

        return true;
    }

    bool DaemonService::isValidClientPacket(const PWTS::ClientPacket &packet) const {
        return packet.os == getOS() && packet.vendor == device->getCPUVendor();
    }

    void DaemonService::setApplyTimer(const int interval) const {
        if (interval <= PWTS::DaemonSettings::MinApplyInterval && !applyTimer.isNull()) {
            QObject::disconnect(applyTimer.get(), &QTimer::timeout, this, &DaemonService::onApplyTimerTimeout);
            applyTimer->stop();
            applyTimer.reset();

        } else if (interval > PWTS::DaemonSettings::MinApplyInterval) {
            if (applyTimer.isNull()) {
                applyTimer.reset(new QTimer);

                QObject::connect(applyTimer.get(), &QTimer::timeout, this, &DaemonService::onApplyTimerTimeout);
            }

            applyTimer->setInterval(daemonSettings->getApplyInterval() * 1000);
            applyTimer->start();
        }
    }

    void DaemonService::stopApplyTimer() const {
        if (applyTimer.isNull())
            return;

        applyTimer->stop();
    }

    void DaemonService::startApplyTimer() const {
        if (applyTimer.isNull())
            return;

        applyTimer->start();
    }

    void DaemonService::writeErrorsToLog(const QSet<PWTS::DError> &errors) const {
        if (!logger->isLevel(PWTS::LogLevel::Error))
            return;

        for (const PWTS::DError &e: errors)
            logger->write(PWTS::getErrorStr(e));
    }

    PWTS::DeviceInfoPacket DaemonService::createDeviceInfoPacket() const {
        PWTS::DeviceInfoPacket packet;

        packet.daemonMajorVersion = PWTD_VER_MAJOR;
        packet.daemonMinorVersion = PWTD_VER_MINOR;
        packet.daemonPwtsMajorVersion = PWTS::getLibMajorVersion();
        packet.daemonPwtsMinorVersion = PWTS::getLibMinorVersion();
        packet.daemonDataPath = AppDataPath::appDataLocation();
        packet.daemonSettings = daemonSettings->getData();
        packet.sysInfo = *(device->getSystemInfo());
        packet.dynSysInfo = device->getDynamicSystemInfo();
        packet.cpuInfo = *(device->getCPUInfo());
        packet.gpusInfo = device->getGPUInfoMap();
        packet.fanLabels = device->getFanLabelsMap();
        packet.features = device->getFeatures();

        return packet;
    }

    PWTS::DaemonPacket DaemonService::createDaemonPacket() const {
        PWTS::DaemonPacket packet;

        packet.os = getOS();
        packet.vendor = device->getCPUVendor();
        packet.dynSysInfo = device->getDynamicSystemInfo();
        packet.profilesList = profileDiskMan->getProfilesList();
        packet.activeProfile = activeProfile;

        device->fillPacketDeviceData(packet);
        return packet;
    }

    void DaemonService::applyClientSettings(const PWTS::ClientPacket &packet) {
        const QSet<PWTS::DError> errors = device->applySettings(packet);

        activeProfile.clear();
        lastClientPacket.reset();

        if (errors.isEmpty())
            lastClientPacket = packet;

        emit sendSettingsApplyResult(PWTS::DCMD::APPLY_CLIENT_SETTINGS, errors);
    }

    QSet<PWTS::DError> DaemonService::applyProfileSettings(const QString &name) const {
        PWTS::ClientPacket packet;

        if (!profileDiskMan->load(name, packet))
            return {PWTS::DError::PROFILE_LOAD_FAILED};

        const QSet<PWTS::DError> errors = device->applySettings(packet);

        if (errors.isEmpty()) {
            lastClientPacket.reset();

            activeProfile = name;
            lastClientPacket = packet;
        }

        return errors;
    }

    void DaemonService::loadProfile(const QString &name) {
        PWTS::DaemonPacket packet = createDaemonPacket();

        packet.hasProfileData = true;

        if (!profileDiskMan->load(name, packet)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("Failed to load profile %1").arg(name));

            emit sendError(PWTS::DError::PROFILE_LOAD_FAILED);
            emit sendCMDFail(PWTS::DCMD::LOAD_PROFILE);
            return;
        }

        if (logger->isLevel(PWTS::LogLevel::Info))
            logger->write(QString("Loaded profile: %1").arg(name));

        emit sendLoadedProfile(packet, name);
    }

    void DaemonService::importProfiles(const QByteArray &profilesData) {
        QHash<QString, QByteArray> profiles;
        bool res = true;

        if (!PWTS::unpackData<QHash<QString, QByteArray>>(profilesData, profiles)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("Failed to unpack profiles data for import"));

            return;
        }

        QHashIterator it {profiles};

        while (it.hasNext()) {
            it.next();

            if (!profileDiskMan->importProfile(it.key(), it.value()))
                res = false;
            else if (logger->isLevel(PWTS::LogLevel::Info))
                logger->write(QString("imported profile: %1").arg(it.key()));
        }

        emit sendCmdResult(PWTS::DCMD::IMPORT_PROFILES, res);
    }

    void DaemonService::applyDaemonSettings(const QByteArray &data) {
        const QString oldAdr = daemonSettings->getAddress();
        const quint16 oldPort = daemonSettings->getSocketTcpPort();

        if (!daemonSettings->load(data)) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("Unable to load daemon settings from data, cannot apply settings!"));

            emit sendCmdResult(PWTS::DCMD::APPLY_DAEMON_SETT, false);
            return;
        }

        logger->setLevel(daemonSettings->getLogLevel());

        if (oldAdr != daemonSettings->getAddress() || oldPort != daemonSettings->getSocketTcpPort())
            emit restartService(QHostAddress(daemonSettings->getAddress()), daemonSettings->getSocketTcpPort());

        setApplyTimer(daemonSettings->getApplyInterval());

        if (logger->isLevel(PWTS::LogLevel::Info))
            logger->write(QStringLiteral("Daemon settings received and applied from client, saving.."));

        emit sendCmdResult(PWTS::DCMD::APPLY_DAEMON_SETT, daemonSettingDiskMan->save(data));
    }

    void DaemonService::start(const bool hasServer, const QString &adr, const quint16 port) {
        if (!daemonSettings->load(daemonSettingDiskMan->load()))
            qWarning("Failed to load daemon settings, using defaults");

        if (logger->isLevel(PWTS::LogLevel::Service)) {
            logger->write(QJsonDocument(PWTS::getDeviceInfoJson(createDeviceInfoPacket())).toJson().toStdString().c_str());
            logger->write(QString("Profiles directory: %1").arg(profileDiskMan->getPath()));
        }

        if (hasServer) {
            serviceWorker = new ServiceWorker();
            serviceThread = new QThread();

            serviceWorker->moveToThread(serviceThread);

            QObject::connect(serviceThread, &QThread::started, serviceWorker, &ServiceWorker::init);
            QObject::connect(serviceThread, &QThread::finished, serviceWorker, &QObject::deleteLater);
            QObject::connect(serviceWorker, &ServiceWorker::logMessageSent, this, &DaemonService::onLogMessageSent);
            QObject::connect(serviceWorker, &ServiceWorker::cmdReceived, this, &DaemonService::onCmdReceived);
            QObject::connect(this, &DaemonService::connectService, serviceWorker, &ServiceWorker::startServer);
            QObject::connect(this, &DaemonService::restartService, serviceWorker, &ServiceWorker::restartServer);
            QObject::connect(this, &DaemonService::stopService, serviceWorker, &ServiceWorker::stopServer);
            QObject::connect(this, &DaemonService::sendError, serviceWorker, &ServiceWorker::sendError);
            QObject::connect(this, &DaemonService::sendCMDFail, serviceWorker, &ServiceWorker::sendCMDFail);
            QObject::connect(this, &DaemonService::sendDeviceInfoPacket, serviceWorker, &ServiceWorker::sendDeviceInfoPacket);
            QObject::connect(this, &DaemonService::sendDaemonPacket, serviceWorker, &ServiceWorker::sendDaemonPacket);
            QObject::connect(this, &DaemonService::sendSettingsApplyResult, serviceWorker, &ServiceWorker::sendSettingsApplyResult);
            QObject::connect(this, &DaemonService::sendLoadedProfile, serviceWorker, &ServiceWorker::sendLoadedProfile);
            QObject::connect(this, &DaemonService::sendExportedProfiles, serviceWorker, &ServiceWorker::sendExportedProfiles);
            QObject::connect(this, &DaemonService::sendProfileList, serviceWorker, &ServiceWorker::sendProfileList);
            QObject::connect(this, &DaemonService::sendCmdResult, serviceWorker, &ServiceWorker::sendCmdResult);
            QObject::connect(this, &DaemonService::sendByteArray, serviceWorker, &ServiceWorker::sendByteArray);
            QObject::connect(profileDiskMan.get(), &ProfileDiskManager::profileDiskChanged, serviceWorker, &ServiceWorker::sendProfileList);

            serviceThread->start();
            emit connectService(getListenAddress(adr), getServerPort(port));
        }

        if (!daemonSettings->getOnStartProfile().isEmpty())
            writeErrorsToLog(applyProfileSettings(daemonSettings->getOnStartProfile()));

		if (!powerNotifications.isNull()) {
			powerNotifications->initNotifications();
			QObject::connect(powerNotifications.get(), &PowerNotifications::batteryStatusChanged, this, &DaemonService::onBatteryStatusChanged);
			QObject::connect(powerNotifications.get(), &PowerNotifications::prepareForSleepTriggered, this, &DaemonService::onPrepareForSleepEventTriggered);
			QObject::connect(powerNotifications.get(), &PowerNotifications::wakeFromSleepEventTriggered, this, &DaemonService::onWakeFromSleepEventTriggered);
		}

        setApplyTimer(daemonSettings->getApplyInterval());
    }

    void DaemonService::reload(const bool hasServer, const QString &adr, const quint16 port) {
        stopApplyTimer();

        if (hasServer)
            emit stopService();

        if (!daemonSettings->load(daemonSettingDiskMan->load()))
            qWarning("Failed to load daemon settings, using defaults");

        logger->init(daemonSettings->getLogLevel(), daemonSettings->getMaxLogFiles());

        if (logger->isLevel(PWTS::LogLevel::Service))
            logger->write(QString("Profiles directory: %1").arg(profileDiskMan->getPath()));

        if (hasServer)
            emit connectService(getListenAddress(adr), getServerPort(port));

        if (!daemonSettings->getOnStartProfile().isEmpty())
            writeErrorsToLog(applyProfileSettings(daemonSettings->getOnStartProfile()));

        setApplyTimer(daemonSettings->getApplyInterval());
    }

    void DaemonService::onLogMessageSent(const QString &msg, const PWTS::LogLevel lvl) const {
        if (logger->isLevel(lvl))
            logger->write(msg);
    }

    void DaemonService::onCmdReceived(const QList<QVariant> &args) {
        if (!hasValidMessageArgs(args)) {
            emit sendError(PWTS::DError::INVALID_ARGS);
            return;
        }

        stopApplyTimer();

        const PWTS::DCMD cmd = static_cast<PWTS::DCMD>(args[0].toInt());

        switch (cmd) {
            case PWTS::DCMD::GET_DEVICE_INFO_PACKET:
                emit sendDeviceInfoPacket(createDeviceInfoPacket());
                break;
            case PWTS::DCMD::GET_DAEMON_PACKET:
                emit sendDaemonPacket(createDaemonPacket());
                break;
            case PWTS::DCMD::APPLY_CLIENT_SETTINGS: {
                if (!args[1].canConvert<PWTS::ClientPacket>()) {
                    emit sendError(PWTS::DError::CORRUPTED_DATA);
                    emit sendCMDFail(cmd);
                    break;
                }

                const PWTS::ClientPacket packet = args[1].value<PWTS::ClientPacket>();

                if (!isValidClientPacket(packet)) {
                    emit sendError(PWTS::DError::INVALID_PACKET);
                    emit sendCMDFail(cmd);
                    break;

                } else if (packet.error != PWTS::PacketError::NoError) {
                    if (logger->isLevel(PWTS::LogLevel::Error))
                        logger->write(QString("client packet error: %1").arg(PWTS::getPacketErrorStr(packet.error)));

                    emit sendError(PWTS::DError::INVALID_PACKET);
                    emit sendCMDFail(cmd);
                    break;
                }

                applyClientSettings(packet);
            }
                break;
            case PWTS::DCMD::APPLY_PROFILE: {
                const QString profile = args[1].toString();

                emit sendSettingsApplyResult(PWTS::DCMD::APPLY_PROFILE, applyProfileSettings(profile), profile);
            }
                break;
            case PWTS::DCMD::WRITE_PROFILE: {
                if (!args[2].canConvert<PWTS::ClientPacket>()) {
                    emit sendError(PWTS::DError::CORRUPTED_DATA);
                    emit sendCMDFail(cmd);
                    break;
                }

                const QString profile = args[1].toString();
                const PWTS::ClientPacket packet = args[2].value<PWTS::ClientPacket>();

                if (!isValidClientPacket(packet)) {
                    emit sendError(PWTS::DError::INVALID_PACKET);
                    emit sendCMDFail(cmd);
                    break;
                }

                emit sendCmdResult(PWTS::DCMD::WRITE_PROFILE, profileDiskMan->save(profile, packet));
            }
                break;
            case PWTS::DCMD::DELETE_PROFILE:
                emit sendCmdResult(PWTS::DCMD::DELETE_PROFILE, profileDiskMan->destroy(args[1].toString()));
                break;
            case PWTS::DCMD::LOAD_PROFILE:
                loadProfile(args[1].toString());
                break;
            case PWTS::DCMD::GET_PROFILE_LIST:
                emit sendProfileList(profileDiskMan->getProfilesList());
                break;
            case PWTS::DCMD::EXPORT_PROFILES:
                emit sendExportedProfiles(profileDiskMan->exportProfiles(args[1].toString()));
                break;
            case PWTS::DCMD::IMPORT_PROFILES:
                importProfiles(args[1].toByteArray());
                break;
            case PWTS::DCMD::GET_DAEMON_SETTS:
                emit sendByteArray(PWTS::DCMD::GET_DAEMON_SETTS, daemonSettings->getData());
                break;
            case PWTS::DCMD::APPLY_DAEMON_SETT:
                applyDaemonSettings(args[1].toByteArray());
                break;
            default: {
                emit sendError(PWTS::DError::INVALID_DCMD);
                emit sendCMDFail(cmd);
            }
                break;
        }

        startApplyTimer();
    }

    void DaemonService::onApplyTimerTimeout() {
        applyTimer->stop();

        if (!lastClientPacket.has_value()) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("no data, stopping.."));

            return;
        }

        const QSet<PWTS::DError> errors = device->applySettings(lastClientPacket.value());

        if (logger->isLevel(PWTS::LogLevel::Info))
            logger->write(QStringLiteral("applying settings.."));

        writeErrorsToLog(errors);
        emit sendSettingsApplyResult(PWTS::DCMD::APPLY_TIMER, errors);
        applyTimer->start();
    }

    void DaemonService::onBatteryStatusChanged(const bool onBattery) {
        if (daemonSettings->getIgnoreBatteryEvent())
            return;

        const QString profile = onBattery ? daemonSettings->getOnBatteryProfile() : daemonSettings->getOnPowerSupplyProfile();

        if (profile.isEmpty())
            return;

        stopApplyTimer();

        const QSet<PWTS::DError> errors = applyProfileSettings(profile);

        if (logger->isLevel(PWTS::LogLevel::Info))
            logger->write(QString("Battery status change: on battery: %1, profile: %2").arg(onBattery).arg(profile));

        writeErrorsToLog(errors);
        emit sendSettingsApplyResult(PWTS::DCMD::BATTERY_STATUS_CHANGED, errors, profile);
        startApplyTimer();
    }

    void DaemonService::onPrepareForSleepEventTriggered() const {
        device->prepareForSleep();
    }

    void DaemonService::onWakeFromSleepEventTriggered() {
        const QList<QVariant> refreshArgs {static_cast<int>(PWTS::DCMD::GET_DAEMON_PACKET), false};

		if (daemonSettings->getApplyOnWakeFromSleep() && lastClientPacket.has_value()) {
		    stopApplyTimer();

		    const QSet<PWTS::DError> errors = device->applySettings(lastClientPacket.value());

		    if (logger->isLevel(PWTS::LogLevel::Info))
		        logger->write(QStringLiteral("Wake from sleep: applying settings"));

		    writeErrorsToLog(errors);
		    emit sendSettingsApplyResult(PWTS::DCMD::SYS_WAKE_FROM_SLEEP, errors);
		    startApplyTimer();
		}

        // force refresh client, things may have changed
        onCmdReceived(refreshArgs);
    }
}
