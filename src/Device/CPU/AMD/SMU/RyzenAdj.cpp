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
#include <thread>

#include "RyzenAdj.h"
#include "../../../../Utils/Utils.h"

namespace PWTD::AMD {
    using namespace Qt::StringLiterals;

    RyzenAdj::RyzenAdj(LIBPWT::DeviceData &devData) {
        std::unordered_set<LIBPWT::Feature> &features = devData.daemonInfo->features.cpu;
        const ADJ_ERROR res = ryzenadj_init();

        if (res != ADJ_OK) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to init: %1"_s.arg(res));
            return;
        }

        initialized = true;

        if (ryzenadj_can_write(ADJ_OPT_STAPM_LIMIT) == ADJ_OK)
            features.insert({LIBPWT::Feature::AMD_RY_STAPM_LIMIT_W, LIBPWT::Feature::AMD_RY_STAPM_LIMIT_R});

        if (ryzenadj_can_write(ADJ_OPT_FAST_LIMIT) == ADJ_OK)
            features.insert({LIBPWT::Feature::AMD_RY_FAST_LIMIT_W, LIBPWT::Feature::AMD_RY_FAST_LIMIT_R});

        if (ryzenadj_can_write(ADJ_OPT_SLOW_LIMIT) == ADJ_OK)
            features.insert({LIBPWT::Feature::AMD_RY_SLOW_LIMIT_W, LIBPWT::Feature::AMD_RY_SLOW_LIMIT_R});

        if (ryzenadj_can_write(ADJ_OPT_TCTL_TEMP) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_TCTL_TEMP_W);

        if (ryzenadj_can_read(ADJ_OPT_TCTL_TEMP) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_TCTL_TEMP_R);

        if (ryzenadj_can_read_value(ADJ_OPT_TCTL_TEMP) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_TCTL_TEMP_VAL);

        if (ryzenadj_can_write(ADJ_OPT_SLOW_LIMIT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_APU_SLOW_W);

        if (ryzenadj_can_read(ADJ_OPT_SLOW_LIMIT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_APU_SLOW_R);

        if (ryzenadj_can_write(ADJ_OPT_APU_SKIN_TEMP_LIMIT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_APU_SKIN_TEMP_W);

        if (ryzenadj_can_read(ADJ_OPT_APU_SKIN_TEMP_LIMIT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_APU_SKIN_TEMP_R);

        if (ryzenadj_can_write(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_DGPU_SKIN_TEMP_W);

        if (ryzenadj_can_read(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_DGPU_SKIN_TEMP_R);

        if (ryzenadj_can_write(ADJ_OPT_VRM_CURRENT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_VRM_CURRENT_W);

        if (ryzenadj_can_read(ADJ_OPT_VRM_CURRENT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_VRM_CURRENT_R);

        if (ryzenadj_can_write(ADJ_OPT_VRMSOC_CURRENT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_VRM_SOC_CURRENT_W);

        if (ryzenadj_can_read(ADJ_OPT_VRMSOC_CURRENT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_VRM_SOC_CURRENT_R);

        if (ryzenadj_can_write(ADJ_OPT_VRMMAX_CURRENT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_VRM_MAX_CURRENT_W);

        if (ryzenadj_can_read(ADJ_OPT_VRMMAX_CURRENT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_VRM_MAX_CURRENT_R);

        if (ryzenadj_can_write(ADJ_OPT_VRMSOCMAX_CURRENT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_W);

        if (ryzenadj_can_read(ADJ_OPT_VRMSOCMAX_CURRENT) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_R);

        if (ryzenadj_can_write(ADJ_OPT_CCLK_SETPOINT) == ADJ_OK && ryzenadj_can_write(ADJ_OPT_CCLK_BUSY) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_POWER_PROFILE_W);

        if (ryzenadj_can_write(ADJ_OPT_COPER) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_CO_PER_W);

        if (ryzenadj_can_write(ADJ_OPT_COALL) == ADJ_OK)
            features.insert(LIBPWT::Feature::AMD_RY_CO_ALL_W);

        if constexpr (isWindows()) {
            if (ryzenadj_can_write(ADJ_OPT_GFX_CLK) == ADJ_OK)
                features.insert(LIBPWT::Feature::AMD_RY_STATIC_GFX_CLK_W);

            if (ryzenadj_can_write(ADJ_OPT_MIN_GFXCLK_FREQ) == ADJ_OK)
                features.insert(LIBPWT::Feature::AMD_RY_MIN_GFX_CLOCK_W);

            if (ryzenadj_can_read(ADJ_OPT_MIN_GFXCLK_FREQ) == ADJ_OK)
                features.insert(LIBPWT::Feature::AMD_RY_MIN_GFX_CLOCK_R);

            if (ryzenadj_can_write(ADJ_OPT_MAX_GFXCLK_FREQ) == ADJ_OK)
                features.insert(LIBPWT::Feature::AMD_RY_MAX_GFX_CLOCK_W);

            if (ryzenadj_can_read(ADJ_OPT_MAX_GFXCLK_FREQ) == ADJ_OK)
                features.insert(LIBPWT::Feature::AMD_RY_MAX_GFX_CLOCK_R);
        }
    }

    RyzenAdj::~RyzenAdj() {
        ryzenadj_cleanup();
    }

    bool RyzenAdj::set(const ADJ_OPT opt, const int value) const {
        const ADJ_ERROR err = ryzenadj_set(opt, value);

        if (err != ADJ_OK) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to set option %1: code %2"_s.arg(opt).arg(err));

            return false;
        }

        return true;
    }

    std::optional<int> RyzenAdj::get(const ADJ_OPT opt, const float valueMult) const {
        ADJ_ERROR err;
        const float value = ryzenadj_get(opt, &err);

        if (err != ADJ_OK || std::isnan(value)) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to get option %1: code %2"_s.arg(opt).arg(err));

            return {};
        }

        return static_cast<int>(value * valueMult);
    }

    std::optional<int> RyzenAdj::getValue(const ADJ_OPT opt, const float valueMult) const {
        ADJ_ERROR err;
        const float value = ryzenadj_get_value(opt, &err);

        if (err != ADJ_OK || std::isnan(value)) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to read value %1: code %2"_s.arg(opt).arg(err));

            return {};
        }

        return static_cast<int>(value * valueMult);
    }

    bool RyzenAdj::refreshTable() const {
        using namespace std::chrono_literals;

        const ADJ_ERROR ret = ryzenadj_refresh_table();

        if (ret != ADJ_OK) {
            if (logger.isLevel(LIBPWT::LogLevel::Error))
                logger.write(u"failed to refresh table: code %1"_s.arg(ret));

            return false;
        }

        // refresh table is a cmd, give it some time to complete so we don't read old values
        std::this_thread::sleep_for(8ms);
        return true;
    }

    void RyzenAdj::read(LIBPWT::DeviceData &devData) const {
        const std::unordered_set<LIBPWT::Feature> &features = devData.daemonInfo->features.cpu;

        if (!refreshTable())
            return devData.errors.push_back(LIBPWT::Error::RY_REFRESH_TABLE);

        if (features.contains(LIBPWT::Feature::AMD_RY_STAPM_LIMIT_R)) {
            devData.amdMsr->stapmLimit = get(ADJ_OPT_STAPM_LIMIT, 1000);

            if (!devData.amdMsr->stapmLimit)
                devData.errors.push_back(LIBPWT::Error::R_RY_STAPM_LIMIT);
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_FAST_LIMIT_R)) {
            devData.amdMsr->fastLimit = get(ADJ_OPT_FAST_LIMIT, 1000);

            if (!devData.amdMsr->fastLimit)
                devData.errors.push_back(LIBPWT::Error::R_RY_FAST_LIMIT);
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_SLOW_LIMIT_R)) {
            devData.amdMsr->slowLimit = get(ADJ_OPT_SLOW_LIMIT, 1000);

            if (!devData.amdMsr->slowLimit)
                devData.errors.push_back(LIBPWT::Error::R_RY_SLOW_LIMIT);
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_TCTL_TEMP_R)) {
            devData.amdMsr->tctlTemp = get(ADJ_OPT_TCTL_TEMP, 1);

            if (!devData.amdMsr->tctlTemp)
                devData.errors.push_back(LIBPWT::Error::R_RY_TCTL_TEMP);
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_APU_SLOW_R)) {
            devData.amdMsr->apuSlow = get(ADJ_OPT_APU_SLOW_LIMIT, 1000);

            if (!devData.amdMsr->apuSlow)
                devData.errors.push_back(LIBPWT::Error::R_RY_APU_SLOW);
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_APU_SKIN_TEMP_R)) {
            devData.amdMsr->apuSkinTemp = get(ADJ_OPT_APU_SKIN_TEMP_LIMIT, 1);

            if (!devData.amdMsr->apuSkinTemp)
                devData.errors.push_back(LIBPWT::Error::R_RY_APU_SKIN_TEMP);
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_DGPU_SKIN_TEMP_R)) {
            devData.amdMsr->dgpuSkinTemp = get(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT, 1);

            if (!devData.amdMsr->dgpuSkinTemp)
                devData.errors.push_back(LIBPWT::Error::R_RY_DGPU_SKIN_TEMP);
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_VRM_CURRENT_R)) {
            devData.amdMsr->vrmCurrent = get(ADJ_OPT_VRM_CURRENT, 1000);

            if (!devData.amdMsr->vrmCurrent)
                devData.errors.push_back(LIBPWT::Error::R_RY_VRM_CURRENT);
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_VRM_SOC_CURRENT_R)) {
            devData.amdMsr->vrmSocCurrent = get(ADJ_OPT_VRMSOC_CURRENT, 1000);

            if (!devData.amdMsr->vrmSocCurrent)
                devData.errors.push_back(LIBPWT::Error::R_RY_VRM_SOC_CURRENT);
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_VRM_MAX_CURRENT_R)) {
            devData.amdMsr->vrmMaxCurrent = get(ADJ_OPT_VRMMAX_CURRENT, 1000);

            if (!devData.amdMsr->vrmMaxCurrent)
                devData.errors.push_back(LIBPWT::Error::R_RY_VRM_MAX_CURRENT);
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_R)) {
            devData.amdMsr->vrmSocMaxCurrent = get(ADJ_OPT_VRMSOCMAX_CURRENT, 1000);

            if (!devData.amdMsr->vrmSocMaxCurrent)
                devData.errors.push_back(LIBPWT::Error::R_RY_VRM_SOC_MAX_CURRENT);
        }

        if constexpr (isWindows()) {
            if (features.contains(LIBPWT::Feature::AMD_RY_MIN_GFX_CLOCK_R)) {
                devData.amdMsr->minGfxClock = get(ADJ_OPT_MIN_GFXCLK_FREQ, 1);

                if (!devData.amdMsr->minGfxClock)
                    devData.errors.push_back(LIBPWT::Error::R_RY_MIN_GFX_CLOCK);
            }

            if (features.contains(LIBPWT::Feature::AMD_RY_MAX_GFX_CLOCK_R)) {
                devData.amdMsr->maxGfxClock = get(ADJ_OPT_MAX_GFXCLK_FREQ, 1);

                if (!devData.amdMsr->maxGfxClock)
                    devData.errors.push_back(LIBPWT::Error::R_RY_MAX_GFX_CLOCK);
            }
        }
    }

    void RyzenAdj::writeCore(const int cpu, LIBPWT::DeviceData &devData, const LIBPWT::DeviceData &clientData, std::vector<LIBPWT::Error> &errors) const {
        const std::unordered_set<LIBPWT::Feature> &features = devData.daemonInfo->features.cpu;
        const LIBPWT::AMD::MSRCore &clientCore = clientData.amdMsr->cores[cpu];
        LIBPWT::AMD::MSRCore &devCore = devData.amdMsr->cores[cpu];

        if (features.contains(LIBPWT::Feature::AMD_RY_CO_PER_W) && clientCore.curveOptimizer) {
            if (!set(ADJ_OPT_COPER, (cpu << 20) | (*clientCore.curveOptimizer & 0xffff)))
                errors.push_back(LIBPWT::Error::W_RY_CO_PER);
            else
                devCore.curveOptimizer = clientCore.curveOptimizer;
        }
    }

    std::vector<LIBPWT::Error> RyzenAdj::write(LIBPWT::DeviceData &devData, const LIBPWT::DeviceData &clientData) const {
        const std::unordered_set<LIBPWT::Feature> &features = devData.daemonInfo->features.cpu;
        std::vector<LIBPWT::Error> errors;

        if (features.contains(LIBPWT::Feature::AMD_RY_APU_SLOW_W) && clientData.amdMsr->apuSlow && !set(ADJ_OPT_APU_SLOW_LIMIT, *clientData.amdMsr->apuSlow))
            errors.push_back(LIBPWT::Error::W_RY_APU_SLOW);

        if (features.contains(LIBPWT::Feature::AMD_RY_STAPM_LIMIT_W) && clientData.amdMsr->stapmLimit && !set(ADJ_OPT_STAPM_LIMIT, *clientData.amdMsr->stapmLimit))
            errors.push_back(LIBPWT::Error::W_RY_STAPM_LIMIT);

        if (features.contains(LIBPWT::Feature::AMD_RY_SLOW_LIMIT_W) && clientData.amdMsr->slowLimit && !set(ADJ_OPT_SLOW_LIMIT, *clientData.amdMsr->slowLimit))
            errors.push_back(LIBPWT::Error::W_RY_SLOW_LIMIT);

        if (features.contains(LIBPWT::Feature::AMD_RY_FAST_LIMIT_W) && clientData.amdMsr->fastLimit && !set(ADJ_OPT_FAST_LIMIT, *clientData.amdMsr->fastLimit))
            errors.push_back(LIBPWT::Error::W_RY_FAST_LIMIT);

        if (features.contains(LIBPWT::Feature::AMD_RY_TCTL_TEMP_W) && clientData.amdMsr->tctlTemp) {
            if (!set(ADJ_OPT_TCTL_TEMP, *clientData.amdMsr->tctlTemp))
                errors.push_back(LIBPWT::Error::W_RY_TCTL_TEMP);
            else if (!features.contains(LIBPWT::Feature::AMD_RY_TCTL_TEMP_R))
                devData.amdMsr->tctlTemp = clientData.amdMsr->tctlTemp;
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_APU_SKIN_TEMP_W) && clientData.amdMsr->apuSkinTemp) {
            if (!set(ADJ_OPT_APU_SKIN_TEMP_LIMIT, *clientData.amdMsr->apuSkinTemp))
                errors.push_back(LIBPWT::Error::W_RY_APU_SKIN_TEMP);
            else if (!features.contains(LIBPWT::Feature::AMD_RY_APU_SKIN_TEMP_R))
                devData.amdMsr->apuSkinTemp = clientData.amdMsr->apuSkinTemp;
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_DGPU_SKIN_TEMP_W) && clientData.amdMsr->dgpuSkinTemp) {
            if (!set(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT, *clientData.amdMsr->dgpuSkinTemp))
                errors.push_back(LIBPWT::Error::W_RY_DGPU_SKIN_TEMP);
            else if (!features.contains(LIBPWT::Feature::AMD_RY_DGPU_SKIN_TEMP_R))
                devData.amdMsr->dgpuSkinTemp = clientData.amdMsr->dgpuSkinTemp;
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_VRM_CURRENT_W) && clientData.amdMsr->vrmCurrent) {
            if (!set(ADJ_OPT_VRM_CURRENT, *clientData.amdMsr->vrmCurrent))
                errors.push_back(LIBPWT::Error::W_RY_VRM_CURRENT);
            else if (!features.contains(LIBPWT::Feature::AMD_RY_VRM_CURRENT_R))
                devData.amdMsr->vrmCurrent = clientData.amdMsr->vrmCurrent;
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_VRM_SOC_CURRENT_W) && clientData.amdMsr->vrmSocCurrent) {
            if (!set(ADJ_OPT_VRMSOC_CURRENT, *clientData.amdMsr->vrmSocCurrent))
                errors.push_back(LIBPWT::Error::W_RY_VRM_SOC_CURRENT);
            else if (!features.contains(LIBPWT::Feature::AMD_RY_VRM_SOC_CURRENT_R))
                devData.amdMsr->vrmSocCurrent = clientData.amdMsr->vrmSocCurrent;
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_VRM_MAX_CURRENT_W) && clientData.amdMsr->vrmMaxCurrent) {
            if (!set(ADJ_OPT_VRMMAX_CURRENT, *clientData.amdMsr->vrmMaxCurrent))
                errors.push_back(LIBPWT::Error::W_RY_VRM_MAX_CURRENT);
            else if (!features.contains(LIBPWT::Feature::AMD_RY_VRM_MAX_CURRENT_R))
                devData.amdMsr->vrmMaxCurrent = clientData.amdMsr->vrmMaxCurrent;
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_W) && clientData.amdMsr->vrmSocMaxCurrent) {
            if (!set(ADJ_OPT_VRMSOCMAX_CURRENT, *clientData.amdMsr->vrmSocMaxCurrent))
                errors.push_back(LIBPWT::Error::W_RY_VRM_SOC_MAX_CURRENT);
            else if (!features.contains(LIBPWT::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_R))
                devData.amdMsr->vrmSocMaxCurrent = clientData.amdMsr->vrmSocMaxCurrent;
        }

        if constexpr (isWindows()) {
            if (features.contains(LIBPWT::Feature::AMD_RY_STATIC_GFX_CLK_W) && clientData.amdMsr->staticGfxClock) {
                if (!set(ADJ_OPT_GFX_CLK, *clientData.amdMsr->staticGfxClock))
                    errors.push_back(LIBPWT::Error::W_RY_STATIC_GFX_CLOCK);
                else
                    devData.amdMsr->staticGfxClock = clientData.amdMsr->staticGfxClock;
            }

            if (features.contains(LIBPWT::Feature::AMD_RY_MIN_GFX_CLOCK_W) && clientData.amdMsr->minGfxClock) {
                if (!set(ADJ_OPT_MIN_GFXCLK_FREQ, *clientData.amdMsr->minGfxClock))
                    errors.push_back(LIBPWT::Error::W_RY_MIN_GFX_CLOCK);
                else if (!features.contains(LIBPWT::Feature::AMD_RY_MIN_GFX_CLOCK_R))
                    devData.amdMsr->minGfxClock = clientData.amdMsr->minGfxClock;
            }

            if (features.contains(LIBPWT::Feature::AMD_RY_MAX_GFX_CLOCK_W) && clientData.amdMsr->maxGfxClock) {
                if (!set(ADJ_OPT_MAX_GFXCLK_FREQ, *clientData.amdMsr->maxGfxClock))
                    errors.push_back(LIBPWT::Error::W_RY_MAX_GFX_CLOCK);
                else if (!features.contains(LIBPWT::Feature::AMD_RY_MAX_GFX_CLOCK_R))
                    devData.amdMsr->maxGfxClock = clientData.amdMsr->maxGfxClock;
            }
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_POWER_PROFILE_W) && clientData.amdMsr->powerProfile) {
            const int profile = *clientData.amdMsr->powerProfile;

            if (!set(profile == 0 ? ADJ_OPT_CCLK_SETPOINT:ADJ_OPT_CCLK_BUSY, profile))
                errors.push_back(LIBPWT::Error::W_RY_POWER_PROFILE);
            else
                devData.amdMsr->powerProfile = clientData.amdMsr->powerProfile;
        }

        if (features.contains(LIBPWT::Feature::AMD_RY_CO_ALL_W) && clientData.amdMsr->curveOptimizer) {
            if (!setCurveOptimizerAll(*clientData.amdMsr->curveOptimizer))
                errors.push_back(LIBPWT::Error::W_RY_CO_ALL);
            else
                devData.amdMsr->curveOptimizer = clientData.amdMsr->curveOptimizer;
        }

        for (const int core: devData.systemInfo->coreIDs)
            writeCore(core, devData, clientData, errors);

        return errors;
    }

    bool RyzenAdj::setCurveOptimizerAll(int value) const {
        if (value == 0)
            value = curveOptimizerBase;
        else if (value < 0)
            value = curveOptimizerBase - static_cast<uint32_t>(-1 * value);

        return set(ADJ_OPT_COALL, value);
    }

    std::optional<int> RyzenAdj::getTemperature() const {
        if (!refreshTable())
            return {};

        return getValue(ADJ_OPT_TCTL_TEMP, 1);
    }
}
