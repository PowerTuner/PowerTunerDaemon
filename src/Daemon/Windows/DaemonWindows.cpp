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
#include "../../external/libPWTWin32/src/win32Svc.h"
#include <csignal>

#include "DaemonWindows.h"

#define SVCNAME L"PowerTunerDaemon"

namespace PWTD {
    using namespace Qt::StringLiterals;

    DaemonWindows::DaemonWindows(const QString &dataPath) {
        appDataPath = dataPath;
    }

    DaemonWindows::~DaemonWindows() {
        if (svcThread == nullptr)
            return;

        svcThread->quit();
        svcThread->wait();
        delete svcThread;
    }

    void DaemonWindows::setupCmdArgs() const {
        Daemon::setupCmdArgs();

        cmdParser->addOption({u"installsvc"_s, u"Install PowerTunerDaemon service"_s});
        cmdParser->addOption({u"uninstallsvc"_s, u"Uninstall PowerTunerDaemon service"_s});
        cmdParser->addOption({u"startsvc"_s, u"Start PowerTunerDaemon service"_s});
        cmdParser->addOption({u"stopsvc"_s, u"Stop PowerTunerDaemon service"_s});
        cmdParser->addOption({u"nosvc"_s, u"Run in portable mode instead of a service"_s});
    }

    void DaemonWindows::parseCmdArgs(const QCoreApplication &app) {
        Daemon::parseCmdArgs(app);

        cmdInstallSvc = cmdParser->isSet(u"installsvc"_s);
        cmdUninstallSvc = cmdParser->isSet(u"uninstallsvc"_s);
        cmdStartSvc = cmdParser->isSet(u"startsvc"_s);
        cmdStopSvc = cmdParser->isSet(u"stopsvc"_s);
        cmdNoSvc = cmdParser->isSet(u"nosvc"_s);
    }

    int DaemonWindows::run() { // ret: 0 success, 1 fail
        const std::function<void(const std::wstring &)> logcb = [](const std::wstring &msg) {
            qCritical() << msg;
        };

        if (cmdInstallSvc) {
            const std::wstring exePath = QString("\"%1\"").arg(QCoreApplication::applicationFilePath()).toStdWString();

            if (!PWTSVC::installService(SVCNAME, exePath, logcb)) {
                qCritical("failed to install service");
                return 1;
            }

            qInfo("service installed successfully");
            return 0;

        } else if (cmdUninstallSvc) {
            if (!PWTSVC::stopService(SVCNAME, logcb)) {
                qCritical("failed to stop service");
                return 1;
            }

            if (!PWTSVC::deleteService(SVCNAME, logcb)) {
                qCritical("failed to delete service");
                return 1;
            }

            qInfo("service uninstalled successfully");
            return 0;

        } else if (cmdStartSvc) {
            if (!PWTSVC::startService(SVCNAME, logcb)) {
                qCritical("failed to start service");
                return 1;
            }

            qInfo("service started successfully");
            return 0;

        } else if (cmdStopSvc) {
            if (!PWTSVC::stopService(SVCNAME, logcb)) {
                qCritical("failed to stop service");
                return 1;
            }

            qInfo("service stopped successfully");
            return 0;
        }

        if (cmdNoSvc) {
            const std::optional<bool> ret = PWTSVC::isServiceRunning(SVCNAME, logcb);

            if (!ret) {
                qCritical("service status check failed, cannot start daemon");
                return 1;

            } else if (ret.value()) {
                qCritical("service is running, cannot start a new daemon");
                return 1;
            }

            std::signal(SIGTERM, sigterm);
            std::signal(SIGINT, sigterm);
            std::signal(SIGABRT, sigterm);

            sigNotifier.reset(new SignalNotifier);
            service.reset(new DaemonService(appDataPath));

            QObject::connect(sigNotifier.get(), &SignalNotifier::sigTermReceived, this, &DaemonWindows::onSigTerm);

            service->start(cmdAdr, cmdPort);

        } else {
            svcThread = new QThread();
            svcWorker = new SVCWorker(appDataPath, cmdAdr, cmdPort);

            svcWorker->moveToThread(svcThread);

            QObject::connect(svcThread, &QThread::started, svcWorker, &SVCWorker::start);
            QObject::connect(svcThread, &QThread::finished, svcWorker, &QObject::deleteLater);
            QObject::connect(svcWorker, &SVCWorker::svcStopped, this, &DaemonWindows::onSvcStop);

            svcThread->start();
        }

        return 2;
    }

    void DaemonWindows::onSvcStop() {
        QCoreApplication::quit();
    }

    void DaemonWindows::onSigTerm() {
        qInfo("Termination signal received, exiting..");
        service.reset();
        QCoreApplication::quit();
    }
}
