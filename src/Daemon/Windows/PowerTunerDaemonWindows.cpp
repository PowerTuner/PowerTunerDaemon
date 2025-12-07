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
#include "pwtWin32/win32Svc.h"

#include <csignal>

#include "PowerTunerDaemonWindows.h"

namespace PWTD {
    PowerTunerDaemonWindows::~PowerTunerDaemonWindows() {
        if (svcThread == nullptr)
            return;

        svcThread->quit();
        svcThread->wait();
        delete svcThread;
    }

    void PowerTunerDaemonWindows::setupCmdArgs() const {
        PowerTunerDaemon::setupCmdArgs();

        cmdParser->addOption({"installsvc", "Install PowerTunerDaemon service"});
        cmdParser->addOption({"uninstallsvc", "Uninstall PowerTunerDaemon service"});
        cmdParser->addOption({"startsvc", "Start PowerTunerDaemon service"});
        cmdParser->addOption({"stopsvc", "Stop PowerTunerDaemon service"});
        cmdParser->addOption({"nosvc", "Run in portable mode instead of a service"});
    }

    void PowerTunerDaemonWindows::parseCmdArgs(const QCoreApplication &app) {
        PowerTunerDaemon::parseCmdArgs(app);

        cmdInstallSvc = cmdParser->isSet("installsvc");
        cmdUninstallSvc = cmdParser->isSet("uninstallsvc");
        cmdStartSvc = cmdParser->isSet("startsvc");
        cmdStopSvc = cmdParser->isSet("stopsvc");
        cmdNoSvc = cmdParser->isSet("nosvc");
    }

    int PowerTunerDaemonWindows::run() {
        QString errStr;
        bool result;

        // ret: 0 success, 1 fail
        if (cmdInstallSvc) {
            const QString exePath = QString("\"%1\"").arg(QCoreApplication::applicationFilePath());

            result = PWTW32::installService(exePath, errStr);

            if (!result)
                qCritical() << errStr;
            else
                qInfo("service installed successfully");

            return !result;

        } else if (cmdUninstallSvc) {
            result = PWTW32::stopService(errStr);

            if (!result) {
                qCritical() << errStr;
                return 1;
            }

            result = PWTW32::deleteService(errStr);
            if (!result) {
                qCritical() << errStr;
                return 1;
            }

            qInfo("service uninstalled successfully");
            return 0;

        } else if (cmdStartSvc) {
            result = PWTW32::startService(errStr);

            if (!result)
                qCritical() << errStr;
            else
                qInfo("service started successfully");

            return !result;

        } else if (cmdStopSvc) {
            result = PWTW32::stopService(errStr);

            if (!result)
                qCritical() << errStr;
            else
                qInfo("service stopped successfully");

            return !result;
        }

        if (cmdNoSvc) {
            if (PWTW32::isServiceRunning(errStr)) {
                if (!errStr.isEmpty())
                    qCritical() << errStr;
                else
                    qCritical("service is running, cannot start a new daemon");

                return 1;
            }

            std::signal(SIGTERM, sigterm);
            std::signal(SIGINT, sigterm);
            std::signal(SIGABRT, sigterm);

            sigNotifier.reset(new SignalNotifier);
            service.reset(new DaemonService);

            QObject::connect(sigNotifier.get(), &SignalNotifier::sigTermReceived, this, &PowerTunerDaemonWindows::onSigTerm);

            service->start(!cmdNoClients, cmdAdr, cmdPort);

        } else {
            svcThread = new QThread();
            svcWorker = new SVCWorker(cmdNoClients, cmdAdr, cmdPort);

            svcWorker->moveToThread(svcThread);

            QObject::connect(svcThread, &QThread::started, svcWorker, &SVCWorker::start);
            QObject::connect(svcThread, &QThread::finished, svcWorker, &QObject::deleteLater);
            QObject::connect(svcWorker, &SVCWorker::svcStopped, this, &PowerTunerDaemonWindows::onSvcStop);

            svcThread->start();
        }

        return 2;
    }

    void PowerTunerDaemonWindows::onSvcStop() {
        QCoreApplication::quit();
    }

    void PowerTunerDaemonWindows::onSigTerm() {
        qInfo("Termination signal received, exiting..");
        service.reset();
        QCoreApplication::quit();
    }
}
