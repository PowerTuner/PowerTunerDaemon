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
#include "DBusServices.h"

namespace PWTD {
    using namespace Qt::StringLiterals;

    DBusServices::DBusServices() {
        initDBusServicesConnections();
        initDBusWatcher();
    }

    void DBusServices::initDBusWatcher() {
        dbusWatcher.setWatchMode(QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration);
        dbusWatcher.addWatchedService(upowerSrvName);
        dbusWatcher.addWatchedService(login1SrvName);

        QObject::connect(&dbusWatcher, &QDBusServiceWatcher::serviceRegistered, this, &DBusServices::onDBusServiceRegistered);
        QObject::connect(&dbusWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &DBusServices::onDBusServiceUnregistered);
    }

    void DBusServices::initDBusServicesConnections() {
        if (upowerDBus.isValid())
            onDBusServiceRegistered(upowerSrvName);
        else if (logger.isLevel(PWTS::LogLevel::Error))
            logger.write(u"UPower DBus service not available, OnBattery event disabled"_s);

        if (login1Dbus.isValid())
            onDBusServiceRegistered(login1SrvName);
        else if (logger.isLevel(PWTS::LogLevel::Error))
            logger.write(u"login1 DBus service not available, OnSystemWake event disabled"_s);
    }

    void DBusServices::onDBusServiceRegistered(const QString &name) {
        bool res;

        if (name == upowerSrvName) {
            res = upowerDBus.connection().connect(name, upowerPath, u"org.freedesktop.DBus.Properties"_s, u"PropertiesChanged"_s, this, SLOT(onDBusUPowerPropsChanged()));

            if (res)
                prevIsOnBattery = upowerDBus.property("OnBattery").toBool();
            else if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(u"Failed to connect to UPower PropertiesChanged signal"_s);

        } else if (name == login1SrvName) {
            res = login1Dbus.connection().connect(name, login1Path, login1IFace, u"PrepareForSleep"_s, this, SLOT(onDBusLogin1PrepareForSleep(bool)));

            if (!res && logger.isLevel(PWTS::LogLevel::Error))
                logger.write(u"Failed to connect to login1 PrepareForSleep signal"_s);
        }
    }

    void DBusServices::onDBusServiceUnregistered(const QString &name) {
        if (name == upowerSrvName)
            upowerDBus.connection().disconnect(name, upowerPath, u"org.freedesktop.DBus.Properties"_s, u"PropertiesChanged"_s, this, SLOT(onDBusUPowerPropsChanged()));
        else if (name == login1SrvName)
            login1Dbus.connection().disconnect(name, login1Path, u"org.freedesktop.login1.Manager"_s, u"PrepareForSleep"_s, this, SLOT(onDBusLogin1PrepareForSleep(bool)));
    }

    void DBusServices::onDBusUPowerPropsChanged() {
        const bool isOnBattery = upowerDBus.property("OnBattery").toBool();

        if (isOnBattery == prevIsOnBattery)
            return;

        prevIsOnBattery = isOnBattery;

        emit batteryStatusChanged(isOnBattery);
    }

    void DBusServices::onDBusLogin1PrepareForSleep(const bool start) {
        if (start)
            emit prepareForSleep();
        else
            emit wakeFromSleep();
    }
}
