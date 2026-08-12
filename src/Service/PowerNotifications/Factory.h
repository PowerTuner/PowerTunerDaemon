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
#include "PowerNotifications.h"
#if defined(__linux__) && defined(WITH_DBUS_SERVICES)
#include "Linux/PowerNotificationsLinux.h"
#elifdef _WIN32
#include "Windows/PowerNotificationsWindows.h"
#endif

namespace PWTD::PwrNotf {
    [[nodiscard]]
	inline QSharedPointer<PowerNotifications> factory() {
#if defined(__linux__) && defined(WITH_DBUS_SERVICES)
		return QSharedPointer<PowerNotificationsLinux>::create();
#elifdef _WIN32
		return QSharedPointer<PowerNotificationsWindows>::create();
#else
		return nullptr;
#endif
	}
}
