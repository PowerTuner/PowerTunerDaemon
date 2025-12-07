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
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

#include "AppDataPath.h"

namespace PWTD {
    QString AppDataPath::appDataLocation() {
        if (!path.isEmpty() || failed)
            return path;

        const QList<QString> locations = {
#ifdef _WIN32
            QString("C:/ProgramData/%1").arg(QCoreApplication::applicationName()),
#elif defined(__APPLE__)
            QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::HomeLocation), QCoreApplication::applicationName()),
#else
            QString("%1/%2").arg("/var/lib", QCoreApplication::applicationName()),
#endif
            QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation),
            QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation),
            QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), QCoreApplication::applicationName())
        };
        const QDir qdir;

        for (const QString &appDataPath: locations) {
            if (!appDataPath.isEmpty()) {
                path = appDataPath;

                if (qdir.exists(path) || qdir.mkdir(path))
                    return path;
            }

            qWarning() << QString("App data path: %1 is not writable").arg(appDataPath);
        }

        failed = true;
        path = "";

        return "";
    }
}
