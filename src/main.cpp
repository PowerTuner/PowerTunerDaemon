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
#ifdef __linux__
#include "Daemon/Linux/PowerTunerDaemonLinux.h"
#elif defined(_WIN32)
#include "Daemon/Windows/PowerTunerDaemonWindows.h"
#endif
#include "Device/Device.h"
#include "Utils/ProcessStatus.h"
#include "Utils/DaemonUtils.h"
#include "pwtShared/DaemonSettings.h"
#include "DiskManagers/DaemonSettingDiskManager.h"
#include "Utils/FileLogger/FileLogger.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setApplicationName("PowerTunerDaemon");
    qRegisterMetaType<PWTS::DeviceInfoPacket>();
    qRegisterMetaType<PWTS::ClientPacket>();
    qRegisterMetaType<PWTS::DaemonPacket>();

    if constexpr (PWTD::isUnknownOS()) {
        qCritical("Unsupported OS!");
        return 1;
    }

    const QScopedPointer<PWTD::ProcessStatus> procStatus(new PWTD::ProcessStatus);
    const QLockFile::LockError isRunning = procStatus->isAlreadyRunning();
    QScopedPointer<PWTS::DaemonSettings> settings = QScopedPointer<PWTS::DaemonSettings>(new PWTS::DaemonSettings);
    QSharedPointer<PWTD::Device> device;
    QCoreApplication a(argc, argv);
    int ret;

    if (isRunning != QLockFile::NoError) {
        qCritical(isRunning == QLockFile::LockFailedError ? "PowerTunerDaemon is already running! (code %d)" : "presence check failed, aborting! (code %d)", isRunning);
        return 1;
    }

    if (!settings->load(PWTD::DaemonSettingDiskManager::getInstance()->load()))
        qWarning("Failed to load daemon settings, using default log settings");

    PWTD::FileLogger::getInstance()->init(settings->getLogLevel(), settings->getMaxLogFiles());

    device = PWTD::Device::getDevice();
    if (!device->isCPUSupported()) {
        qCritical("Unsupported CPU!");
        return 1;

    } else if (!device->isOSSupported()) {
        qCritical("Unsupported OS!");
        return 1;
    }

    QScopedPointer<PWTD::PowerTunerDaemon> daemonSvc;

    settings.reset();
#ifdef __linux__
    daemonSvc.reset(new PWTD::PowerTunerDaemonLinux);
#elif defined(_WIN32)
    daemonSvc.reset(new PWTD::PowerTunerDaemonWindows);
#endif

    daemonSvc->setupCmdArgs();
    daemonSvc->parseCmdArgs(a);

    ret = daemonSvc->run();
    if (ret != 2)
        return ret;

    return a.exec();
}
