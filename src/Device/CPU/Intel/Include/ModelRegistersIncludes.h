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

// nehalem
#include "../Registers/MSR_POWER_CTL/MSR_POWER_CTL_NHLM.h"
#include "../Registers/MSR_PLATFORM_INFO/MSR_PLATFORM_INFO_NHLM.h"
#include "../Registers/MSR_MISC_PWR_MGMT/MSR_MISC_PWR_MGMT_NHLM.h"
#include "../Registers/MSR_TEMPERATURE_TARGET/MSR_TEMPERATURE_TARGET_NHLM.h"

// sandy bridge
#include "../Registers/MSR_PKG_CST_CONFIG_CONTROL/MSR_PKG_CST_CONFIG_CONTROL_SB.h"
#include "../Registers/MSR_POWER_CTL/MSR_POWER_CTL_SB.h"
#include "../Registers/MSR_VR_CURRENT_CONFIG/MSR_VR_CURRENT_CONFIG_SB.h"

// ivy bridge
#include "../Registers/MSR_PLATFORM_INFO/MSR_PLATFORM_INFO_IVB.h"

// ice lake
#include "../Registers/MSR_UNK_FIVR_CONTROL/MSR_UNK_FIVR_CONTROL_ICL.h"
#include "../Registers/MCHBAR_PACKAGE_RAPL_LIMIT/MCHBAR_PACKAGE_RAPL_LIMIT_IVB.h"

// tiger lake
#include "../Registers/MCHBAR_PACKAGE_RAPL_LIMIT/MCHBAR_PACKAGE_RAPL_LIMIT_TGL.h"

// core ultra series 1
#include "../Registers/MSR_PKG_CST_CONFIG_CONTROL/MSR_PKG_CST_CONFIG_CONTROL_CU1.h"
#include "../Registers/MSR_POWER_CTL/MSR_POWER_CTL_CU1.h"
#include "../Registers/MSR_VR_CURRENT_CONFIG/MSR_VR_CURRENT_CONFIG_CU1.h"
