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
#include  <QDir>

#ifdef __linux__
#include "Daemon/Linux/DaemonLinux.h"
#elif defined(_WIN32)
#include "Daemon/Windows/DaemonWindows.h"
#endif
#include "Device/Device.h"
#include "Utils/ProcessStatus.h"
#include "Utils/Utils.h"
#include "pwtShared/DaemonSettings.h"
#include "DiskManagers/DaemonSettingDiskManager.h"
#include "Utils/FileLogger/FileLogger.h"

[[nodiscard]]
static QString getAppDataPath() {
    const QList<QString> locations = {
#ifdef _WIN32
        QString("C:/ProgramData/%1").arg(QCoreApplication::applicationName()),
#elifdef __linux__
        QString("%1/%2").arg("/var/lib", QCoreApplication::applicationName()),
#endif
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation),
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation),
        QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), QCoreApplication::applicationName())
    };
    const QDir qdir;

    for (const QString &loc: locations) {
        if (!loc.isEmpty() && (qdir.exists(loc) || qdir.mkdir(loc))) {
            qWarning() << "App data location: " << loc;
            return loc;
        }

        qWarning() << QString("%1: %2 is not writable").arg(__func__, loc);
    }

    qWarning("No writable location found, unable to write files to disk");
    return "";
}

int main(int argc, char *argv[]) {
    QCoreApplication::setApplicationName("PowerTunerDaemon");
    qRegisterMetaType<PWTS::DeviceInfoPacket>();
    qRegisterMetaType<PWTS::ClientPacket>();
    qRegisterMetaType<PWTS::DaemonPacket>();

    if constexpr (PWTD::isUnknownOS()) {
        qCritical("Unsupported OS!");
        return 1;
    }

    PWTD::ProcessStatus procStatus;
    const QLockFile::LockError isRunning = procStatus.isRunning();

    if (isRunning != QLockFile::NoError) {
        qCritical(isRunning == QLockFile::LockFailedError ? "PowerTunerDaemon is already running! (code %d)" : "presence check failed, aborting! (code %d)", isRunning);
        return 1;
    }

    const QString dataPath = getAppDataPath();
    PWTD::FileLogger &logger = PWTD::FileLogger::get();
    QScopedPointer<PWTS::DaemonSettings> settings = QScopedPointer<PWTS::DaemonSettings>(new PWTS::DaemonSettings);
    QCoreApplication a(argc, argv);
    int ret;

    if (!settings->load(PWTD::DaemonSettingDiskManager::getInstance()->load()))
        qWarning("Failed to load daemon settings, using defaults");

    logger.setOutput(dataPath);
    logger.setLevel(settings->getLogLevel());
    logger.init();

    if (!PWTD::Device::get().isCPUSupported()) {
        qCritical("Unsupported CPU!");
        return 1;

    } else if (!PWTD::Device::get().isOSSupported()) {
        qCritical("Unsupported OS!");
        return 1;
    }

    QScopedPointer<PWTD::Daemon> daemonSvc;

    settings.reset();
#ifdef __linux__
    daemonSvc.reset(new PWTD::DaemonLinux(dataPath));
#elifdef _WIN32
    daemonSvc.reset(new PWTD::DaemonWindows(dataPath));
#endif

    daemonSvc->setupCmdArgs();
    daemonSvc->parseCmdArgs(a);

    ret = daemonSvc->run();
    if (ret != 2)
        return ret;

    return QCoreApplication::exec();
}
