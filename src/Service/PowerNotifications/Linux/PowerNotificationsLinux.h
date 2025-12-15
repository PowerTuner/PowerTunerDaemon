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

#include "../PowerNotifications.h"
#ifdef ENABLE_DBUS_SERVICES
#include "../../../Device/OS/Linux/DBusServices.h"
#endif

namespace PWTD::LNX {
	class PowerNotificationsLinux final: public PowerNotifications {
		Q_OBJECT

	private:
#ifdef ENABLE_DBUS_SERVICES
		QScopedPointer<DBusServices> dbusServices;
#endif

	public:
		void initNotifications() override;

	private slots:
#ifdef ENABLE_DBUS_SERVICES
		void onDBusServiceBatteryStatusChange(bool onBattery);
		void onDBusServicePrepareForSleep();
        void onDBusServiceWakeFromSleep();
#endif
	};
}
