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
#include "PowerTunerDaemon.h"
#include "../Utils/AppDataPath.h"

namespace PWTD {
    PowerTunerDaemon::PowerTunerDaemon() {
        const QString appDataPath = AppDataPath::appDataLocation();

        if (appDataPath.isEmpty())
            qWarning("No writable app data location found, unable to write files to disk");
        else
            qWarning() << "App data location: " << appDataPath;

        cmdParser.reset(new QCommandLineParser);
    }

    void PowerTunerDaemon::setupCmdArgs() const {
        cmdParser->addHelpOption();
        cmdParser->addOption({"a", "listen on address|localhost|any, default any", "address", "any"});
        cmdParser->addOption({"p", QString("port, default %1").arg(PWTS::DaemonSettings::DefaultTCPPort), "port", QString::number(PWTS::DaemonSettings::DefaultTCPPort)});
        cmdParser->addOption({"nc", "disable client connection, no TCP/UDP server"});
    }

    void PowerTunerDaemon::parseCmdArgs(const QCoreApplication &app) {
        cmdParser->process(app);

        cmdNoClients = cmdParser->isSet("nc");
        cmdAdr = cmdParser->value("a");
        cmdPort = cmdParser->value("p").toUInt();
    }
}
