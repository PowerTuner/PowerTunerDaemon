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
#pragma once

#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QScopedPointer>

#include "../../Utils/FileLogger/FileLogger.h"

namespace PWTD::LNX {
    class DBusServices final: public QObject {
        Q_OBJECT

    private:
        QScopedPointer<QDBusServiceWatcher> dbusWatcher;
        QScopedPointer<QDBusInterface> upowerDBus;
        QScopedPointer<QDBusInterface> login1Dbus;
        QSharedPointer<FileLogger> logger;
        bool prevIsOnBattery = false;

        void initDBusServicesConnections();
        void initDBusWatcher() const;

    public:
        DBusServices();

    private slots:
        void onDBusServiceRegistered(const QString &name);
        void onDBusServiceUnregistered(const QString &name);
        void onDBusUPowerPropsChanged();
        void onDBusLogin1PrepareForSleep(bool start);

    signals:
        void batteryStatusChanged(bool isOnBattery);
        void prepareForSleep();
        void wakeFromSleep();
    };
}
