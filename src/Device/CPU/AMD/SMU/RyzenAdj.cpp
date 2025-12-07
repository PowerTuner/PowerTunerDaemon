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
#include <QThread>

#include "RyzenAdj.h"
#include "../../Utils/DaemonUtils.h"

namespace PWTD::AMD {
    RyzenAdj::RyzenAdj() {
        logger = FileLogger::getInstance();
    }

    RyzenAdj::~RyzenAdj() {
        ryzenadj_cleanup();
    }

    bool RyzenAdj::init(const int numCores) {
        const ADJ_ERROR res = ryzenadj_init();

        if (res != ADJ_OK) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to init: %1").arg(res));

            return false;
        }

        if (ryzenadj_refresh_table() != ADJ_OK) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QStringLiteral("RyzenAdj: failed to init table"));

            return false;
        }

        cpuCoreCount = numCores;

        return true;
    }

    bool RyzenAdj::ryzenAdjSet(const ADJ_OPT opt, const uint32_t value) const {
        const ADJ_ERROR err = ryzenadj_set(opt, value);

        if (err != ADJ_OK) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to set option %1: code %2").arg(opt).arg(err));

            return false;
        }

        return true;
    }

    bool RyzenAdj::ryzenAdjSet(const ADJ_OPT opt, const PWTS::RWData<int> &data) const {
        if (!data.isValid())
            return true;

        return ryzenAdjSet(opt, data.getValue());
    }

    bool RyzenAdj::ryzenAdjSet(const ADJ_OPT opt, const PWTS::RWData<uint32_t> &data) const {
        if (!data.isValid())
            return true;

        return ryzenAdjSet(opt, data.getValue());
    }

    PWTS::RWData<int> RyzenAdj::ryzenAdjGet(const ADJ_OPT opt, const int valueMult) const {
        ADJ_ERROR err;
        const float value = ryzenadj_get(opt, &err);

        if (err != ADJ_OK) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to get option %1: code %2").arg(opt).arg(err));

            return PWTS::RWData<int>(-1, err == ADJ_OPT_NOT_SUPPORTED);
        }

        return PWTS::RWData<int>(std::isnan(value) ? -1 : static_cast<int>(value * valueMult), !std::isnan(value));
    }

    PWTS::ROData<int> RyzenAdj::ryzenAdjRead(const ADJ_OPT opt, const int valueMult) const {
        ADJ_ERROR err;
        const float value = ryzenadj_get_value(opt, &err);

        if (err != ADJ_OK) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to read value %1: code %2").arg(opt).arg(err));

            return {};
        }

        return PWTS::ROData<int>(std::isnan(value) ? -1 : static_cast<int>(value * valueMult), !std::isnan(value));
    }

    bool RyzenAdj::refreshRyzenAdjTable() const {
        const ADJ_ERROR ret = ryzenadj_refresh_table();

        if (ret != ADJ_OK) {
            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to refresh table: code %1").arg(ret));

            return false;
        }

        // refresh table is a cmd, give it some time to complete so we don't read old values
        QThread::msleep(7);
        return true;
    }

    QSet<PWTS::Feature> RyzenAdj::getFeatures() {
        QSet<PWTS::Feature> features;

        if (ryzenadj_can_write(ADJ_OPT_STAPM_LIMIT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_STAPM_LIMIT, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_FAST_LIMIT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_FAST_LIMIT, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_SLOW_LIMIT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_SLOW_LIMIT, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_TCTL_TEMP) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_TCTL_TEMP, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_read_value(ADJ_OPT_TCTL_TEMP) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_TCTL_TEMP_VAL, PWTS::Feature::AMD_CPU_RY_STAT_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_SLOW_LIMIT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_APU_SLOW, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_APU_SKIN_TEMP_LIMIT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_APU_SKIN_TEMP, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_DGPU_SKIN_TEMP, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_VRM_CURRENT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_VRM_CURRENT, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_VRMSOC_CURRENT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_VRM_SOC_CURRENT, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_VRMMAX_CURRENT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_VRM_MAX_CURRENT, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_VRMSOCMAX_CURRENT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_VRM_SOC_MAX_CURRENT, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (isWindows() && ryzenadj_can_write(ADJ_OPT_GFX_CLK) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_STATIC_GFX_CLK, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (isWindows() && ryzenadj_can_write(ADJ_OPT_MIN_GFXCLK_FREQ) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_MIN_GFX_CLOCK, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (isWindows() && ryzenadj_can_write(ADJ_OPT_MAX_GFXCLK_FREQ) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_MAX_GFX_CLOCK, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_CCLK_SETPOINT) == ADJ_OK && ryzenadj_can_write(ADJ_OPT_CCLK_BUSY) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_POWER_PROFILE, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_COPER) == ADJ_OK) {
            features.unite({PWTS::Feature::AMD_RY_CO_PER, PWTS::Feature::AMD_CPU_RY_GROUP});
            ryCOCore.fill(0, cpuCoreCount);

        } else if (ryzenadj_can_write(ADJ_OPT_COALL) == ADJ_OK) {
            features.unite({PWTS::Feature::AMD_RY_CO_ALL, PWTS::Feature::AMD_CPU_RY_GROUP});
        }

        return features;
    }

    void RyzenAdj::fillPackageData(const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::AMD_CPU_RY_GROUP))
            return;

        if (features.contains(PWTS::Feature::AMD_RY_STAPM_LIMIT))
            packet.amdData->stapmLimit = ryzenAdjGet(ADJ_OPT_STAPM_LIMIT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_FAST_LIMIT))
            packet.amdData->fastLimit = ryzenAdjGet(ADJ_OPT_FAST_LIMIT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_SLOW_LIMIT))
            packet.amdData->slowLimit = ryzenAdjGet(ADJ_OPT_SLOW_LIMIT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_TCTL_TEMP))
            packet.amdData->tctlTemp = ryzenAdjGet(ADJ_OPT_TCTL_TEMP, 1);

        if (features.contains(PWTS::Feature::AMD_RY_APU_SLOW))
            packet.amdData->apuSlow = ryzenAdjGet(ADJ_OPT_APU_SLOW_LIMIT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_APU_SKIN_TEMP))
            packet.amdData->apuSkinTemp = ryzenAdjGet(ADJ_OPT_APU_SKIN_TEMP_LIMIT, 1);

        if (features.contains(PWTS::Feature::AMD_RY_DGPU_SKIN_TEMP))
            packet.amdData->dgpuSkinTemp = ryzenAdjGet(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT, 1);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_CURRENT))
            packet.amdData->vrmCurrent = ryzenAdjGet(ADJ_OPT_VRM_CURRENT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_SOC_CURRENT))
            packet.amdData->vrmSocCurrent = ryzenAdjGet(ADJ_OPT_VRMSOC_CURRENT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_MAX_CURRENT))
            packet.amdData->vrmMaxCurrent = ryzenAdjGet(ADJ_OPT_VRMMAX_CURRENT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_SOC_MAX_CURRENT))
            packet.amdData->vrmSocMaxCurrent = ryzenAdjGet(ADJ_OPT_VRMSOCMAX_CURRENT, 1000);

        if (isWindows() && features.contains(PWTS::Feature::AMD_RY_STATIC_GFX_CLK))
            packet.amdData->staticGfxClock = PWTS::RWData<int>(ryStaticGfxClock, true);

        if (isWindows() && features.contains(PWTS::Feature::AMD_RY_MIN_GFX_CLOCK))
            packet.amdData->minGfxClock = PWTS::RWData<int>(ryMinGfxClock, true);

        if (isWindows() && features.contains(PWTS::Feature::AMD_RY_MAX_GFX_CLOCK))
            packet.amdData->maxGfxClock = PWTS::RWData<int>(ryMaxGfxClock, true);

        if (features.contains(PWTS::Feature::AMD_RY_POWER_PROFILE))
            packet.amdData->powerProfile = PWTS::RWData<int>(ryPowerProfile, true);

        if (features.contains(PWTS::Feature::AMD_RY_CO_ALL))
            packet.amdData->curveOptimizer = PWTS::RWData<int>(ryCOAll, true);
    }

    void RyzenAdj::fillCoreData(const int cpu, const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::AMD_CPU_RY_GROUP))
            return;

        PWTS::AMD::AMDCoreData &coreData = packet.amdData->coreData[cpu];

        if (features.contains(PWTS::Feature::AMD_RY_CO_PER))
            coreData.curveOptimizer = PWTS::RWData<int>(ryCOCore[cpu], true);
    }

    void RyzenAdj::fillPacketData(const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const {
        if (!refreshRyzenAdjTable())
            packet.errors.insert(PWTS::DError::RY_REFRESH_TABLE);

        fillPackageData(features, packet);

        for (int i=0; i<cpuCoreCount; ++i)
            fillCoreData(i, features, packet);
    }

    void RyzenAdj::applyPackageSettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) {
        const QSharedPointer<PWTS::AMD::AMDData> data = packet.amdData;

        if (features.contains(PWTS::Feature::AMD_RY_APU_SLOW) && !ryzenAdjSet(ADJ_OPT_APU_SLOW_LIMIT, data->apuSlow))
            errors.insert(PWTS::DError::W_RY_APU_SLOW);

        if (features.contains(PWTS::Feature::AMD_RY_STAPM_LIMIT) && !ryzenAdjSet(ADJ_OPT_STAPM_LIMIT, data->stapmLimit))
            errors.insert(PWTS::DError::W_RY_STAPM_LIMIT);

        if (features.contains(PWTS::Feature::AMD_RY_SLOW_LIMIT) && !ryzenAdjSet(ADJ_OPT_SLOW_LIMIT, data->slowLimit))
            errors.insert(PWTS::DError::W_RY_SLOW_LIMIT);

        if (features.contains(PWTS::Feature::AMD_RY_FAST_LIMIT) && !ryzenAdjSet(ADJ_OPT_FAST_LIMIT, data->fastLimit))
            errors.insert(PWTS::DError::W_RY_FAST_LIMIT);

        if (features.contains(PWTS::Feature::AMD_RY_TCTL_TEMP) && !ryzenAdjSet(ADJ_OPT_TCTL_TEMP, data->tctlTemp))
            errors.insert(PWTS::DError::W_RY_TCTL_TEMP);

        if (features.contains(PWTS::Feature::AMD_RY_APU_SKIN_TEMP) && !ryzenAdjSet(ADJ_OPT_APU_SKIN_TEMP_LIMIT, data->apuSkinTemp))
            errors.insert(PWTS::DError::W_RY_APU_SKIN_TEMP);

        if (features.contains(PWTS::Feature::AMD_RY_DGPU_SKIN_TEMP) && !ryzenAdjSet(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT, data->dgpuSkinTemp))
            errors.insert(PWTS::DError::W_RY_DGPU_SKIN_TEMP);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_CURRENT) && !ryzenAdjSet(ADJ_OPT_VRM_CURRENT, data->vrmCurrent))
            errors.insert(PWTS::DError::W_RY_VRM_CURRENT);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_SOC_CURRENT) && !ryzenAdjSet(ADJ_OPT_VRMSOC_CURRENT, data->vrmSocCurrent))
            errors.insert(PWTS::DError::W_RY_VRM_SOC_CURRENT);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_MAX_CURRENT) && !ryzenAdjSet(ADJ_OPT_VRMMAX_CURRENT, data->vrmMaxCurrent))
            errors.insert(PWTS::DError::W_RY_VRM_MAX_CURRENT);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_SOC_MAX_CURRENT) && !ryzenAdjSet(ADJ_OPT_VRMSOCMAX_CURRENT, data->vrmSocMaxCurrent))
            errors.insert(PWTS::DError::W_RY_VRM_SOC_MAX_CURRENT);

        if (features.contains(PWTS::Feature::AMD_RY_STATIC_GFX_CLK) && !setStaticGfxClock(data->staticGfxClock))
            errors.insert(PWTS::DError::W_RY_STATIC_GFX_CLOCK);

        if (features.contains(PWTS::Feature::AMD_RY_MIN_GFX_CLOCK) && !setMinGfxClock(data->minGfxClock))
            errors.insert(PWTS::DError::W_RY_MIN_GFX_CLOCK);

        if (features.contains(PWTS::Feature::AMD_RY_MAX_GFX_CLOCK) && !setMaxGfxClock(data->maxGfxClock))
            errors.insert(PWTS::DError::W_RY_MAX_GFX_CLOCK);

        if (features.contains(PWTS::Feature::AMD_RY_POWER_PROFILE) && !setPowerProfile(data->powerProfile))
            errors.insert(PWTS::DError::W_RY_POWER_PROFILE);

        if (features.contains(PWTS::Feature::AMD_RY_CO_ALL) && !setCurveOptimizerAll(data->curveOptimizer))
            errors.insert(PWTS::DError::W_RY_CO_ALL);
    }

    void RyzenAdj::applyCoreSettings(const int cpu, const int coreIdx, const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) {
        const PWTS::AMD::AMDCoreData &data = packet.amdData->coreData[cpu];

        if (features.contains(PWTS::Feature::AMD_RY_CO_PER) && !setCurveOptimizerCore(coreIdx, data.curveOptimizer))
            errors.insert(PWTS::DError::W_RY_CO_PER);
    }

    QSet<PWTS::DError> RyzenAdj::applySettings(const QSet<PWTS::Feature> &features, const QList<int> &coreIdxList, const PWTS::ClientPacket &packet) {
        if (!features.contains(PWTS::Feature::AMD_CPU_RY_GROUP))
            return {};

        QSet<PWTS::DError> errors;

        applyPackageSettings(features, packet, errors);

        for (int i=0; i<cpuCoreCount; ++i)
            applyCoreSettings(i, coreIdxList[i], features, packet, errors);

        return errors;
    }

    bool RyzenAdj::setStaticGfxClock(const PWTS::RWData<int> &data) {
        if (!data.isValid())
            return true;

        ryStaticGfxClock = data.getValue();

        if (ryStaticGfxClock == -1)
            return true;

        return ryzenAdjSet(ADJ_OPT_GFX_CLK, data);
    }

    bool RyzenAdj::setMinGfxClock(const PWTS::RWData<int> &data) {
        if (!data.isValid())
            return true;

        ryMinGfxClock = data.getValue();

        if (ryMinGfxClock == -1)
            return true;

        return ryzenAdjSet(ADJ_OPT_MIN_GFXCLK_FREQ, data);
    }

    bool RyzenAdj::setMaxGfxClock(const PWTS::RWData<int> &data) {
        if (!data.isValid())
            return true;

        ryMaxGfxClock = data.getValue();

        if (ryMaxGfxClock == -1)
            return true;

        return ryzenAdjSet(ADJ_OPT_MAX_GFXCLK_FREQ, data);
    }

    bool RyzenAdj::setPowerProfile(const PWTS::RWData<int> &data) {
        if (!data.isValid())
            return true;

        const ADJ_OPT opt = data.getValue() == 0 ? ADJ_OPT_CCLK_SETPOINT : ADJ_OPT_CCLK_BUSY;

        ryPowerProfile = data.getValue();

        if (ryPowerProfile == -1)
            return true;

        return ryzenAdjSet(opt, PWTS::RWData<int>(0, true));
    }

    bool RyzenAdj::setCurveOptimizerAll(const PWTS::RWData<int> &data) {
        if (!data.isValid())
            return true;

        const int offt = data.getValue();
        uint32_t co = offt;

        if (offt == 0)
            co = curveOptimizerBase;
        else if (offt < 0)
            co = curveOptimizerBase - static_cast<uint32_t>(-1 * offt);

        ryCOAll = offt;

        return ryzenAdjSet(ADJ_OPT_COALL, PWTS::RWData<uint32_t>(co, true));
    }

    bool RyzenAdj::setCurveOptimizerCore(const int cpu, const PWTS::RWData<int> &data) {
        if (!data.isValid())
            return true;

        const int offt = data.getValue();
        const uint32_t co = (cpu << 20) | (offt & 0xffff);

        ryCOCore[cpu] = offt;

        return ryzenAdjSet(ADJ_OPT_COPER, PWTS::RWData<uint32_t>(co, true));
    }

    PWTS::ROData<int> RyzenAdj::getTemperature() const {
        if (!refreshRyzenAdjTable())
            return {};

        return ryzenAdjRead(ADJ_OPT_TCTL_TEMP, 1);
    }
}
