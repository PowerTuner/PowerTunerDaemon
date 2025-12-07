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

#include "../Registers/IA32_ENERGY_PERF_BIAS.h"
#include "../Registers/IA32_MISC_ENABLE.h"
#include "../Registers/IA32_PM_ENABLE.h"
#include "../Registers/IA32_PACKAGE_THERM_STATUS.h"
#include "../Registers/IA32_HWP_CAPABILITIES.h"
#include "../Registers/IA32_HWP_REQUEST_PKG.h"
#include "../Registers/IA32_HWP_REQUEST.h"
#include "../Registers/IA32_HWP_CTL.h"
#include "../Registers/MSR_PLATFORM_INFO/MSR_PLATFORM_INFO.h"
#include "../Registers/MSR_PKG_POWER_LIMIT.h"
#include "../Registers/MSR_VR_CURRENT_CONFIG/MSR_VR_CURRENT_CONFIG.h"
#include "../Registers/MSR_PP1_CURRENT_CONFIG.h"
#include "../Registers/MSR_TURBO_POWER_CURRENT_LIMIT.h"
#include "../Registers/MSR_PP0_POLICY.h"
#include "../Registers/MSR_PP1_POLICY.h"
#include "../Registers/MSR_TURBO_RATIO_LIMIT.h"
#include "../Registers/MSR_POWER_CTL/MSR_POWER_CTL.h"
#include "../Registers/MSR_PKG_CST_CONFIG_CONTROL/MSR_PKG_CST_CONFIG_CONTROL.h"
#include "../Registers/MSR_MISC_PWR_MGMT/MSR_MISC_PWR_MGMT.h"
#include "../Registers/MSR_UNK_FIVR_CONTROL/MSR_UNK_FIVR_CONTROL.h"
#include "../Registers/MSR_TEMPERATURE_TARGET/MSR_TEMPERATURE_TARGET.h"

#include "../Registers/MCHBAR_PACKAGE_RAPL_LIMIT/MCHBAR_PACKAGE_RAPL_LIMIT.h"
