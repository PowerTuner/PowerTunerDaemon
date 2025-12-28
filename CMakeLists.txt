cmake_minimum_required(VERSION 3.23)

project(PowerTunerDaemon VERSION 1.1 DESCRIPTION "Official PowerTuner daemon service" LANGUAGES CXX)

option(WITH_INTEL "Enable support for Intel CPUs" ON)
option(WITH_AMD "Enable support for AMD CPUs" ON)
option(WITH_GPD_FAN "Enable support for GPD fan control" ON)
option(ENABLE_DBUS_SERVICES "Enable support for wake from sleep and battery status change events on linux" ON)
option(WITH_SYSTEMD_NOTIFY "Enable systemd notifications, required when running as systemd service" ON)

set(PROJECT_AUTHOR "kylon")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(PRIV_DEFS "")
set(LINK_LIBS "")
set(LINK_DIRS "")

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Network)

configure_file(src/Resources/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

add_subdirectory(src/external/libPWTShared)

set(ENABLE_DOCS OFF)
set(LIBCPUID_DISABLE_DEPRECATED ON)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(src/external/libcpuid EXCLUDE_FROM_ALL)
unset(BUILD_SHARED_LIBS)

set(PROJECT_SOURCES
	src/Device/Device.h
	src/Device/Device.cpp

	src/Device/CPU/Utils/CPUUtils.cpp
	src/Device/CPU/Utils/CPUUtils.h
	src/Device/CPU/Utils/MSR/MSRFactory.h
	src/Device/CPU/Utils/MSR/MSR.h
	src/Device/CPU/Utils/MSR/MSRNull.h
	src/Device/CPU/Utils/Memory/MemoryFactory.h
	src/Device/CPU/Utils/Memory/Memory.h
	src/Device/CPU/Utils/Memory/MemoryNull.h

	src/Device/FAN/Include/FanControls.h
	src/Device/FAN/Include/FanBoard.h
	src/Device/FAN/Include/FanType.h
	src/Device/FAN/FANDevice.h
	src/Device/FAN/FANDevice.cpp
	src/Device/FAN/CPUFANDevice.h
	src/Device/FAN/FANFactory.h
	src/Device/FAN/FANFactory.cpp

	src/Device/CPU/CPUDevice.h
	src/Device/CPU/CPUDevice.cpp
	src/Device/CPU/CPUDeviceFactory.h
	src/Device/CPU/CPUDeviceFactory.cpp
	src/Device/CPU/CPURegister.h

	src/Device/GPU/GPUDevice.cpp
	src/Device/GPU/GPUDevice.h
	src/Device/GPU/GPUDeviceFactory.h
	src/Device/GPU/GPUDeviceFactory.cpp
	src/Device/GPU/AMD/AMDGPU.h
	src/Device/GPU/Intel/IntelGPU.h
	src/Device/GPU/NVIDIA/NVIDIAGPU.h

	src/Device/OS/OS.cpp
	src/Device/OS/OS.h
	src/Device/OS/OSFactory.h
	src/Device/OS/OSFactory.cpp

	src/Utils/DaemonUtils.h
	src/Utils/DaemonUtils.cpp
	src/Utils/ProcessStatus.h
	src/Utils/FileLogger/FileLogger.h
	src/Utils/FileLogger/FileLogger.cpp
	src/Utils/AppDataPath.h
	src/Utils/AppDataPath.cpp

	src/DiskManagers/ProfileUtils/FAN/ProfileFanUtils.h
	src/DiskManagers/ProfileUtils/FAN/ProfileFanUtils.cpp
	src/DiskManagers/ProfileDiskManager.cpp
	src/DiskManagers/ProfileDiskManager.h
	src/DiskManagers/DaemonSettingDiskManager.cpp
	src/DiskManagers/DaemonSettingDiskManager.h

	src/Service/PowerNotifications/PowerNotifications.h
	src/Service/PowerNotifications/PowerNotificationsFactory.h
	src/Service/Workers/ServiceWorker.h
	src/Service/Workers/ServiceWorker.cpp
	src/Service/DaemonService.cpp
	src/Service/DaemonService.h

	src/Daemon/PowerTunerDaemon.h
	src/Daemon/PowerTunerDaemon.cpp
	src/Daemon/SignalNotifier.h
	src/main.cpp
)

if (LINUX)
	list(APPEND LINK_LIBS
		pci
		kmod
	)

	if (WITH_SYSTEMD_NOTIFY)
		list(APPEND PRIV_DEFS SYSTEMD_NOTIFY)
		list(APPEND LINK_LIBS systemd)
	endif ()

	list(APPEND PROJECT_SOURCES
		src/Daemon/Linux/PowerTunerDaemonLinux.h
		src/Daemon/Linux/PowerTunerDaemonLinux.cpp

		src/Device/OS/Linux/OSLinux.cpp
		src/Device/OS/Linux/OSLinux.h

		src/Device/CPU/Utils/Memory/OS/Linux/MemoryLinux.cpp
		src/Device/CPU/Utils/Memory/OS/Linux/MemoryLinux.h
		src/Device/CPU/Utils/MSR/OS/Linux/MSRLinux.cpp
		src/Device/CPU/Utils/MSR/OS/Linux/MSRLinux.h

		src/Service/PowerNotifications/Linux/PowerNotificationsLinux.h
		src/Service/PowerNotifications/Linux/PowerNotificationsLinux.cpp

		src/DiskManagers/ProfileUtils/OS/ProfileLinuxUtils.h
		src/DiskManagers/ProfileUtils/OS/ProfileLinuxUtils.cpp
	)

	if (ENABLE_DBUS_SERVICES)
		message(STATUS "${PROJECT_NAME}: DBus services support is enabled")

		find_package(QT NAMES Qt6 COMPONENTS DBus REQUIRED)
		find_package(Qt${QT_VERSION_MAJOR} COMPONENTS DBus REQUIRED)

		list(APPEND PRIV_DEFS DBUS_SERVICES)
		list(APPEND LINK_LIBS Qt${QT_VERSION_MAJOR}::DBus)

		list(APPEND PROJECT_SOURCES
			src/Device/OS/Linux/DBusServices.cpp
			src/Device/OS/Linux/DBusServices.h
		)
	endif ()
elseif (WIN32)
	add_subdirectory(src/external/libPWTWin32)
	configure_file(src/Resources/Windows/win.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/win.rc)

	list(APPEND PROJECT_SOURCES
		src/Daemon/Windows/SVCSigHandler.h
		src/Daemon/Windows/SVCWorker.h
		src/Daemon/Windows/SVCWorker.cpp
		src/Daemon/Windows/PowerTunerDaemonWindows.h
		src/Daemon/Windows/PowerTunerDaemonWindows.cpp

		src/Device/OS/Windows/OSWindows.h
		src/Device/OS/Windows/OSWindows.cpp

		src/Device/CPU/Utils/Memory/OS/Windows/MemoryWindows.h
		src/Device/CPU/Utils/Memory/OS/Windows/MemoryWindows.cpp
		src/Device/CPU/Utils/MSR/OS/Windows/MSRWindows.h
		src/Device/CPU/Utils/MSR/OS/Windows/MSRWindows.cpp

		src/Service/PowerNotifications/Windows/PowerNotificationsWindows.h
		src/Service/PowerNotifications/Windows/PowerNotificationsWindows.cpp

		src/DiskManagers/ProfileUtils/OS/ProfileWindowsUtils.h
		src/DiskManagers/ProfileUtils/OS/ProfileWindowsUtils.cpp

		win.rc
	)

	list(APPEND LINK_LIBS
		PowrProf
		Ole32
		WinRing0x64
		PWT::WIN32
	)

	list(APPEND LINK_DIRS
		src/external/winring0
	)
endif ()

if (WITH_INTEL)
	message(STATUS "${PROJECT_NAME}: Intel CPU support is enabled")
	list(APPEND PRIV_DEFS WITH_INTEL)

	list(APPEND PROJECT_SOURCES
		src/Device/CPU/Intel/Registers/IA32_ENERGY_PERF_BIAS.h
		src/Device/CPU/Intel/Registers/IA32_MISC_ENABLE.h
		src/Device/CPU/Intel/Registers/IA32_PM_ENABLE.h
		src/Device/CPU/Intel/Registers/IA32_PACKAGE_THERM_STATUS.h
		src/Device/CPU/Intel/Registers/IA32_HWP_CAPABILITIES.h
		src/Device/CPU/Intel/Registers/IA32_HWP_REQUEST_PKG.h
		src/Device/CPU/Intel/Registers/IA32_HWP_REQUEST.h
		src/Device/CPU/Intel/Registers/IA32_HWP_CTL.h
		src/Device/CPU/Intel/Registers/MSR_PLATFORM_INFO/MSR_PLATFORM_INFO.h
		src/Device/CPU/Intel/Registers/MSR_PLATFORM_INFO/MSR_PLATFORM_INFO_NHLM.h
		src/Device/CPU/Intel/Registers/MSR_PLATFORM_INFO/MSR_PLATFORM_INFO_IVB.h
		src/Device/CPU/Intel/Registers/MSR_TURBO_POWER_CURRENT_LIMIT.h
		src/Device/CPU/Intel/Registers/MSR_RAPL_POWER_UNIT.h
		src/Device/CPU/Intel/Registers/MSR_PKG_POWER_LIMIT.h
		src/Device/CPU/Intel/Registers/MSR_PP0_POLICY.h
		src/Device/CPU/Intel/Registers/MSR_PP1_POLICY.h
		src/Device/CPU/Intel/Registers/MSR_UNK_FIVR_CONTROL/MSR_UNK_FIVR_CONTROL.h
		src/Device/CPU/Intel/Registers/MSR_UNK_FIVR_CONTROL/MSR_UNK_FIVR_CONTROL_ICL.h
		src/Device/CPU/Intel/Registers/MSR_TURBO_RATIO_LIMIT.h
		src/Device/CPU/Intel/Registers/MSR_VR_CURRENT_CONFIG/MSR_VR_CURRENT_CONFIG.h
		src/Device/CPU/Intel/Registers/MSR_VR_CURRENT_CONFIG/MSR_VR_CURRENT_CONFIG_SB.h
		src/Device/CPU/Intel/Registers/MSR_VR_CURRENT_CONFIG/MSR_VR_CURRENT_CONFIG_CU1.h
		src/Device/CPU/Intel/Registers/MSR_PP1_CURRENT_CONFIG.h
		src/Device/CPU/Intel/Registers/MSR_POWER_CTL/MSR_POWER_CTL.h
		src/Device/CPU/Intel/Registers/MSR_POWER_CTL/MSR_POWER_CTL_NHLM.h
		src/Device/CPU/Intel/Registers/MSR_POWER_CTL/MSR_POWER_CTL_SB.h
		src/Device/CPU/Intel/Registers/MSR_POWER_CTL/MSR_POWER_CTL_CU1.h
		src/Device/CPU/Intel/Registers/MSR_MISC_PWR_MGMT/MSR_MISC_PWR_MGMT.h
		src/Device/CPU/Intel/Registers/MSR_MISC_PWR_MGMT/MSR_MISC_PWR_MGMT_NHLM.h
		src/Device/CPU/Intel/Registers/MSR_PKG_CST_CONFIG_CONTROL/MSR_PKG_CST_CONFIG_CONTROL.h
		src/Device/CPU/Intel/Registers/MSR_PKG_CST_CONFIG_CONTROL/MSR_PKG_CST_CONFIG_CONTROL_SB.h
		src/Device/CPU/Intel/Registers/MSR_PKG_CST_CONFIG_CONTROL/MSR_PKG_CST_CONFIG_CONTROL_CU1.h
		src/Device/CPU/Intel/Registers/MSR_TEMPERATURE_TARGET/MSR_TEMPERATURE_TARGET.h
		src/Device/CPU/Intel/Registers/MSR_TEMPERATURE_TARGET/MSR_TEMPERATURE_TARGET_NHLM.h

		src/Device/CPU/Intel/Registers/MCHBAR_PACKAGE_POWER_SKU_UNIT.h
		src/Device/CPU/Intel/Registers/MCHBAR_PACKAGE_RAPL_LIMIT/MCHBAR_PACKAGE_RAPL_LIMIT.h
		src/Device/CPU/Intel/Registers/MCHBAR_PACKAGE_RAPL_LIMIT/MCHBAR_PACKAGE_RAPL_LIMIT_IVB.h
		src/Device/CPU/Intel/Registers/MCHBAR_PACKAGE_RAPL_LIMIT/MCHBAR_PACKAGE_RAPL_LIMIT_TGL.h

		src/Device/CPU/Intel/IntelCPU.h
		src/Device/CPU/Intel/IntelCPU.cpp

		src/Device/CPU/Intel/MCHBAR/MCHBAR.h
		src/Device/CPU/Intel/MCHBAR/MCHBAR.cpp
		src/Device/CPU/Intel/MCHBAR/MCHBARRegister.h
		src/Device/CPU/Intel/MCHBAR/MCHBARUtils.h
		src/Device/CPU/Intel/MCHBAR/MCHBARUtils.cpp

		src/Device/CPU/Intel/Include/CPUFamily.h
		src/Device/CPU/Intel/Include/CPUModel.h
		src/Device/CPU/Intel/Include/ModelRegistersIncludes.h
		src/Device/CPU/Intel/Include/RegistersIncludes.h
		src/Device/CPU/Intel/Include/HWPActivityWindowBits.h
		src/Device/CPU/Intel/Include/PlatformInfoData.h
		src/Device/CPU/Intel/Include/PowerLimitRawTimeWindow.h

		src/Device/CPU/Intel/Utils/IntelRegisterUtils.cpp
		src/Device/CPU/Intel/Utils/IntelRegisterUtils.h

		src/DiskManagers/ProfileUtils/Vendor/ProfileIntelUtils.h
		src/DiskManagers/ProfileUtils/Vendor/ProfileIntelUtils.cpp
	)
endif ()

if (WITH_AMD)
	message(STATUS "${PROJECT_NAME}: AMD CPU support is enabled")

	set(BUILD_SHARED_LIBS OFF)
	add_subdirectory(src/external/RyzenAdj EXCLUDE_FROM_ALL)
	unset(BUILD_SHARED_LIBS)

	list(APPEND PRIV_DEFS WITH_AMD)
	list(APPEND LINK_LIBS RYADJ::RyzenAdjLib)

	list(APPEND PROJECT_SOURCES
		src/Device/CPU/AMD/Registers/MSR_PSTATE_CURRENT_LIMIT.h
		src/Device/CPU/AMD/Registers/MSR_PSTATE_CONTROL.h
		src/Device/CPU/AMD/Registers/MSR_CORE_PERFORMANCE_BOOST.h
		src/Device/CPU/AMD/Registers/MSR_CPPC_ENABLE.h
		src/Device/CPU/AMD/Registers/MSR_CPPC_CAPABILITY_1.h
		src/Device/CPU/AMD/Registers/MSR_CPPC_REQUEST.h

		src/Device/CPU/AMD/SMU/RyzenAdj.h
		src/Device/CPU/AMD/SMU/RyzenAdj.cpp
		src/Device/CPU/AMD/AMDCPU.h
		src/Device/CPU/AMD/AMDCPU.cpp

		src/DiskManagers/ProfileUtils/Vendor/ProfileAMDUtils.h
		src/DiskManagers/ProfileUtils/Vendor/ProfileAMDUtils.cpp
	)

	if (LINUX)
		list(APPEND PROJECT_SOURCES
			src/DiskManagers/ProfileUtils/OS/ProfileLinuxAMDUtils.h
			src/DiskManagers/ProfileUtils/OS/ProfileLinuxAMDUtils.cpp
		)
	endif ()
endif ()

if (WITH_GPD_FAN)
	message(STATUS "${PROJECT_NAME}: GPD fan support is enabled")
	list(APPEND PRIV_DEFS WITH_GPD_FAN)

	list(APPEND PROJECT_SOURCES
		src/Device/FAN/GPD/GPDWin4FanBoard.h
		src/Device/FAN/GPD/GPDWinMiniFanBoard.h
		src/Device/FAN/GPD/GPDWinMax2FanBoard.h
		src/Device/FAN/GPD/GPDDUOFanBoard.h
		src/Device/FAN/GPD/GPDMPC2FanBoard.h
	)
endif ()

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include(CheckIPOSupported)
check_ipo_supported(RESULT has_ipo OUTPUT ipo_error)
if (has_ipo)
	message(STATUS "${PROJECT_NAME}: IPO/LTO enabled")
	set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

list(LENGTH LINK_DIRS link_dirs_len)
if (${link_dirs_len} GREATER 0)
	target_link_directories(${PROJECT_NAME} PRIVATE ${LINK_DIRS})
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE ${PRIV_DEFS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core Qt::Network PWT::Shared cpuid ${LINK_LIBS})

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
	BUNDLE  DESTINATION .
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
	TARGET ${PROJECT_NAME}
	OUTPUT_SCRIPT deploy_script
	NO_TRANSLATIONS
	NO_COMPILER_RUNTIME
	NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
