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

namespace PWTD::LNX {
    DBusServices::DBusServices() {
        upowerDBus.reset(new QDBusInterface("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", QDBusConnection::systemBus()));
        login1Dbus.reset(new QDBusInterface("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus()));
        dbusWatcher.reset(new QDBusServiceWatcher);
        logger = FileLogger::getInstance();

        initDBusServicesConnections();
        initDBusWatcher();
    }

    void DBusServices::initDBusWatcher() const {
        const QList<QString> services {
            "org.freedesktop.UPower",
            "org.freedesktop.login1"
        };

        dbusWatcher->setWatchMode(QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration);

        for (const QString &service: services)
            dbusWatcher->addWatchedService(service);

        QObject::connect(dbusWatcher.get(), &QDBusServiceWatcher::serviceRegistered, this, &DBusServices::onDBusServiceRegistered);
        QObject::connect(dbusWatcher.get(), &QDBusServiceWatcher::serviceUnregistered, this, &DBusServices::onDBusServiceUnregistered);
    }

    void DBusServices::initDBusServicesConnections() {
        if (upowerDBus->isValid())
            onDBusServiceRegistered("org.freedesktop.UPower");
        else if (logger->isLevel(PWTS::LogLevel::Error))
            logger->write(QStringLiteral("UPower DBus service not available, OnBattery event disabled"));

        if (login1Dbus->isValid())
            onDBusServiceRegistered("org.freedesktop.login1");
        else if (logger->isLevel(PWTS::LogLevel::Error))
            logger->write(QStringLiteral("login1 DBus service not available, OnSystemWake event disabled"));
    }

    void DBusServices::onDBusServiceRegistered(const QString &name) {
        bool res;

        if (name == "org.freedesktop.UPower") {
            res = upowerDBus->connection().connect(name, "/org/freedesktop/UPower", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(onDBusUPowerPropsChanged()));

            if (res)
                prevIsOnBattery = upowerDBus->property("OnBattery").toBool();
            else if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("Failed to connect to UPower PropertiesChanged signal"));

        } else if (name == "org.freedesktop.login1") {
            res = login1Dbus->connection().connect(name, "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PrepareForSleep", this, SLOT(onDBusLogin1PrepareForSleep(bool)));

            if (!res && logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("Failed to connect to login1 PrepareForSleep signal"));
        }
    }

    void DBusServices::onDBusServiceUnregistered(const QString &name) {
        if (name == "org.freedesktop.UPower")
            upowerDBus->connection().disconnect(name, "/org/freedesktop/UPower", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(onDBusUPowerPropsChanged()));
        else if (name == "org.freedesktop.login1")
            login1Dbus->connection().disconnect(name, "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PrepareForSleep", this, SLOT(onDBusLogin1PrepareForSleep(bool)));
    }

    void DBusServices::onDBusUPowerPropsChanged() {
        const bool isOnBattery = upowerDBus->property("OnBattery").toBool();

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
