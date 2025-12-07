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
#include "PowerNotificationsWindows.h"

#include <powrprof.h>
#include <powerbase.h>

namespace PWTD::WIN {
	PowerNotificationsWindows::PowerNotificationsWindows() {
		logger = FileLogger::getInstance();
	}

	PowerNotificationsWindows::~PowerNotificationsWindows() {
		PowerUnregisterSuspendResumeNotification(powerSuspendResumeNotificationHandle);
		PowerSettingUnregisterNotification(powerACDCNotificationHandle);
	}

	ULONG PowerNotificationsWindows::powerSuspendResumeNotificationCB(PVOID context, ULONG type, PVOID setting) {
		if (type == PBT_APMSUSPEND)
			emit static_cast<PowerNotificationsWindows *>(context)->prepareForSleepTriggered();
		else if (type == PBT_APMRESUMESUSPEND)
			emit static_cast<PowerNotificationsWindows *>(context)->wakeFromSleepEventTriggered();

		return ERROR_SUCCESS;
	}

	ULONG PowerNotificationsWindows::powerACDCNotificationCB(PVOID context, ULONG type, PVOID setting) {
		SYSTEM_POWER_STATUS powerStatus {};

		if (GetSystemPowerStatus(&powerStatus) == 0) {
			if (logger->isLevel(PWTS::LogLevel::Error)) {
				const DWORD code = GetLastError();
				logger->write(QString("Battery status change: failed to get power status: %1").arg(code));
			}

			return ERROR_INVALID_DATA;
		}

		if (oldACLineStatus != static_cast<BYTE>(ACLine::Unknown) && oldACLineStatus != powerStatus.ACLineStatus)
			emit static_cast<PowerNotificationsWindows *>(context)->batteryStatusChanged(powerStatus.ACLineStatus == static_cast<BYTE>(ACLine::Offline));

		oldACLineStatus = powerStatus.ACLineStatus;
		return ERROR_SUCCESS;
	}

	void PowerNotificationsWindows::initNotifications() {
		DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS notifySuspendResumeParams {
            PowerNotificationsWindows::powerSuspendResumeNotificationCB,
            this
		};
		DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS notifyACDCParams {
			PowerNotificationsWindows::powerACDCNotificationCB,
			this
		};
		DWORD ret;

		ret = PowerRegisterSuspendResumeNotification(DEVICE_NOTIFY_CALLBACK, &notifySuspendResumeParams, &powerSuspendResumeNotificationHandle);
		if (ret != ERROR_SUCCESS && logger->isLevel(PWTS::LogLevel::Error))
			logger->write(QString("Failed to register sleep events: %1").arg(ret));

		ret = PowerSettingRegisterNotification(&GUID_ACDC_POWER_SOURCE, DEVICE_NOTIFY_CALLBACK, &notifyACDCParams, &powerACDCNotificationHandle);
		if (ret != ERROR_SUCCESS && logger->isLevel(PWTS::LogLevel::Error))
			logger->write(QString("Failed to register battery status change event: %1").arg(ret));
	}
}
