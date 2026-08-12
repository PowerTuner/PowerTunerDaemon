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

    RyzenAdj::~RyzenAdj() {
        ryzenadj_cleanup();
    }

    bool RyzenAdj::init(const int numCores) {
        const ADJ_ERROR res = ryzenadj_init();

        if (res != ADJ_OK) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("failed to init: %1").arg(res));

            return false;
        }

        if (ryzenadj_refresh_table() != ADJ_OK) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(u"RyzenAdj: failed to init table"_s);

            return false;
        }

        cpuCoreCount = numCores;

        buildTableCache();
        return true;
    }

    void RyzenAdj::buildTableCache() const {
        if (ryzenadj_can_read(ADJ_OPT_TCTL_TEMP) == ADJ_OPT_NOT_SUPPORTED)
            tableCache.insert({ADJ_OPT_TCTL_TEMP, -1});

        if (ryzenadj_can_read(ADJ_OPT_APU_SKIN_TEMP_LIMIT) == ADJ_OPT_NOT_SUPPORTED)
            tableCache.insert({ADJ_OPT_APU_SKIN_TEMP_LIMIT, -1});

        if (ryzenadj_can_read(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT) == ADJ_OPT_NOT_SUPPORTED)
            tableCache.insert({ADJ_OPT_DGPU_SKIN_TEMP_LIMIT, -1});

        if (ryzenadj_can_read(ADJ_OPT_VRM_CURRENT) == ADJ_OPT_NOT_SUPPORTED)
            tableCache.insert({ADJ_OPT_VRM_CURRENT, -1});

        if (ryzenadj_can_read(ADJ_OPT_VRMSOC_CURRENT) == ADJ_OPT_NOT_SUPPORTED)
            tableCache.insert({ADJ_OPT_VRMSOC_CURRENT, -1});

        if (ryzenadj_can_read(ADJ_OPT_VRMMAX_CURRENT) == ADJ_OPT_NOT_SUPPORTED)
            tableCache.insert({ADJ_OPT_VRMMAX_CURRENT, -1});

        if (ryzenadj_can_read(ADJ_OPT_VRMSOCMAX_CURRENT) == ADJ_OPT_NOT_SUPPORTED)
            tableCache.insert({ADJ_OPT_VRMSOCMAX_CURRENT, -1});

        if constexpr (isWindows()) {
            if (ryzenadj_can_read(ADJ_OPT_GFX_CLK) == ADJ_OPT_NOT_SUPPORTED)
                tableCache.insert({ADJ_OPT_GFX_CLK, -1});

            if (ryzenadj_can_read(ADJ_OPT_MIN_GFXCLK_FREQ) == ADJ_OPT_NOT_SUPPORTED)
                tableCache.insert({ADJ_OPT_MIN_GFXCLK_FREQ, -1});

            if (ryzenadj_can_read(ADJ_OPT_MAX_GFXCLK_FREQ) == ADJ_OPT_NOT_SUPPORTED)
                tableCache.insert({ADJ_OPT_MAX_GFXCLK_FREQ, -1});
        }

        if (ryzenadj_can_write(ADJ_OPT_CCLK_SETPOINT) == ADJ_OK && ryzenadj_can_write(ADJ_OPT_CCLK_BUSY) == ADJ_OK)
            tableCache.insert({ADJ_OPT_CCLK_SETPOINT, -1});

        if (ryzenadj_can_write(ADJ_OPT_COPER) == ADJ_OK)
            tableCache.insert({ADJ_OPT_COPER, std::vector<int>(cpuCoreCount, 0)});
        else if (ryzenadj_can_write(ADJ_OPT_COALL) == ADJ_OK)
            tableCache.insert({ADJ_OPT_COALL, 0});
    }

    bool RyzenAdj::set(const ADJ_OPT opt, const uint32_t value) const {
        const ADJ_ERROR err = ryzenadj_set(opt, value);

        if (err != ADJ_OK) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("failed to set option %1: code %2").arg(opt).arg(err));

            return false;
        }

        return true;
    }

    bool RyzenAdj::set(const ADJ_OPT opt, const PWTS::RWData<int> &data) const {
        if (!data.isValid() || data.isIgnored())
            return true;

        if (tableCache.contains(opt))
            tableCache[opt] = data.getValue();

        return set(opt, data.getValue());
    }

    bool RyzenAdj::set(const ADJ_OPT opt, const PWTS::RWData<uint32_t> &data) const {
        if (!data.isValid() || data.isIgnored())
            return true;

        if (tableCache.contains(opt))
            tableCache[opt] = data.getValue();

        return set(opt, data.getValue());
    }

    PWTS::RWData<int> RyzenAdj::get(const ADJ_OPT opt, const float valueMult) const {
        ADJ_ERROR err;
        const float value = ryzenadj_get(opt, &err);

        if (err != ADJ_OK) {
            if (err == ADJ_OPT_NOT_SUPPORTED && tableCache.contains(opt)) {
                const int val = std::get<int>(tableCache.at(opt));

                if (logger.isLevel(PWTS::LogLevel::Warning))
                    logger.write(QString("read cmd not implemented for option %1, using internal cache").arg(opt));

                return PWTS::RWData<int>(val, true);
            }

            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("failed to get option %1: code %2").arg(opt).arg(err));

            return PWTS::RWData<int>(0, false);
        }

        return PWTS::RWData<int>(std::isnan(value) ? 0 : static_cast<int>(value * valueMult), !std::isnan(value));
    }

    PWTS::ROData<int> RyzenAdj::read(const ADJ_OPT opt, const float valueMult) const {
        ADJ_ERROR err;
        const float value = ryzenadj_get_value(opt, &err);

        if (err != ADJ_OK) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("failed to read value %1: code %2").arg(opt).arg(err));

            return {};
        }

        return PWTS::ROData<int>(std::isnan(value) ? -1 : static_cast<int>(value * valueMult), !std::isnan(value));
    }

    bool RyzenAdj::refreshTable() const {
        using namespace std::chrono_literals;

        const ADJ_ERROR ret = ryzenadj_refresh_table();

        if (ret != ADJ_OK) {
            if (logger.isLevel(PWTS::LogLevel::Error))
                logger.write(QString("failed to refresh table: code %1").arg(ret));

            return false;
        }

        // refresh table is a cmd, give it some time to complete so we don't read old values
        std::this_thread::sleep_for(7ms);
        return true;
    }

    QSet<PWTS::Feature> RyzenAdj::getFeatures() {
        QSet<PWTS::Feature> features;

        if (ryzenadj_can_write(ADJ_OPT_STAPM_LIMIT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_STAPM_LIMIT_W, PWTS::Feature::AMD_RY_STAPM_LIMIT_R});

        if (ryzenadj_can_write(ADJ_OPT_FAST_LIMIT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_FAST_LIMIT_W, PWTS::Feature::AMD_RY_FAST_LIMIT_R});

        if (ryzenadj_can_write(ADJ_OPT_SLOW_LIMIT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_SLOW_LIMIT_W, PWTS::Feature::AMD_RY_SLOW_LIMIT_R});

        if (ryzenadj_can_write(ADJ_OPT_TCTL_TEMP) == ADJ_OK) {
            features.insert(PWTS::Feature::AMD_RY_TCTL_TEMP_W);

            if (ryzenadj_can_read(ADJ_OPT_TCTL_TEMP) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_TCTL_TEMP_R);
        }

        if (ryzenadj_can_read_value(ADJ_OPT_TCTL_TEMP) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_TCTL_TEMP_VAL, PWTS::Feature::AMD_CPU_RY_STAT_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_SLOW_LIMIT) == ADJ_OK) {
            features.insert(PWTS::Feature::AMD_RY_APU_SLOW_W);

            if (ryzenadj_can_read(ADJ_OPT_SLOW_LIMIT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_APU_SLOW_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_APU_SKIN_TEMP_LIMIT) == ADJ_OK) {
            features.insert(PWTS::Feature::AMD_RY_APU_SKIN_TEMP_W);

            if (ryzenadj_can_read(ADJ_OPT_APU_SKIN_TEMP_LIMIT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_APU_SKIN_TEMP_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT) == ADJ_OK) {
            features.insert(PWTS::Feature::AMD_RY_DGPU_SKIN_TEMP_W);

            if (ryzenadj_can_read(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_DGPU_SKIN_TEMP_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_VRM_CURRENT) == ADJ_OK) {
            features.insert(PWTS::Feature::AMD_RY_VRM_CURRENT_W);

            if (ryzenadj_can_read(ADJ_OPT_VRM_CURRENT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_VRM_CURRENT_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_VRMSOC_CURRENT) == ADJ_OK) {
            features.insert(PWTS::Feature::AMD_RY_VRM_SOC_CURRENT_W);

            if (ryzenadj_can_read(ADJ_OPT_VRMSOC_CURRENT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_VRM_SOC_CURRENT_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_VRMMAX_CURRENT) == ADJ_OK) {
            features.insert(PWTS::Feature::AMD_RY_VRM_MAX_CURRENT_W);

            if (ryzenadj_can_read(ADJ_OPT_VRMMAX_CURRENT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_VRM_MAX_CURRENT_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_VRMSOCMAX_CURRENT) == ADJ_OK) {
            features.insert(PWTS::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_W);

            if (ryzenadj_can_read(ADJ_OPT_VRMSOCMAX_CURRENT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_R);
        }

        if constexpr (isWindows()) {
            if (ryzenadj_can_write(ADJ_OPT_GFX_CLK) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_STATIC_GFX_CLK_W);

            if (ryzenadj_can_write(ADJ_OPT_MIN_GFXCLK_FREQ) == ADJ_OK) {
                features.insert(PWTS::Feature::AMD_RY_MIN_GFX_CLOCK_W);

                if (ryzenadj_can_read(ADJ_OPT_MIN_GFXCLK_FREQ) == ADJ_OK)
                    features.insert(PWTS::Feature::AMD_RY_MIN_GFX_CLOCK_R);
            }

            if (ryzenadj_can_write(ADJ_OPT_MAX_GFXCLK_FREQ) == ADJ_OK) {
                features.insert(PWTS::Feature::AMD_RY_MAX_GFX_CLOCK_W);

                if (ryzenadj_can_read(ADJ_OPT_MAX_GFXCLK_FREQ) == ADJ_OK)
                    features.insert(PWTS::Feature::AMD_RY_MAX_GFX_CLOCK_R);
            }
        }

        if (ryzenadj_can_write(ADJ_OPT_CCLK_SETPOINT) == ADJ_OK && ryzenadj_can_write(ADJ_OPT_CCLK_BUSY) == ADJ_OK)
            features.insert(PWTS::Feature::AMD_RY_POWER_PROFILE_W);

        if (ryzenadj_can_write(ADJ_OPT_COPER) == ADJ_OK)
            features.insert(PWTS::Feature::AMD_RY_CO_PER_W);
        else if (ryzenadj_can_write(ADJ_OPT_COALL) == ADJ_OK)
            features.insert(PWTS::Feature::AMD_RY_CO_ALL_W);

        if (!features.isEmpty())
            features.insert(PWTS::Feature::AMD_CPU_RY_GROUP);

        return features;
    }

    void RyzenAdj::fillPackageData(const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::AMD_CPU_RY_GROUP))
            return;

        if (features.contains(PWTS::Feature::AMD_RY_STAPM_LIMIT_W))
            packet.amdData->stapmLimit = get(ADJ_OPT_STAPM_LIMIT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_FAST_LIMIT_W))
            packet.amdData->fastLimit = get(ADJ_OPT_FAST_LIMIT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_SLOW_LIMIT_W))
            packet.amdData->slowLimit = get(ADJ_OPT_SLOW_LIMIT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_TCTL_TEMP_W))
            packet.amdData->tctlTemp = get(ADJ_OPT_TCTL_TEMP, 1);

        if (features.contains(PWTS::Feature::AMD_RY_APU_SLOW_W))
            packet.amdData->apuSlow = get(ADJ_OPT_APU_SLOW_LIMIT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_APU_SKIN_TEMP_W))
            packet.amdData->apuSkinTemp = get(ADJ_OPT_APU_SKIN_TEMP_LIMIT, 1);

        if (features.contains(PWTS::Feature::AMD_RY_DGPU_SKIN_TEMP_W))
            packet.amdData->dgpuSkinTemp = get(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT, 1);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_CURRENT_W))
            packet.amdData->vrmCurrent = get(ADJ_OPT_VRM_CURRENT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_SOC_CURRENT_W))
            packet.amdData->vrmSocCurrent = get(ADJ_OPT_VRMSOC_CURRENT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_MAX_CURRENT_W))
            packet.amdData->vrmMaxCurrent = get(ADJ_OPT_VRMMAX_CURRENT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_W))
            packet.amdData->vrmSocMaxCurrent = get(ADJ_OPT_VRMSOCMAX_CURRENT, 1000);

        if constexpr (isWindows()) {
            if (features.contains(PWTS::Feature::AMD_RY_STATIC_GFX_CLK_W))
                packet.amdData->staticGfxClock = get(ADJ_OPT_GFX_CLK, 1);

            if (features.contains(PWTS::Feature::AMD_RY_MIN_GFX_CLOCK_W))
                packet.amdData->minGfxClock = get(ADJ_OPT_MIN_GFXCLK_FREQ, 1);

            if (features.contains(PWTS::Feature::AMD_RY_MAX_GFX_CLOCK_W))
                packet.amdData->maxGfxClock = get(ADJ_OPT_MAX_GFXCLK_FREQ, 1);
        }

        if (features.contains(PWTS::Feature::AMD_RY_POWER_PROFILE_W))
            packet.amdData->powerProfile = PWTS::RWData<int>(std::get<int>(tableCache.at(ADJ_OPT_CCLK_SETPOINT)), true);

        if (features.contains(PWTS::Feature::AMD_RY_CO_ALL_W))
            packet.amdData->curveOptimizer = get(ADJ_OPT_COALL, 1);
    }

    void RyzenAdj::fillCoreData(const int cpu, const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::AMD_CPU_RY_GROUP))
            return;

        PWTS::AMD::AMDCoreData &coreData = packet.amdData->coreData[cpu];

        if (features.contains(PWTS::Feature::AMD_RY_CO_PER_W))
            coreData.curveOptimizer = PWTS::RWData<int>(std::get<std::vector<int>>(tableCache.at(ADJ_OPT_COPER))[cpu], true);
    }

    void RyzenAdj::fillPacketData(const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const {
        if (!refreshTable())
            packet.errors.insert(PWTS::DError::RY_REFRESH_TABLE);

        fillPackageData(features, packet);

        for (int i=0; i<cpuCoreCount; ++i)
            fillCoreData(i, features, packet);
    }

    void RyzenAdj::applyPackageSettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        const QSharedPointer<PWTS::AMD::AMDData> data = packet.amdData;

        if (features.contains(PWTS::Feature::AMD_RY_APU_SLOW_W) && !set(ADJ_OPT_APU_SLOW_LIMIT, data->apuSlow))
            errors.insert(PWTS::DError::W_RY_APU_SLOW);

        if (features.contains(PWTS::Feature::AMD_RY_STAPM_LIMIT_W) && !set(ADJ_OPT_STAPM_LIMIT, data->stapmLimit))
            errors.insert(PWTS::DError::W_RY_STAPM_LIMIT);

        if (features.contains(PWTS::Feature::AMD_RY_SLOW_LIMIT_W) && !set(ADJ_OPT_SLOW_LIMIT, data->slowLimit))
            errors.insert(PWTS::DError::W_RY_SLOW_LIMIT);

        if (features.contains(PWTS::Feature::AMD_RY_FAST_LIMIT_W) && !set(ADJ_OPT_FAST_LIMIT, data->fastLimit))
            errors.insert(PWTS::DError::W_RY_FAST_LIMIT);

        if (features.contains(PWTS::Feature::AMD_RY_TCTL_TEMP_W) && !set(ADJ_OPT_TCTL_TEMP, data->tctlTemp))
            errors.insert(PWTS::DError::W_RY_TCTL_TEMP);

        if (features.contains(PWTS::Feature::AMD_RY_APU_SKIN_TEMP_W) && !set(ADJ_OPT_APU_SKIN_TEMP_LIMIT, data->apuSkinTemp))
            errors.insert(PWTS::DError::W_RY_APU_SKIN_TEMP);

        if (features.contains(PWTS::Feature::AMD_RY_DGPU_SKIN_TEMP_W) && !set(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT, data->dgpuSkinTemp))
            errors.insert(PWTS::DError::W_RY_DGPU_SKIN_TEMP);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_CURRENT_W) && !set(ADJ_OPT_VRM_CURRENT, data->vrmCurrent))
            errors.insert(PWTS::DError::W_RY_VRM_CURRENT);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_SOC_CURRENT_W) && !set(ADJ_OPT_VRMSOC_CURRENT, data->vrmSocCurrent))
            errors.insert(PWTS::DError::W_RY_VRM_SOC_CURRENT);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_MAX_CURRENT_W) && !set(ADJ_OPT_VRMMAX_CURRENT, data->vrmMaxCurrent))
            errors.insert(PWTS::DError::W_RY_VRM_MAX_CURRENT);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_W) && !set(ADJ_OPT_VRMSOCMAX_CURRENT, data->vrmSocMaxCurrent))
            errors.insert(PWTS::DError::W_RY_VRM_SOC_MAX_CURRENT);

        if constexpr (isWindows()) {
            if (features.contains(PWTS::Feature::AMD_RY_STATIC_GFX_CLK_W) && !set(ADJ_OPT_GFX_CLK, data->staticGfxClock))
                errors.insert(PWTS::DError::W_RY_STATIC_GFX_CLOCK);

            if (features.contains(PWTS::Feature::AMD_RY_MIN_GFX_CLOCK_W) && !set(ADJ_OPT_MIN_GFXCLK_FREQ, data->minGfxClock))
                errors.insert(PWTS::DError::W_RY_MIN_GFX_CLOCK);

            if (features.contains(PWTS::Feature::AMD_RY_MAX_GFX_CLOCK_W) && !set(ADJ_OPT_MAX_GFXCLK_FREQ, data->maxGfxClock))
                errors.insert(PWTS::DError::W_RY_MAX_GFX_CLOCK);
        }

        if (features.contains(PWTS::Feature::AMD_RY_POWER_PROFILE_W) && !setPowerProfile(data->powerProfile))
            errors.insert(PWTS::DError::W_RY_POWER_PROFILE);

        if (features.contains(PWTS::Feature::AMD_RY_CO_ALL_W) && !setCurveOptimizerAll(data->curveOptimizer))
            errors.insert(PWTS::DError::W_RY_CO_ALL);
    }

    void RyzenAdj::applyCoreSettings(const int cpu, const int coreIdx, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        const PWTS::AMD::AMDCoreData &data = packet.amdData->coreData[cpu];

        if (features.contains(PWTS::Feature::AMD_RY_CO_PER_W) && !setCurveOptimizerCore(cpu, data.curveOptimizer))
            errors.insert(PWTS::DError::W_RY_CO_PER);
    }

    QSet<PWTS::DError> RyzenAdj::applySettings(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) const {
        if (!features.contains(PWTS::Feature::AMD_CPU_RY_GROUP))
            return {};

        QSet<PWTS::DError> errors;

        applyPackageSettings(features, packet, errors);

        for (int i=0; i<cpuCoreCount; ++i)
            applyCoreSettings(i, coreIdxList[i], features, packet, errors);

        return errors;
    }

    bool RyzenAdj::setPowerProfile(const PWTS::RWData<int> &data) const {
        if (!data.isValid() || data.isIgnored())
            return true;

        const ADJ_OPT opt = data.getValue() == 0 ? ADJ_OPT_CCLK_SETPOINT : ADJ_OPT_CCLK_BUSY;

        tableCache[ADJ_OPT_CCLK_SETPOINT] = data.getValue();

        return set(opt, data.getValue());
    }

    bool RyzenAdj::setCurveOptimizerAll(const PWTS::RWData<int> &data) const {
        if (!data.isValid() || data.isIgnored())
            return true;

        const int offt = data.getValue();
        uint32_t co = offt;

        if (offt == 0)
            co = curveOptimizerBase;
        else if (offt < 0)
            co = curveOptimizerBase - static_cast<uint32_t>(-1 * offt);

        tableCache[ADJ_OPT_COALL] = offt;

        return set(ADJ_OPT_COALL, co);
    }

    bool RyzenAdj::setCurveOptimizerCore(const int cpu, const PWTS::RWData<int> &data) const {
        if (!data.isValid() || data.isIgnored())
            return true;

        std::vector<int> &coList = std::get<std::vector<int>>(tableCache.at(ADJ_OPT_COPER));
        const int offt = data.getValue();
        const uint32_t co = (cpu << 20) | (offt & 0xffff);

        coList[cpu] = offt;

        return set(ADJ_OPT_COPER, co);
    }

    PWTS::ROData<int> RyzenAdj::getTemperature() const {
        if (!refreshTable())
            return {};

        return read(ADJ_OPT_TCTL_TEMP, 1);
    }
}
