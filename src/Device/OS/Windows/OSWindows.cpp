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
#include "pwtWin32/win32Reg.h"
#include "OSWindows.h"

#include <winreg.h>
#include <sysinfoapi.h>
#include <combaseapi.h>
#include <powrprof.h>

#include "libryzenadj/win32/OlsApi.h"
#include "libryzenadj/win32/OlsDef.h"

namespace PWTD::WIN {
    OSWindows::OSWindows() {
		setDisplayGUID();
    }

	bool OSWindows::setupOSAccess() const {
    	DWORD ret = GetDllStatus();

    	if (ret == OLS_DLL_NO_ERROR)
    		return true;

		if (InitializeOls() == FALSE) {
			if (logger->isLevel(PWTS::LogLevel::Error))
				logger->write(QStringLiteral("failed to load winring"));

			return false;
		}

    	ret = GetDllStatus();
    	if (ret != OLS_DLL_NO_ERROR && logger->isLevel(PWTS::LogLevel::Error))
    		logger->write(QString("failed to load winring dll, code %1").arg(ret));

    	return ret == OLS_DLL_NO_ERROR;
	}

	void OSWindows::unsetOSAccess() const {
    	DeinitializeOls();
	}

	QSet<PWTS::Feature> OSWindows::getCPUFeatures(const int numLogicalCPUs, const PWTS::CPUVendor vendor) const {
    	return {PWTS::Feature::PWR_SCHEME_GROUP};
    }

	std::pair<PWTS::GPUVendor, QSet<PWTS::Feature>> OSWindows::getGPUFeatures(const int index, const PWTS::GPUVendor vendor) const {
    	return {};
    }

	bool OSWindows::hasFanControls() const {
#ifdef WITH_GPD_FAN
    	if (sysInfo->manufacturer == "GPD")
    			return true;
#endif

    	return false;
    }

	void OSWindows::fillPackageData(const PWTS::Features &features, const PWTS::DaemonPacket &packet) const {
    	if (features.cpu.contains(PWTS::Feature::PWR_SCHEME_GROUP)) {
    		const PWTS::WIN::PowerSchemesData schemesData = getPowerSchemes();

    		packet.windowsData->schemeOptionsData = schemesData.settingsData;
    		packet.windowsData->schemes = schemesData.schemes;
    		packet.windowsData->activeScheme = getActivePowerScheme();
    	}
	}

	void OSWindows::fillDaemonPacket(const PWTS::Features &features, const PWTS::CPUVendor cpuVendor, const int numLogicalCPUs, PWTS::DaemonPacket &packet) const {
    	packet.windowsData = QSharedPointer<PWTS::WIN::WindowsData>::create();

    	fillPackageData(features, packet);
	}

	void OSWindows::applyPackageSettings(const PWTS::Features &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
    	const QSharedPointer<PWTS::WIN::WindowsData> data = packet.windowsData;

    	if (features.cpu.contains(PWTS::Feature::PWR_SCHEME_GROUP)) {
    		QSet<PWTS::DError> schemeErrs;

    		if (data->resetSchemesDefault) {
    			const DWORD ret = PowerRestoreDefaultPowerSchemes();

    			if (ret != ERROR_SUCCESS) {
    				if (logger->isLevel(PWTS::LogLevel::Error))
    					logger->write(QString("failed to restore default schemes, code: %1").arg(ret));

    				schemeErrs.insert(PWTS::DError::W_PWR_RESET_SCHEMES_DEFAULT);
    			}
    		}

    		if (schemeErrs.isEmpty())
    			schemeErrs.unite(setPowerSchemes(data->schemes, data->activeScheme));

    		if (data->replaceDefaultSchemes && schemeErrs.isEmpty()) {
    			const DWORD ret = PowerReplaceDefaultPowerSchemes();

    			if (ret != ERROR_SUCCESS) {
    				if (logger->isLevel(PWTS::LogLevel::Error))
    					logger->write(QString("failed to replace default schemes, code: %1").arg(ret));

    				schemeErrs.insert(PWTS::DError::W_PWR_REPLACE_WIN_DEFAULT_SCHEMES);
    			}
    		}

    		errors.unite(schemeErrs);
    	}
	}

	QSet<PWTS::DError> OSWindows::applySettings(const PWTS::Features &features, PWTS::CPUVendor cpuVendor, const int numLogicalCPUs, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) const {
    	if (packet.windowsData.isNull()) {
    		if (logger->isLevel(PWTS::LogLevel::Error))
    			logger->write("empty data");

    		return {};
    	}

    	QSet<PWTS::DError> errors;

    	applyPackageSettings(features, packet, errors);

    	return errors;
	}

	QString OSWindows::GUIDToQString(const GUID &guid) const {
    	OLECHAR str[50] = {0};

    	if (StringFromGUID2(guid, str, sizeof(str)) == 0)
    		return "";

    	return QString(str);
	}

	GUID OSWindows::QStringToGUID(const QString &guidStr) const {
    	GUID guid;

    	if (CLSIDFromString(guidStr.toStdWString().data(), &guid) != NOERROR)
    		return {.Data1 = 0};

    	return guid;
	}

	bool OSWindows::isNullSchemeGUID(const GUID &schemeGuid) const {
    	for (const GUID &guid: nullSchemeGUIDList) {
    		if (IsEqualGUID(guid, schemeGuid))
    			return true;
    	}

    	return false;
	}

	bool OSWindows::isNullPowerSettingGUID(const GUID &settingGuid) const {
    	for (const GUID &guid: nullPowerSettingGUIDList) {
    		if (IsEqualGUID(guid, settingGuid))
    			return true;
    	}

    	return false;
	}

	bool OSWindows::hasPowerSettingAC(const GUID &setting) const {
	    return PowerSettingAccessCheckEx(ACCESS_AC_POWER_SETTING_INDEX, &setting, KEY_WRITE) == ERROR_SUCCESS;
    }

	bool OSWindows::hasPowerSettingDC(const GUID &setting) const {
	    return PowerSettingAccessCheckEx(ACCESS_DC_POWER_SETTING_INDEX, &setting, KEY_WRITE) == ERROR_SUCCESS;
    }

	QString OSWindows::getPowerSchemeFriendlyNameFor(const GUID * const schemeGuid, const GUID *subgroupGuid, const GUID * const settingGuid) const {
		std::unique_ptr<UCHAR[]> buf;
    	DWORD bufSz;
    	DWORD ret;

    	ret = PowerReadFriendlyName(nullptr, schemeGuid, subgroupGuid, settingGuid, nullptr, &bufSz);
		if (ret != ERROR_SUCCESS) {
			if (logger->isLevel(PWTS::LogLevel::Error)) {
				const QString scheme = schemeGuid != nullptr ? GUIDToQString(*schemeGuid):"";
				const QString subg = subgroupGuid != nullptr ? GUIDToQString(*subgroupGuid):"";
				const QString sett = settingGuid != nullptr ? GUIDToQString(*settingGuid):"";

				logger->write(QString("failed to read buf size: scheme: %1, subgroup: %2, setting: %3, code: %4").arg(scheme, subg, sett).arg(ret));
			}

			return {};
		}

    	bufSz += sizeof(wchar_t);
		buf = std::make_unique<UCHAR[]>(bufSz);
    	ret = PowerReadFriendlyName(nullptr, schemeGuid, subgroupGuid, settingGuid, buf.get(), &bufSz);

		if (ret != ERROR_SUCCESS) {
			if (logger->isLevel(PWTS::LogLevel::Error)) {
				const QString scheme = schemeGuid != nullptr ? GUIDToQString(*schemeGuid):"";
				const QString subg = subgroupGuid != nullptr ? GUIDToQString(*subgroupGuid):"";
				const QString sett = settingGuid != nullptr ? GUIDToQString(*settingGuid):"";

				logger->write(QString("failed to read friendly name: scheme: %1, subgroup: %2, setting: %3, code: %4").arg(scheme, subg, sett).arg(ret));
			}

			return {};
		}

		return QString(reinterpret_cast<LPWSTR>(buf.get()));
	}

	QString OSWindows::getSettingDescription(const GUID &settingGuid) const {
    	std::unique_ptr<UCHAR[]> buf;
		DWORD bufSz;

		if (PowerReadDescription(nullptr, nullptr, nullptr, &settingGuid, nullptr, &bufSz) != ERROR_SUCCESS)
			return {};

    	bufSz += sizeof(wchar_t);
		buf = std::make_unique<UCHAR[]>(bufSz);

		if (PowerReadDescription(nullptr, nullptr, nullptr, &settingGuid, buf.get(), &bufSz) != ERROR_SUCCESS)
			return {};

    	return QString(reinterpret_cast<LPWSTR>(buf.get()));
	}

	QString OSWindows::getSettingValueUnit(const GUID &subgroupGuid, const GUID &settingGuid) const {
		std::unique_ptr<UCHAR[]> buf;
    	DWORD bufSz;

		if (PowerReadValueUnitsSpecifier(nullptr, &subgroupGuid, &settingGuid, nullptr, &bufSz) != ERROR_SUCCESS)
			return {};

    	bufSz += sizeof(wchar_t);
		buf = std::make_unique<UCHAR[]>(bufSz);

		if (PowerReadValueUnitsSpecifier(nullptr, &subgroupGuid, &settingGuid, buf.get(), &bufSz) != ERROR_SUCCESS)
			return {};

		return QString(reinterpret_cast<LPWSTR>(buf.get()));
	}

	DWORD OSWindows::getSettingValueIncrement(const GUID &subgroupGuid, const GUID &settingGuid) const {
		DWORD vinc;

		if (PowerReadValueIncrement(nullptr, &subgroupGuid, &settingGuid, &vinc) != ERROR_SUCCESS)
			return 0;

		return vinc;
	}

	PWTS::WIN::PowerSettingValue OSWindows::getPowerValue(const GUID &scheme, const GUID &subGroup, const GUID &setting) const {
		PWTS::WIN::PowerSettingValue psetting {};
		DWORD val;

		if (!hasPowerSettingAC(setting))
			return psetting;

		if (PowerReadACValueIndex(nullptr, &scheme, &subGroup, &setting, &val) == ERROR_SUCCESS)
			psetting.ac = val > INT_MAX ? INT_MAX : static_cast<int>(val);

		if (hasPowerSettingDC(setting) && PowerReadDCValueIndex(nullptr, &scheme, &subGroup, &setting, &val) == ERROR_SUCCESS)
			psetting.dc = val > INT_MAX ? INT_MAX : static_cast<int>(val);

		return psetting;
	}

	bool OSWindows::setPowerValue(const GUID &scheme, const GUID &subGroup, const GUID &setting, const PWTS::WIN::PowerSettingValue &psetting) const {
		bool success = true;

		if ((psetting.ac == -1 && hasPowerSettingAC(setting)) || PowerWriteACValueIndex(nullptr, &scheme, &subGroup, &setting, psetting.ac) != ERROR_SUCCESS) {
			if (logger->isLevel(PWTS::LogLevel::Error))
				logger->write(QString("failed to set AC value '%1'").arg(psetting.ac));

			success = false;
		}

		if (psetting.dc == -1 && hasPowerSettingDC(setting) || PowerWriteDCValueIndex(nullptr, &scheme, &subGroup, &setting, psetting.dc) != ERROR_SUCCESS) {
			if (logger->isLevel(PWTS::LogLevel::Error))
				logger->write(QString("failed to set DC value '%1'").arg(psetting.ac));

			success = false;
		}

		return success;
	}

	QMap<QString, OSWindows::SchemeGUID> OSWindows::enumeratePowerSchemesAll(HKEY hkey) const {
    	QMap<QString, SchemeGUID> guidMap;
    	std::unique_ptr<TCHAR[]> buf;
    	DWORD maxKeySz;
    	LSTATUS ret;
    	int i = 0;

    	ret = RegQueryInfoKey(hkey, nullptr, nullptr, nullptr, nullptr, &maxKeySz, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    	if (ret != ERROR_SUCCESS) {
    		if (logger->isLevel(PWTS::LogLevel::Error))
    			logger->write(QString("failed to query max key size, code %1").arg(ret));

    		RegCloseKey(hkey);
    		return {};
    	}

    	maxKeySz += sizeof(TCHAR);
    	buf = std::make_unique<TCHAR[]>(maxKeySz);

    	while (true) {
    		DWORD bufSz = maxKeySz;
    		GUID guid {};
    		QString guidStr;
    		QString name;

    		wmemset(buf.get(), 0, maxKeySz);

    		if (RegEnumKeyEx(hkey, i++, buf.get(), &bufSz, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
    			break;

    		guidStr.assign(buf.get());

    		if (guidStr.isEmpty())
    			continue;

    		guidStr = guidStr.toUpper().prepend('{').append('}');
    		guid = QStringToGUID(guidStr);

    		if (isNullSchemeGUID(guid))
    			continue;

    		name = getPowerSchemeFriendlyNameFor(&guid, nullptr, nullptr);

    		if (!name.isEmpty())
    			guidMap.insert(guidStr, {.friendlyName = name, .schemeGUID = guid});
    	}

    	RegCloseKey(hkey);
    	return guidMap;
	}

	QMap<QString, OSWindows::SchemeGUID> OSWindows::enumeratePowerSchemesPartial() const {
    	QMap<QString, SchemeGUID> guidMap;
    	int i = 0;

    	while (true) {
    		GUID guid {};
    		DWORD guidSz = sizeof(guid);
    		QString guidStr;
    		QString name;

    		if (PowerEnumerate(nullptr, nullptr, nullptr, ACCESS_SCHEME, i++, reinterpret_cast<PUCHAR>(&guid), &guidSz) != ERROR_SUCCESS)
    			break;

    		guidStr = GUIDToQString(guid);

    		if (guidStr.isEmpty() || isNullSchemeGUID(guid))
    			continue;

    		name = getPowerSchemeFriendlyNameFor(&guid, nullptr, nullptr);
    		if (!name.isEmpty())
    			guidMap.insert(guidStr, {.friendlyName = name, .schemeGUID = guid});
    	}

    	return guidMap;
	}

	QMap<QString, OSWindows::SchemeGUID> OSWindows::enumeratePowerSchemes() const {
    	HKEY hkey;

    	// to catch hidden schemes we cant use powerenumerate
    	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, LR"(SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes)", 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    		return enumeratePowerSchemesAll(hkey);

    	return enumeratePowerSchemesPartial();
    }

	GUID *OSWindows::duplicatePowerScheme(const GUID &sourceScheme, const QString &name) const {
    	std::wstring wname = name.toStdWString();
		GUID *guid = nullptr;
		DWORD ret;

		ret = PowerSettingAccessCheck(ACCESS_CREATE_SCHEME, nullptr);
		if (ret != ERROR_SUCCESS) {
			if (logger->isLevel(PWTS::LogLevel::Error))
				logger->write(QString("cannot create %1, no access to %2, code %3").arg(name, GUIDToQString(sourceScheme)).arg(ret));

			return nullptr;
		}

		ret = PowerDuplicateScheme(nullptr, &sourceScheme, &guid);
		if (ret != ERROR_SUCCESS) {
			if (logger->isLevel(PWTS::LogLevel::Error))
				logger->write(QString("cannot create %1, cannot duplicate %2, code %3").arg(name, GUIDToQString(sourceScheme)).arg(ret));

			return nullptr;
		}

		ret = PowerWriteFriendlyName(nullptr, guid, nullptr, nullptr, reinterpret_cast<PUCHAR>(wname.data()), (wname.size() + 1) * sizeof(TCHAR));
		if (ret != ERROR_SUCCESS) {
			if (logger->isLevel(PWTS::LogLevel::Error))
				logger->write(QString("cannot write friendly name for %1, code %2").arg(name).arg(ret));

			LocalFree(guid);
			return nullptr;
		}

		return guid;
	}

	bool OSWindows::deletePowerScheme(const GUID &guid) const {
    	const DWORD ret = PowerDeleteScheme(nullptr, &guid);

		if (ret != ERROR_SUCCESS && logger->isLevel(PWTS::LogLevel::Error))
			logger->write(QString("failed to delete scheme '%1', code: %2").arg(GUIDToQString(guid)).arg(ret));

		return ret == ERROR_SUCCESS;
	}

	bool OSWindows::CreatePowerScheme(QMap<QString, SchemeGUID> &guidMap, const GUID &baseGuid, const QString &duplicateTmpGuid, const QString &duplicateName) const {
		if (baseGuid.Data1 == 0) {
			if (logger->isLevel(PWTS::LogLevel::Error))
				logger->write(QString("cannot create scheme '%1': no base scheme available").arg(duplicateName));

			return false;
		}

    	GUID *duplicateGuid = duplicatePowerScheme(baseGuid, duplicateName);

		if (duplicateGuid == nullptr) {
			if (logger->isLevel(PWTS::LogLevel::Error))
				logger->write(QString("failed to create scheme '%1'").arg(duplicateName));

			return false;
		}

    	guidMap.insert(duplicateTmpGuid, {.friendlyName = duplicateName, .schemeGUID = *duplicateGuid});
    	LocalFree(duplicateGuid);
		return true;
	}

	// PowerIsSettingRangeDefined is broken after active scheme is changed, our own check doesn't fail
	LSTATUS OSWindows::isPowerSettingRangeDefined(const QString &subGroup, const QString &setting, bool *result) const {
    	const QString sett = setting.sliced(1, setting.size() - 2); // hide { }
    	QString subKey;
    	HKEY rkey;
    	LSTATUS ret;

    	if (subGroup.isEmpty()) {
    		subKey = QString(LR"(%1\%2)").arg(regPowerSettings, sett);

    	} else {
    		const QString group = subGroup.sliced(1, subGroup.size() - 2);

    		subKey = QString(LR"(%1\%2\%3)").arg(regPowerSettings, group, sett);
    	}

    	*result = false;
    	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKey.toStdWString().c_str(), 0, KEY_QUERY_VALUE, &rkey);

    	if (ret != ERROR_SUCCESS) {
    		RegCloseKey(rkey);
    		return ret;
    	}

    	ret = RegQueryValueEx(rkey, L"ValueIncrement", nullptr, nullptr, nullptr, nullptr);
    	*result = (ret == ERROR_SUCCESS);

    	RegCloseKey(rkey);
    	return (ret == ERROR_FILE_NOT_FOUND) ? ERROR_SUCCESS : ret;
	}

	PWTS::MinMax OSWindows::getPowerSettingRange(const GUID &subGroup, const GUID &setting) const {
		PWTS::MinMax range {.min = -1, .max = -1};
		DWORD min, max;

		if (PowerReadValueMin(nullptr, &subGroup, &setting, &min) == ERROR_SUCCESS)
			range.min = min > INT_MAX ? INT_MAX : static_cast<int>(min);

		if (PowerReadValueMax(nullptr,  &subGroup, &setting, &max) == ERROR_SUCCESS)
			range.max = max > INT_MAX ? INT_MAX : static_cast<int>(max);

		return range;
	}

	QList<QString> OSWindows::getPowerSettingOptions(const GUID &subGroup, const GUID &setting) const {
		QList<QString> options;
		ULONG idx = 0;

		while (true) {
			std::unique_ptr<UCHAR[]> buf;
			DWORD bufSz;

			if (PowerReadPossibleFriendlyName(nullptr, &subGroup, &setting, idx, nullptr, &bufSz) != ERROR_SUCCESS)
				break;

			bufSz += sizeof(wchar_t);
			buf = std::make_unique<UCHAR[]>(bufSz);

			if (PowerReadPossibleFriendlyName(nullptr, &subGroup, &setting, idx++, buf.get(), &bufSz) != ERROR_SUCCESS)
				continue;

			options.append(QString(reinterpret_cast<LPWSTR>(buf.get())));
		}

		return options;
	}

	GUID *OSWindows::getActiveSchemeGUID() const {
    	GUID *active;
    	const DWORD ret = PowerGetActiveScheme(nullptr, &active);

    	if (ret != ERROR_SUCCESS) {
    		if (logger->isLevel(PWTS::LogLevel::Error))
    			logger->write(QString("failed to get active power scheme, code: %1").arg(ret));

    		return nullptr;
    	}

    	return active;
    }

    void OSWindows::setDisplayGUID() {
    	constexpr TCHAR displayStr[] = L"Display";
    	constexpr size_t displayLen = std::wstring_view(displayStr).size();
        constexpr DWORD buffSz = 255;
        TCHAR nameBuf[buffSz] = {};
        TCHAR valueBuf[buffSz] = {};
        DWORD idx = 0;
        HKEY hkey;
        LSTATUS ret;

        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regSystemClass, 0, KEY_ENUMERATE_SUB_KEYS, &hkey);
        if (ret != ERROR_SUCCESS)
            return;

        while (true) {
            DWORD bufSz = buffSz;
            HKEY subKey;

            ret = RegEnumKeyEx(hkey, idx++, nameBuf, &bufSz, nullptr, nullptr, nullptr, nullptr);
            if (ret != ERROR_SUCCESS)
                break;

            ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, std::format(L"{}{}", regSystemClass, nameBuf).c_str(), 0, KEY_READ, &subKey);
            if (ret != ERROR_SUCCESS)
                continue;

            bufSz = buffSz;
            ret = RegGetValue(subKey, nullptr, L"class", RRF_RT_REG_SZ, nullptr, valueBuf, &bufSz);

            RegCloseKey(subKey);

            if (ret == ERROR_SUCCESS && wcsncmp(valueBuf, displayStr, displayLen) == 0) {
                displayGUID.assign(nameBuf);
                break;
            }
        }

        RegCloseKey(hkey);
    }

    QString OSWindows::getGPUMatchingID(const int index) const {
        if (displayGUID.isEmpty())
            return "";

    	const std::wstring subKey = QString("%1%2\\%3").arg(regSystemClass, displayGUID, QString::number(index).rightJustified(4, '0')).toStdWString();
		QString errStr;
    	QString val;

    	if (!PWTW32::regReadSZ(HKEY_LOCAL_MACHINE, subKey.c_str(), L"MatchingDeviceId", val, errStr) && logger->isLevel(PWTS::LogLevel::Error))
    		logger->write(errStr);

        return val;
    }

    QString OSWindows::getBiosVendor() const {
    	const bool hasLog = logger->isLevel(PWTS::LogLevel::Error);
    	QString errStr;
    	QString val;

    	if (!PWTW32::regReadSZ(HKEY_LOCAL_MACHINE, regBIOS, L"BIOSVendor", val, errStr, hasLog) && hasLog)
    		logger->write(errStr);

        return val;
    }

    QString OSWindows::getBiosVersion() const {
    	const bool hasLog = logger->isLevel(PWTS::LogLevel::Error);
    	QString errStr;
    	QString val;

    	if (!PWTW32::regReadSZ(HKEY_LOCAL_MACHINE, regBIOS, L"BIOSVersion", val, errStr, hasLog) && hasLog)
    		logger->write(errStr);

        return val;
    }

    QString OSWindows::getBiosDate() const {
    	const bool hasLog = logger->isLevel(PWTS::LogLevel::Error);
    	QString errStr;
    	QString val;

    	if (!PWTW32::regReadSZ(HKEY_LOCAL_MACHINE, regBIOS, L"BIOSReleaseDate", val, errStr, hasLog) && hasLog)
    		logger->write(errStr);

        return val;
    }

    QString OSWindows::getECVersion() const {
    	const bool hasLog = logger->isLevel(PWTS::LogLevel::Error);
        DWORD major, minor;
    	QString errStr;
    	QString version;

    	if (!PWTW32::regReadDword(HKEY_LOCAL_MACHINE, regBIOS, L"ECFirmwareMajorRelease", &major, errStr, hasLog)) {
    		if (hasLog)
    			logger->write(errStr);

    		return version;
    	}

    	version = QString::number(major);

    	if (!PWTW32::regReadDword(HKEY_LOCAL_MACHINE, regBIOS, L"ECFirmwareMinorRelease", &minor, errStr, hasLog)) {
    		if (hasLog)
    			logger->write(errStr);

    		return version;
    	}

    	version.append('.').append(QString::number(minor));
        return version;
    }

    QString OSWindows::getProductName() const {
    	QString errStr;
    	QString val;

    	if (!PWTW32::regReadSZ(HKEY_LOCAL_MACHINE, regBIOS, L"BaseBoardProduct", val, errStr) && logger->isLevel(PWTS::LogLevel::Error))
    		logger->write(errStr);

        return val;
    }

    QString OSWindows::getManufacturer() const {
    	QString errStr;
    	QString val;

    	if (!PWTW32::regReadSZ(HKEY_LOCAL_MACHINE, regBIOS, L"BaseBoardManufacturer", val, errStr) && logger->isLevel(PWTS::LogLevel::Error))
    		logger->write(errStr);

        return val;
    }

    int OSWindows::getOnlineCPUCount(const int numLogicalCPUs) const {
    	GUID *activeGuid = getActiveSchemeGUID();
    	PWTS::WIN::PowerSettingValue val;
        SYSTEM_POWER_STATUS pstatus;
    	int onlineCount;

    	if (activeGuid == nullptr) {
    		if (logger->isLevel(PWTS::LogLevel::Error))
    			logger->write(QStringLiteral("active guid is null"));

    		return 0;

    	} else if (GetSystemPowerStatus(&pstatus) == FALSE) {
    		if (logger->isLevel(PWTS::LogLevel::Error))
    			logger->write(QString("failed to get power status, code: %1").arg(GetLastError()));

    		LocalFree(activeGuid);
    		return 0;
    	}

    	val = getPowerValue(*activeGuid, GUID_PROCESSOR_SETTINGS_SUBGROUP, GUID_PROCESSOR_CORE_PARKING_MAX_CORES);

    	if (pstatus.ACLineStatus == 1)
    		onlineCount = val.ac == -1 ? 0 : val.ac;
    	else
    		onlineCount = val.dc == -1 ? 0 : val.dc;

    	LocalFree(activeGuid);
    	return qFloor(onlineCount / 100.f * numLogicalCPUs);
    }

	QList<int> OSWindows::getCPUCoreIndexList() const {
    	QMap<int, int> coreMap;
    	std::unique_ptr<UCHAR[]> buff;
    	DWORD buffSz = 0;
    	DWORD returnedSz = 0;
    	DWORD lastErr;

    	GetSystemCpuSetInformation(nullptr, 0, &buffSz, nullptr, 0);

    	lastErr = GetLastError();
    	if (lastErr != ERROR_INSUFFICIENT_BUFFER) {
    		if (logger->isLevel(PWTS::LogLevel::Error))
    			logger->write(QString("failed to query buffer size, code: %1").arg(lastErr));

    		return {};
    	}

    	buff = std::make_unique<UCHAR[]>(buffSz);

    	if (GetSystemCpuSetInformation(reinterpret_cast<PSYSTEM_CPU_SET_INFORMATION>(buff.get()), buffSz, &returnedSz, nullptr, 0) == FALSE) {
    		if (logger->isLevel(PWTS::LogLevel::Error))
    			logger->write(QString("failed to get cpu set information, code: %1").arg(GetLastError()));

    		return {};
    	}

    	for (DWORD i=0; i<returnedSz;) {
    		const PSYSTEM_CPU_SET_INFORMATION cpuSet = reinterpret_cast<PSYSTEM_CPU_SET_INFORMATION>(buff.get() + i);
    		BYTE coreIdx, logicalIdx;

    		if (cpuSet->Type != CpuSetInformation) {
    			i += cpuSet->Size;
    			continue;
    		}

    		coreIdx = cpuSet->CpuSet.CoreIndex;
    		logicalIdx = cpuSet->CpuSet.LogicalProcessorIndex;

    		if (logger->isLevel(PWTS::LogLevel::Info))
    			logger->write(QString("found: core index %1, logical index %2").arg(coreIdx).arg(logicalIdx));

    		if (!coreMap.contains(coreIdx)) {
    			if (logger->isLevel(PWTS::LogLevel::Info))
    				logger->write(QString("added core %1").arg(coreIdx));

    			coreMap.insert(coreIdx, logicalIdx);
    		}

    		i += cpuSet->Size;
    	}

    	return coreMap.values();
	}

    QString OSWindows::getMicrocodeRevision(const int cpu) const {
    	QString errStr;
    	DWORD dataSz = 0;
    	const bool hasLog = logger->isLevel(PWTS::LogLevel::Error);
    	const std::wstring path = std::format(L"{}CentralProcessor\\{}", regSystem, cpu);
    	const std::unique_ptr<BYTE[]> data = PWTW32::regReadBinary(HKEY_LOCAL_MACHINE, path.c_str(), L"Update Revision", &dataSz, errStr, hasLog);
    	QByteArray ba;

    	if (data == nullptr)
    		return "0x0";

    	for (int i=dataSz-1; i>=0; --i) {
    		if (data[i] == '\0')
    			continue;

    		ba.append(data[i]);
    	}

        return QString(ba.toHex()).toUpper().prepend("0x");
    }

    quint64 OSWindows::getAvailableRam() const {
        MEMORYSTATUSEX memStatus;

        memStatus.dwLength = sizeof(memStatus);

        if (!GlobalMemoryStatusEx(&memStatus))
            return 0;

        return memStatus.ullTotalPhys;
    }

	quint64 OSWindows::getSwapSize() const {
	    return 0;
    }

    QList<int> OSWindows::getGPUIndexList() const {
        if (displayGUID.isEmpty())
            return {};

        constexpr DWORD buffSz = 255;
        TCHAR buf[buffSz] = {0};
        DWORD idx = 0;
        QList<int> idxList;
        HKEY hkey;
        LSTATUS ret;

        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, QString("%1%2").arg(regSystemClass, displayGUID).toStdWString().c_str(), 0, KEY_ENUMERATE_SUB_KEYS, &hkey);
        if (ret != ERROR_SUCCESS)
            return {};

        while (true) {
            DWORD bufSz = buffSz;
            int gpuIdx;
            bool res;

            ret = RegEnumKeyEx(hkey, idx++, buf, &bufSz, nullptr, nullptr, nullptr, nullptr);
            if (ret != ERROR_SUCCESS)
                break;

            gpuIdx = QString(buf).toInt(&res);
            if (res)
                idxList.append(gpuIdx);
        }

        RegCloseKey(hkey);
        return idxList;
    }

    PWTS::GPUVendor OSWindows::getGPUVendor(const int index) const {
        const QString mathingID = getGPUMatchingID(index).toLower();
        const QRegularExpressionMatch match = gpuVendorIDRex.match(mathingID);
    	const QString id = !match.hasMatch() ? "" : match.captured(1);

        if (id.isEmpty())
            return PWTS::GPUVendor::Unknown;
        else if (id == "1002")
            return PWTS::GPUVendor::AMD;
        else if (id == "8086")
            return PWTS::GPUVendor::Intel;
    	else if (id == "10de")
    		return PWTS::GPUVendor::NVIDIA;

        return PWTS::GPUVendor::Unknown;
    }

    QString OSWindows::getGPUDeviceID(const int index) const {
        const QString mathingID = getGPUMatchingID(index).toLower();
        const QRegularExpressionMatch match = gpuDeviceIDRex.match(mathingID);

        if (!match.hasMatch())
            return "";

        return QString("0x%1").arg(match.captured(1));
    }

    QString OSWindows::getGPURevisionID(const int index) const {
        const QString mathingID = getGPUMatchingID(index).toLower();
        const QRegularExpressionMatch match = gpuRevisionIDRex.match(mathingID);

        if (!match.hasMatch())
            return "";

        return QString("0x%1").arg(match.captured(1));
    }

	QString OSWindows::getGPUVBiosVersion(int index) const {
	    return "";
    }

	QString OSWindows::getActivePowerScheme() const {
    	GUID *activeGuid = getActiveSchemeGUID();

    	if (activeGuid == nullptr) {
    		if (logger->isLevel(PWTS::LogLevel::Error))
    			logger->write(QStringLiteral("active guid is null"));

    		return "";
    	}

    	const QString active = GUIDToQString(*activeGuid);

    	LocalFree(activeGuid);
    	return active;
	}

	void OSWindows::enumeratePowerSettingGroups(const GUID &schemeGuid, PWTS::WIN::PowerSchemesData &pschemeData, PWTS::WIN::PowerScheme &pscheme) const {
		bool isNoSubGroup = true; // first iteration flag to fetch root scheme settings
		int idx = 0;

    	while (true) {
			GUID subgGuid {};
			DWORD bufSz = sizeof(subgGuid);

			if (isNoSubGroup)
				subgGuid = NO_SUBGROUP_GUID;
			else if (PowerEnumerate(nullptr, &schemeGuid, nullptr, ACCESS_SUBGROUP, idx++, reinterpret_cast<PUCHAR>(&subgGuid), &bufSz) != ERROR_SUCCESS)
				break;

			// enumerate all settings in selected setting group
			enumeratePowerSettingsInGroup(schemeGuid, subgGuid, pschemeData, pscheme, isNoSubGroup);

			isNoSubGroup = false;
		}
	}

	void OSWindows::enumeratePowerSettingsInGroup(const GUID &schemeGuid, const GUID &groupGuid, PWTS::WIN::PowerSchemesData &pschemeData, PWTS::WIN::PowerScheme &pscheme, const bool isNoSubGroup) const {
    	int idx = 0;

	    while (true) {
			GUID settingGuid {};
	    	DWORD bufSz = sizeof(settingGuid);

			if (PowerEnumerate(nullptr, &schemeGuid, &groupGuid, ACCESS_INDIVIDUAL_SETTING, idx++, reinterpret_cast<PUCHAR>(&settingGuid), &bufSz) != ERROR_SUCCESS)
				break;
			else if (isNullPowerSettingGUID(settingGuid))
				continue;

	    	const QString settingGuidStr = GUIDToQString(settingGuid);
	    	const QString settingGroupGuidStr = GUIDToQString(groupGuid);

	    	// collect setting static data, but only once for each setting
			if (!pschemeData.settingsData.contains(settingGuidStr)) {
				const QString settingName = getPowerSchemeFriendlyNameFor(nullptr, nullptr, &settingGuid);

				if (settingName.isEmpty()) {
					if (logger->isLevel(PWTS::LogLevel::Error))
						logger->write(QString("collect static data: empty name for setting: %1, skip").arg(settingGuidStr));

					continue;
				}

				PWTS::WIN::PowerSchemeSettingData settingData {
					.groupTitle = isNoSubGroup ? "" : getPowerSchemeFriendlyNameFor(nullptr, &groupGuid, nullptr),
					.name = settingName,
					.description = getSettingDescription(settingGuid),
					.isRangeDefined = false
				};
				bool isRangeDefined;
				const LSTATUS ret = isPowerSettingRangeDefined(isNoSubGroup ? "" : settingGroupGuidStr, settingGuidStr, &isRangeDefined);

				if (ret != ERROR_SUCCESS) {
					if (logger->isLevel(PWTS::LogLevel::Error))
						logger->write(QString("failed to check setting type, setting: %1, group: %2, code %3").arg(settingGuidStr, settingGroupGuidStr).arg(ret));

					continue;
				}

				if (isRangeDefined) {
					settingData.valueUnit = getSettingValueUnit(groupGuid, settingGuid);
					settingData.valueIncrement = getSettingValueIncrement(groupGuid, settingGuid);
					settingData.range = getPowerSettingRange(groupGuid, settingGuid);
					settingData.isRangeDefined = true;

				} else {
					settingData.options = getPowerSettingOptions(groupGuid, settingGuid);
				}

				pschemeData.settingsData.insert(settingGuidStr, settingData);
			}

			pscheme.settings.insert(settingGuidStr, {
				.groupGuid = settingGroupGuidStr,
				.value = getPowerValue(schemeGuid, groupGuid, settingGuid)
			});
		}
    }

	PWTS::WIN::PowerSchemesData OSWindows::getPowerSchemes() const {
		const QMap<QString, SchemeGUID> schemesMap = enumeratePowerSchemes();
    	PWTS::WIN::PowerSchemesData schemesData;

		for (const auto &[schemeGuidStr, schemeGuid]: schemesMap.asKeyValueRange()) {
			PWTS::WIN::PowerScheme powerScheme {.friendlyName = schemeGuid.friendlyName};

			enumeratePowerSettingGroups(schemeGuid.schemeGUID, schemesData, powerScheme);
			schemesData.schemes.insert(schemeGuidStr, powerScheme);
		}

		return schemesData;
	}

	QSet<PWTS::DError> OSWindows::setPowerSchemes(const QMap<QString, PWTS::WIN::PowerScheme> &pschemes, const QString &activeScheme) const {
		QMap<QString, SchemeGUID> guidMap = enumeratePowerSchemes();
    	GUID duplicatesBaseGuid = {0};
    	QList<GUID> deleteList;
    	QSet<PWTS::DError> errors;

    	// find a suitable base scheme to create new schemes
		for (const auto &[guidStr, scheme]: pschemes.asKeyValueRange()) {
			if (scheme.deleteFlag || !guidMap.contains(guidStr))
				continue;

			duplicatesBaseGuid = guidMap[guidStr].schemeGUID;
			break;
		}

		for (const auto &[guidStr, scheme]: pschemes.asKeyValueRange()) {
			const bool isSchemeInstalled = guidMap.contains(guidStr);

			if (scheme.deleteFlag) {
				if (isSchemeInstalled && guidStr != activeScheme)
					deleteList.append(guidMap[guidStr].schemeGUID);

				continue;

			} else if (scheme.resetFlag && isSchemeInstalled) {
				const GUID guid = guidMap[guidStr].schemeGUID;
				DWORD ret;

				ret = PowerCanRestoreIndividualDefaultPowerScheme(&guid);
				if (ret != ERROR_SUCCESS) {
					if (logger->isLevel(PWTS::LogLevel::Error))
						logger->write(QString("cannot restore %1 to defaults, code: %2").arg(guidStr).arg(ret));

					errors.insert(PWTS::DError::W_PWR_RESET_INDIVIDUAL_DEFAULT);
					continue;
				}

				ret = PowerRestoreIndividualDefaultPowerScheme(&guid);
				if (ret != ERROR_SUCCESS) {
					if (logger->isLevel(PWTS::LogLevel::Error))
						logger->write(QString("failed to restore %1 to defaults, code: %2").arg(guidStr).arg(ret));

					errors.insert(PWTS::DError::W_PWR_RESET_INDIVIDUAL_DEFAULT);
				}

				continue;

			} else if (!isSchemeInstalled && !CreatePowerScheme(guidMap, duplicatesBaseGuid, guidStr, scheme.friendlyName)) {
				errors.insert(PWTS::DError::W_PWR_CREATE_SCHEMES);
				continue;
			}

			// get the guid after all changes to guidMap
			const GUID guid = guidMap[guidStr].schemeGUID;

			if (PowerSettingAccessCheck(ACCESS_SCHEME, &guid) != ERROR_SUCCESS) {
				errors.insert(PWTS::DError::W_PWR_NO_SCHEME_ACCESS);

				if (logger->isLevel(PWTS::LogLevel::Error))
					logger->write(QString("no access to scheme: %1").arg(scheme.friendlyName));

				continue;
			}

			for (const auto &[settGuid, settData]: scheme.settings.asKeyValueRange()) {
				const GUID subGroupGuid = QStringToGUID(settData.groupGuid);
				const GUID settingGuid = QStringToGUID(settGuid);

				if (subGroupGuid.Data1 == 0 || settingGuid.Data1 == 0 || !setPowerValue(guid, subGroupGuid, settingGuid, settData.value)) {
					errors.insert(PWTS::DError::W_PWR_SCHEME);

					if (logger->isLevel(PWTS::LogLevel::Error))
						logger->write(QString("failed to write '%1' setting in group '%2' for scheme '%3'").arg(settGuid, settData.groupGuid, scheme.friendlyName));
				}
			}
		}

    	// at this point, all tbd guids were created, we can set the active guid
    	if (guidMap.contains(activeScheme)) {
    		const GUID activeGuid = guidMap[activeScheme].schemeGUID;
    		const DWORD ret = PowerSetActiveScheme(nullptr, &activeGuid);

    		if (ret != ERROR_SUCCESS) {
    			if (logger->isLevel(PWTS::LogLevel::Error))
    				logger->write(QString("failed to set active scheme '%1', code %2").arg(activeScheme).arg(ret));

    			errors.insert(PWTS::DError::W_PWR_ACTIVE_SCHEME);
    		}
    	} else { // active guid not found on system
    		errors.insert(PWTS::DError::W_PWR_ACTIVE_SCHEME);
    	}

    	// now that all changes are done, it is safe to delete the flagged schemes
    	for (const GUID &guid: deleteList) {
    		if (!deletePowerScheme(guid))
    			errors.insert(PWTS::DError::W_PWR_DELETE_SCHEMES);
    	}

		return errors;
	}

	QString OSWindows::getFanControlPath(const FanBoard type) const {
		return "";
	}

	PWTS::RWData<int> OSWindows::getFanMode(const FanControls &controls) const {
		switch (controls.board) {
#ifdef WITH_GPD_FAN
			case FanBoard::GPD_WIN4:
			case FanBoard::GPD_WIN_MINI:
			case FanBoard::GPD_WIN_MAX2:
			case FanBoard::GPD_DUO:
			case FanBoard::GPD_MPC2:
				return getGPDFanMode(controls);
#endif
			default:
				break;
		}

    	return {};
	}

	PWTS::ROData<int> OSWindows::getFanSpeed(const FanControls &controls) const {
    	switch (controls.board) {
#ifdef WITH_GPD_FAN
    		case FanBoard::GPD_WIN4:
    		case FanBoard::GPD_WIN_MINI:
    		case FanBoard::GPD_WIN_MAX2:
    		case FanBoard::GPD_DUO:
    		case FanBoard::GPD_MPC2:
    			return getGPDFanSpeed(controls);
#endif
    		default:
    			break;
    	}

    	return {};
	}

	bool OSWindows::setFanMode(const FanControls &controls, const PWTS::RWData<int> &mode) const {
    	switch (controls.board) {
#ifdef WITH_GPD_FAN
    		case FanBoard::GPD_WIN4:
    		case FanBoard::GPD_WIN_MINI:
    		case FanBoard::GPD_WIN_MAX2:
    		case FanBoard::GPD_DUO:
    		case FanBoard::GPD_MPC2:
    			return setGPDFanMode(mode, controls);
#endif
    		default:
    			break;
    	}

		return false;
	}

	bool OSWindows::setFanSpeed(const FanControls &controls, const int speed) const {
    	switch (controls.board) {
#ifdef WITH_GPD_FAN
    		case FanBoard::GPD_WIN4:
    		case FanBoard::GPD_WIN_MINI:
    		case FanBoard::GPD_WIN_MAX2:
    		case FanBoard::GPD_DUO:
    		case FanBoard::GPD_MPC2:
    			return setGPDFanSpeed(speed, controls);
#endif
    		default:
    			break;
    	}

		return false;
	}

#ifdef WITH_GPD_FAN
	bool OSWindows::gpdECRamRead(const uint8_t addrPort, const uint8_t dataPort, const uint16_t offset, uint8_t *outVal) const {
    	if (
    		WriteIoPortByteEx(addrPort, 0x2e) == FALSE ||
    		WriteIoPortByteEx(dataPort, 0x11) == FALSE ||
    		WriteIoPortByteEx(addrPort, 0x2f) == FALSE ||
    		WriteIoPortByteEx(dataPort, static_cast<uint8_t>((offset >> 8) & 0xFF)) == FALSE ||
    		WriteIoPortByteEx(addrPort, 0x2e) == FALSE ||
    		WriteIoPortByteEx(dataPort, 0x10) == FALSE ||
    		WriteIoPortByteEx(addrPort, 0x2f) == FALSE ||
    		WriteIoPortByteEx(dataPort, static_cast<uint8_t>(offset & 0xFF)) == FALSE ||
    		WriteIoPortByteEx(addrPort, 0x2e) == FALSE ||
    		WriteIoPortByteEx(dataPort, 0x12) == FALSE ||
    		WriteIoPortByteEx(addrPort, 0x2f) == FALSE
    	)
    		return false;

    	return ReadIoPortByteEx(dataPort, outVal) == TRUE;
	}

	bool OSWindows::gpdECRamWrite(const uint8_t addrPort, const uint8_t dataPort, const uint16_t offset, const uint8_t value) const {
    	return WriteIoPortByteEx(addrPort, 0x2e) == TRUE &&
    			WriteIoPortByteEx(dataPort, 0x11) == TRUE &&
    			WriteIoPortByteEx(addrPort, 0x2f) == TRUE &&
    			WriteIoPortByteEx(dataPort, static_cast<uint8_t>((offset >> 8) & 0xFF)) == TRUE &&
    			WriteIoPortByteEx(addrPort, 0x2e) == TRUE &&
    			WriteIoPortByteEx(dataPort, 0x10) == TRUE &&
    			WriteIoPortByteEx(addrPort, 0x2f) == TRUE &&
    			WriteIoPortByteEx(dataPort, static_cast<uint8_t>(offset & 0xFF)) == TRUE &&
    			WriteIoPortByteEx(addrPort, 0x2e) == TRUE &&
    			WriteIoPortByteEx(dataPort, 0x12) == TRUE &&
    			WriteIoPortByteEx(addrPort, 0x2f) == TRUE &&
    			WriteIoPortByteEx(dataPort, value) == TRUE;
	}

	PWTS::ROData<int> OSWindows::getGPDWm2FanRpm(const FanControls &controls) const {
    	static constexpr uint16_t pwmCtrOfft = 0x1841;

    	for (uint16_t i=pwmCtrOfft,l=pwmCtrOfft+2; i<l; ++i) {
    		uint8_t pwmCtr;

    		if (!gpdECRamRead(controls.addrPort, controls.dataPort, i, &pwmCtr))
    			return {};

    		if (pwmCtr != 0xB8 && !gpdECRamWrite(controls.addrPort, controls.dataPort, i, 0xB8))
    			return {};
		}

    	return getGPDFanRpm(controls);
	}

	PWTS::ROData<int> OSWindows::getGPDFanRpm(const FanControls &controls) const {
		uint8_t high, low;

    	if (!gpdECRamRead(controls.addrPort, controls.dataPort, controls.readAdr, &high) ||
    		!gpdECRamRead(controls.addrPort, controls.dataPort, controls.readAdr + 1, &low))
    		return {};

    	return PWTS::ROData<int>((static_cast<uint16_t>(high) << 8) | low, true);
	}

	bool OSWindows::gpdWin4ECInit(const FanControls &controls) const {
    	uint8_t chip_id;

    	if (!gpdECRamRead(controls.addrPort, controls.dataPort, 0x2000, &chip_id)) {
    		if (logger->isLevel(PWTS::LogLevel::Error))
    			logger->write("failed to read chip_id");

    		return false;
    	}

    	if (chip_id == 0x55) {
    		uint8_t chip_ver;

    		if (!gpdECRamRead(controls.addrPort, controls.dataPort, 0x1060, &chip_ver) ||
    			!gpdECRamWrite(controls.addrPort, controls.dataPort, 0x1060, chip_ver | 0x80))
    		{
    			if (logger->isLevel(PWTS::LogLevel::Error))
    				logger->write("failed to init ec");

    			return false;
    		}
    	}

    	if (logger->isLevel(PWTS::LogLevel::Info))
    		logger->write("GPD win 4 EC initialized");

    	return true;
	}

	PWTS::RWData<int> OSWindows::getGPDFanMode(const FanControls &controls) const {
    	uint8_t mode = 0;

    	if (gpdECRamRead(controls.addrPort, controls.dataPort, controls.modeAdr[0], &mode))
    		return PWTS::RWData<int>(mode == 0 ? 0:1, true);

    	return {};
	}

	PWTS::ROData<int> OSWindows::getGPDFanSpeed(const FanControls &controls) const {
    	switch (controls.board) {
    		case FanBoard::GPD_WIN4:
    		case FanBoard::GPD_DUO:
    		case FanBoard::GPD_WIN_MINI:
    		case FanBoard::GPD_MPC2:
    			return getGPDFanRpm(controls);
    		case FanBoard::GPD_WIN_MAX2:
    			return getGPDWm2FanRpm(controls);
    		default:
    			break;
    	}

		return {};
	}

	bool OSWindows::setGPDFanMode(const PWTS::RWData<int> &mode, const FanControls &controls) const {
    	if (!mode.isValid())
    		return true;

    	int fanMode = mode.getValue();

    	switch (controls.board) {
    		case FanBoard::GPD_WIN_MAX2:
    			fanMode = fanMode != 0 ? 1:0;
    			break;
    		default:
    			fanMode = fanMode != 0 ? controls.maxPWM:0;
    			break;
    	}

		for (const uint16_t modeAdr: controls.modeAdr) {
			if (!gpdECRamWrite(controls.addrPort, controls.dataPort, modeAdr, fanMode))
				return false;
		}

		return true;
	}

	bool OSWindows::setGPDFanSpeed(const int speed, const FanControls &controls) const {
    	const PWTS::RWData<int> mode = getGPDFanMode(controls);
    	const int val = qBound(1, controls.maxPWM * speed / 100, static_cast<int>(controls.maxPWM));

    	if (!mode.isValid() || mode.getValue() == 0)
    		return true;

    	for (const uint16_t writeAdr: controls.writeAdr) {
    		if (!gpdECRamWrite(controls.addrPort, controls.dataPort, writeAdr, val))
    			return false;
    	}

		return true;
	}
#endif
}
