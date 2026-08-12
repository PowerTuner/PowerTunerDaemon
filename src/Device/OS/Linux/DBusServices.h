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

#include "../../Utils/FileLogger/FileLogger.h"

namespace PWTD {
    class DBusServices final: public QObject {
        Q_OBJECT

    private:
        static constexpr char upowerSrvName[] = "org.freedesktop.UPower";
        static constexpr char upowerPath[] = "/org/freedesktop/UPower";
        static constexpr char login1SrvName[] = "org.freedesktop.login1";
        static constexpr char login1Path[] = "org.freedesktop.login1";
        static constexpr char login1IFace[] = "org.freedesktop.login1.Manager";
        FileLogger &logger = FileLogger::get();
        QDBusInterface upowerDBus {upowerSrvName, upowerPath, upowerSrvName, QDBusConnection::systemBus()};
        QDBusInterface login1Dbus {login1SrvName, login1Path, login1IFace, QDBusConnection::systemBus()};
        QDBusServiceWatcher dbusWatcher {};
        bool prevIsOnBattery = false;

        void initDBusServicesConnections();
        void initDBusWatcher();

    public:
        DBusServices();
        ~DBusServices() override = default;

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
