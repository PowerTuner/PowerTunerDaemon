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
#include "PowerNotificationsLinux.h"

namespace PWTD::LNX {
	void PowerNotificationsLinux::initNotifications() {
#ifdef ENABLE_DBUS_SERVICES
		dbusServices.reset(new DBusServices());

		QObject::connect(dbusServices.get(), &DBusServices::batteryStatusChanged, this, &PowerNotificationsLinux::onDBusServiceBatteryStatusChange);
		QObject::connect(dbusServices.get(), &DBusServices::prepareForSleep, this, &PowerNotificationsLinux::onDBusServicePrepareForSleep);
		QObject::connect(dbusServices.get(), &DBusServices::wakeFromSleep, this, &PowerNotificationsLinux::onDBusServiceWakeFromSleep);
#endif
	}

#ifdef ENABLE_DBUS_SERVICES
	void PowerNotificationsLinux::onDBusServiceBatteryStatusChange(const bool onBattery) {
		emit batteryStatusChanged(onBattery);
	}

	void PowerNotificationsLinux::onDBusServicePrepareForSleep() {
		emit prepareForSleepTriggered();
	}

	void PowerNotificationsLinux::onDBusServiceWakeFromSleep() {
		emit wakeFromSleepEventTriggered();
	}
#endif
}
