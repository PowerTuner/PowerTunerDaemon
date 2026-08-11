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
#include "Daemon.h"

namespace PWTD {
    Daemon::Daemon() {
        cmdParser.reset(new QCommandLineParser);
    }

    void Daemon::setupCmdArgs() const {
        cmdParser->addHelpOption();
        cmdParser->addOption({"a", "listen on address|localhost|any, default any", "address", "any"});
        cmdParser->addOption({"p", QString("port, default %1").arg(PWTS::DaemonSettings::DefaultTCPPort), "port", QString::number(PWTS::DaemonSettings::DefaultTCPPort)});
    }

    void Daemon::parseCmdArgs(const QCoreApplication &app) {
        cmdParser->process(app);

        cmdAdr = cmdParser->value("a");
        cmdPort = cmdParser->value("p").toUInt();
    }
}
