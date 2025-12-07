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
#include <csignal>
#ifdef SYSTEMD_NOTIFY
#include <systemd/sd-daemon.h>
#include <chrono>
#endif

#include "PowerTunerDaemonLinux.h"

namespace PWTD {
    PowerTunerDaemonLinux::PowerTunerDaemonLinux() {
        std::signal(SIGTERM, sigterm);
        std::signal(SIGINT, sigterm);
        std::signal(SIGABRT, sigterm);
        std::signal(SIGHUP, sigHup);

        QObject::connect(sigNotifier.get(), &SignalNotifier::sigTermReceived, this, &PowerTunerDaemonLinux::onSigTerm);
        QObject::connect(sigNotifier.get(), &SignalNotifier::sigHupReceived, this, &PowerTunerDaemonLinux::onSigHup);
    }

    void PowerTunerDaemonLinux::setupCmdArgs() const {
        PowerTunerDaemon::setupCmdArgs();
#ifdef SYSTEMD_NOTIFY
        cmdParser->addOption({"sd", "Run as systemd daemon"});
#endif
    }

    void PowerTunerDaemonLinux::parseCmdArgs(const QCoreApplication &app) {
        PowerTunerDaemon::parseCmdArgs(app);
#ifdef SYSTEMD_NOTIFY
        cmdSystemdDaemon = cmdParser->isSet("sd");
#endif
    }

    int PowerTunerDaemonLinux::run() {
        service.reset(new DaemonService);
        service->start(!cmdNoClients, cmdAdr, cmdPort);
#ifdef SYSTEMD_NOTIFY
        if (cmdSystemdDaemon && sd_notify(0, "READY=1") < 0) {
            qCritical("%s: failed to notify systemd", __func__);
            return 1;
        }
#endif
        return 2;
    }

    void PowerTunerDaemonLinux::onSigTerm() {
#ifdef SYSTEMD_NOTIFY
        if (cmdSystemdDaemon && sd_notify(0, "STOPPING=1") < 0)
            qCritical("%s: failed to notify systemd", __func__);
#endif
        qInfo("Termination signal received, exiting..");
        service.reset();
        QCoreApplication::quit();
    }

    void PowerTunerDaemonLinux::onSigHup() const {
#ifdef SYSTEMD_NOTIFY
        if (cmdSystemdDaemon) {
            const uint64_t usec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            const std::string reloadMsg = std::format("RELOADING=1\nMONOTONIC_USEC={}", usec);

            if (sd_notify(0, reloadMsg.c_str()) < 0)
                qCritical("%s reload: failed to notify systemd", __func__);
        }
#endif
        qInfo("Reload signal received, reloading service..");
        service->reload(!cmdNoClients, cmdAdr, cmdPort);
#ifdef SYSTEMD_NOTIFY
        if (cmdSystemdDaemon && sd_notify(0, "READY=1") < 0)
            qCritical("%s ready: failed to notify systemd", __func__);
#endif
    }
}
