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

#include "pwtWin32/win.h"
#include <QRegularExpression>

#include "../OS.h"
#include "pwtShared/Include/OS/Windows/PowerSchemesData.h"

namespace PWTD::WIN {
	class OSWindows final: public OS {
	private:
		struct SchemeGUID final {
			QString friendlyName;
			GUID schemeGUID;
		};

		static constexpr std::array<GUID, 4> nullSchemeGUIDList {{
			{.Data1 = 0x3af9b8d9, .Data2 = 0x7c97, .Data3 = 0x431d, .Data4 = {0xad, 0x78, 0x34, 0xa8, 0xbf, 0xea, 0x43, 0x9f}}, // high performance overlay
			{.Data1 = 0x961cc777, .Data2 = 0x2547, .Data3 = 0x4f9d, .Data4 = {0x81, 0x74, 0x7d, 0x86, 0x18, 0x1b, 0x8a, 0x7a}}, // better battery life overlay
			{.Data1 = 0xded574b5, .Data2 = 0x45a0, .Data3 = 0x4f42, .Data4 = {0x87, 0x37, 0x46, 0x34, 0x5c, 0x09, 0xc2, 0x38}}, // max performance overlay
			{.Data1 = 0xe9a42b02, .Data2 = 0xd5df, .Data3 = 0x448d, .Data4 = {0xaa, 0x00, 0x03, 0xf1, 0x47, 0x49, 0xeb, 0x61}} // ultra performance scheme
		}};
		static constexpr std::array<GUID, 2> nullPowerSettingGUIDList {{
			{.Data1 = 0x12BBEBE6, .Data2 = 0x58D6, .Data3 = 0x4636, .Data4 = {0x95, 0xBB, 0x32, 0x17, 0xEF, 0x86, 0x7C, 0x1A}}, // wlansvc
			{.Data1 = 0x309DCE9B, .Data2 = 0xBEF4, .Data3 = 0x4119, .Data4 = {0x99, 0x21, 0xA8, 0x51, 0xFB, 0x12, 0xF0, 0xF4}} // power setting name (placeholder?)
		}};
		static constexpr TCHAR regSystem[] = LR"(HARDWARE\DESCRIPTION\System\)";
		static constexpr TCHAR regBIOS[] = LR"(HARDWARE\DESCRIPTION\System\BIOS)";
		static constexpr TCHAR regSystemClass[] = LR"(SYSTEM\CurrentControlSet\Control\Class\)";
		static constexpr TCHAR regPowerSettings[] = LR"(SYSTEM\CurrentControlSet\Control\Power\PowerSettings)";
		const QRegularExpression gpuVendorIDRex {R"(ven_([\w\d]+))"};
		const QRegularExpression gpuDeviceIDRex {R"(dev_([\w\d]+))"};
		const QRegularExpression gpuRevisionIDRex {R"(rev_([\w\d]+))"};
		QString displayGUID;

        void fillPackageData(const PWTS::Features &features, const PWTS::DaemonPacket &packet) const;
		void applyPackageSettings(const PWTS::Features &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const;
		void setDisplayGUID();
		[[nodiscard]] QString getGPUMatchingID(int index) const;
		[[nodiscard]] QString GUIDToQString(const GUID &guid) const;
		[[nodiscard]] GUID QStringToGUID(const QString &guidStr) const;
		[[nodiscard]] bool isNullSchemeGUID(const GUID &schemeGuid) const;
		[[nodiscard]] bool isNullPowerSettingGUID(const GUID &settingGuid) const;
		[[nodiscard]] bool hasPowerSettingAC(const GUID &setting) const;
		[[nodiscard]] bool hasPowerSettingDC(const GUID &setting) const;
		[[nodiscard]] QString getPowerSchemeFriendlyNameFor(const GUID *schemeGuid, const GUID *subgroupGuid, const GUID *settingGuid) const;
		[[nodiscard]] QString getSettingDescription(const GUID &settingGuid) const;
		[[nodiscard]] QString getSettingValueUnit(const GUID &subgroupGuid, const GUID &settingGuid) const;
		[[nodiscard]] DWORD getSettingValueIncrement(const GUID &subgroupGuid, const GUID &settingGuid) const;
		PWTS::WIN::PowerSettingValue getPowerValue(const GUID &scheme, const GUID &subGroup, const GUID &setting) const;
		[[nodiscard]] bool setPowerValue(const GUID &scheme, const GUID &subGroup, const GUID &setting, const PWTS::WIN::PowerSettingValue &psetting) const;
		[[nodiscard]] QMap<QString, SchemeGUID> enumeratePowerSchemesAll(HKEY hkey) const;
		[[nodiscard]] QMap<QString, SchemeGUID> enumeratePowerSchemesPartial() const;
		[[nodiscard]] QMap<QString, SchemeGUID> enumeratePowerSchemes() const;
		[[nodiscard]] GUID *duplicatePowerScheme(const GUID &sourceScheme, const QString &name) const;
		[[nodiscard]] bool deletePowerScheme(const GUID &guid) const;
		[[nodiscard]] bool CreatePowerScheme(QMap<QString, SchemeGUID> &guidMap, const GUID &baseGuid, const QString &duplicateTmpGuid, const QString &duplicateName) const;
		[[nodiscard]] LSTATUS isPowerSettingRangeDefined(const QString &subGroup, const QString &setting, bool *result) const;
		void enumeratePowerSettingGroups(const GUID &schemeGuid, PWTS::WIN::PowerSchemesData &pschemeData, PWTS::WIN::PowerScheme &pscheme) const;
		void enumeratePowerSettingsInGroup(const GUID &schemeGuid, const GUID &groupGuid, PWTS::WIN::PowerSchemesData &pschemeData, PWTS::WIN::PowerScheme &pscheme, bool isNoSubGroup) const;
		PWTS::MinMax getPowerSettingRange(const GUID &subGroup, const GUID &setting) const;
		[[nodiscard]] QList<QString> getPowerSettingOptions(const GUID &subGroup, const GUID &setting) const;
		[[nodiscard]] GUID *getActiveSchemeGUID() const;
		[[nodiscard]] QString getActivePowerScheme() const;
		[[nodiscard]] PWTS::WIN::PowerSchemesData getPowerSchemes() const;
		[[nodiscard]] QSet<PWTS::DError> setPowerSchemes(const QMap<QString, PWTS::WIN::PowerScheme> &pschemes, const QString &activeScheme) const;
#ifdef WITH_GPD_FAN
		[[nodiscard]] bool gpdECRamRead(uint8_t addrPort, uint8_t dataPort, uint16_t offset, uint8_t *outVal) const;
		[[nodiscard]] bool gpdECRamWrite(uint8_t addrPort, uint8_t dataPort, uint16_t offset, uint8_t value) const;
		[[nodiscard]] PWTS::ROData<int> getGPDWm2FanRpm(const FanControls &controls) const;
		[[nodiscard]] PWTS::ROData<int> getGPDFanRpm(const FanControls &controls) const;
		[[nodiscard]] PWTS::RWData<int> getGPDFanMode(const FanControls &controls) const;
		[[nodiscard]] PWTS::ROData<int> getGPDFanSpeed(const FanControls &controls) const;
		[[nodiscard]] bool setGPDFanMode(const PWTS::RWData<int> &mode, const FanControls &controls) const;
		[[nodiscard]] bool setGPDFanSpeed(int speed, const FanControls &controls) const;
#endif

	protected:
        [[nodiscard]] QString getBiosVendor() const override;
		[[nodiscard]] QString getBiosVersion() const override;
		[[nodiscard]] QString getBiosDate() const override;
		[[nodiscard]] QString getECVersion() const override;
		[[nodiscard]] QString getProductName() const override;
		[[nodiscard]] QString getManufacturer() const override;
        [[nodiscard]] QString getMicrocodeRevision(int cpu) const override;
		[[nodiscard]] int getOnlineCPUCount(int numLogicalCPUs) const override;
		[[nodiscard]] quint64 getAvailableRam() const override;
		[[nodiscard]] quint64 getSwapSize() const override;

	public:
		OSWindows();

		[[nodiscard]] bool setupOSAccess() const override;
		void unsetOSAccess() const override;
		[[nodiscard]] QSet<PWTS::Feature> getCPUFeatures(int numLogicalCPUs, PWTS::CPUVendor vendor) const override;
		[[nodiscard]] std::pair<PWTS::GPUVendor, QSet<PWTS::Feature>> getGPUFeatures(int index, PWTS::GPUVendor vendor) const override;
		[[nodiscard]] bool hasFanControls() const override;
		void fillDaemonPacket(const PWTS::Features &features, PWTS::CPUVendor cpuVendor, int numLogicalCPUs, PWTS::DaemonPacket &packet) const override;
        [[nodiscard]] QSet<PWTS::DError> applySettings(const PWTS::Features &features, PWTS::CPUVendor cpuVendor, int numLogicalCPUs, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) const override;
		[[nodiscard]] QList<int> getCPUCoreIndexList() const override;
		[[nodiscard]] QList<int> getGPUIndexList() const override;
		[[nodiscard]] PWTS::GPUVendor getGPUVendor(int index) const override;
		[[nodiscard]] QString getGPUDeviceID(int index) const override;
		[[nodiscard]] QString getGPURevisionID(int index) const override;
		[[nodiscard]] QString getGPUVBiosVersion(int index) const override;
		[[nodiscard]] QString getFanControlPath(FanBoard type) const override;
		[[nodiscard]] PWTS::RWData<int> getFanMode(const FanControls &controls) const override;
		[[nodiscard]] PWTS::ROData<int> getFanSpeed(const FanControls &controls) const override;
		[[nodiscard]] bool setFanMode(const FanControls &controls, const PWTS::RWData<int> &mode) const override;
		[[nodiscard]] bool setFanSpeed(const FanControls &controls, int speed) const override;
#ifdef WITH_GPD_FAN
		[[nodiscard]] bool gpdWin4ECInit(const FanControls &controls) const;
#endif
	};
}
