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

        fillRyTableCache();
        return true;
    }

    void RyzenAdj::fillRyTableCache() const {
        if (ryzenadj_can_read(ADJ_OPT_TCTL_TEMP) == ADJ_OPT_NOT_SUPPORTED)
            ryTable.insert(ADJ_OPT_TCTL_TEMP, -1);

        if (ryzenadj_can_read(ADJ_OPT_APU_SKIN_TEMP_LIMIT) == ADJ_OPT_NOT_SUPPORTED)
            ryTable.insert(ADJ_OPT_APU_SKIN_TEMP_LIMIT, -1);

        if (ryzenadj_can_read(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT) == ADJ_OPT_NOT_SUPPORTED)
            ryTable.insert(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT, -1);

        if (ryzenadj_can_read(ADJ_OPT_VRM_CURRENT) == ADJ_OPT_NOT_SUPPORTED)
            ryTable.insert(ADJ_OPT_VRM_CURRENT, -1);

        if (ryzenadj_can_read(ADJ_OPT_VRMSOC_CURRENT) == ADJ_OPT_NOT_SUPPORTED)
            ryTable.insert(ADJ_OPT_VRMSOC_CURRENT, -1);

        if (ryzenadj_can_read(ADJ_OPT_VRMMAX_CURRENT) == ADJ_OPT_NOT_SUPPORTED)
            ryTable.insert(ADJ_OPT_VRMMAX_CURRENT, -1);

        if (ryzenadj_can_read(ADJ_OPT_VRMSOCMAX_CURRENT) == ADJ_OPT_NOT_SUPPORTED)
            ryTable.insert(ADJ_OPT_VRMSOCMAX_CURRENT, -1);

        if (isWindows() && ryzenadj_can_read(ADJ_OPT_GFX_CLK) == ADJ_OPT_NOT_SUPPORTED)
            ryTable.insert(ADJ_OPT_GFX_CLK, -1);

        if (isWindows() && ryzenadj_can_read(ADJ_OPT_MIN_GFXCLK_FREQ) == ADJ_OPT_NOT_SUPPORTED)
            ryTable.insert(ADJ_OPT_MIN_GFXCLK_FREQ, -1);

        if (isWindows() && ryzenadj_can_read(ADJ_OPT_MAX_GFXCLK_FREQ) == ADJ_OPT_NOT_SUPPORTED)
            ryTable.insert(ADJ_OPT_MAX_GFXCLK_FREQ, -1);

        if (ryzenadj_can_write(ADJ_OPT_CCLK_SETPOINT) == ADJ_OK && ryzenadj_can_write(ADJ_OPT_CCLK_BUSY) == ADJ_OK)
            ryTable.insert(ADJ_OPT_CCLK_SETPOINT, -1);

        if (ryzenadj_can_write(ADJ_OPT_COPER) == ADJ_OK)
            ryTable.insert(ADJ_OPT_COPER, QList<QVariant>(cpuCoreCount, 0));
        else if (ryzenadj_can_write(ADJ_OPT_COALL) == ADJ_OK)
            ryTable.insert(ADJ_OPT_COALL, 0);
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
        if (!data.isValid() || data.isIgnored())
            return true;

        if (ryTable.contains(opt))
            ryTable[opt] = data.getValue();

        return ryzenAdjSet(opt, data.getValue());
    }

    bool RyzenAdj::ryzenAdjSet(const ADJ_OPT opt, const PWTS::RWData<uint32_t> &data) const {
        if (!data.isValid() || data.isIgnored())
            return true;

        if (ryTable.contains(opt))
            ryTable[opt] = data.getValue();

        return ryzenAdjSet(opt, data.getValue());
    }

    PWTS::RWData<int> RyzenAdj::ryzenAdjGet(const ADJ_OPT opt, const int valueMult) const {
        ADJ_ERROR err;
        const float value = ryzenadj_get(opt, &err);

        if (err != ADJ_OK) {
            if (err == ADJ_OPT_NOT_SUPPORTED && ryTable.contains(opt)) {
                const int val = ryTable[opt].toInt();

                if (logger->isLevel(PWTS::LogLevel::Warning))
                    logger->write(QString("read cmd not implemented for option %1, using internal cache").arg(opt));

                return PWTS::RWData<int>(val, true);
            }

            if (logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("failed to get option %1: code %2").arg(opt).arg(err));

            return PWTS::RWData<int>(0, false);
        }

        return PWTS::RWData<int>(std::isnan(value) ? 0 : static_cast<int>(value * valueMult), !std::isnan(value));
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
            features.unite({PWTS::Feature::AMD_RY_STAPM_LIMIT_W, PWTS::Feature::AMD_RY_STAPM_LIMIT_R, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_FAST_LIMIT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_FAST_LIMIT_W, PWTS::Feature::AMD_RY_FAST_LIMIT_R, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_SLOW_LIMIT) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_SLOW_LIMIT_W, PWTS::Feature::AMD_RY_SLOW_LIMIT_R, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_TCTL_TEMP) == ADJ_OK) {
            features.unite({PWTS::Feature::AMD_RY_TCTL_TEMP_W, PWTS::Feature::AMD_CPU_RY_GROUP});

            if (ryzenadj_can_read(ADJ_OPT_TCTL_TEMP) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_TCTL_TEMP_R);
        }

        if (ryzenadj_can_read_value(ADJ_OPT_TCTL_TEMP) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_TCTL_TEMP_VAL, PWTS::Feature::AMD_CPU_RY_STAT_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_SLOW_LIMIT) == ADJ_OK) {
            features.unite({PWTS::Feature::AMD_RY_APU_SLOW_W, PWTS::Feature::AMD_CPU_RY_GROUP});

            if (ryzenadj_can_read(ADJ_OPT_SLOW_LIMIT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_APU_SLOW_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_APU_SKIN_TEMP_LIMIT) == ADJ_OK) {
            features.unite({PWTS::Feature::AMD_RY_APU_SKIN_TEMP_W, PWTS::Feature::AMD_CPU_RY_GROUP});

            if (ryzenadj_can_read(ADJ_OPT_APU_SKIN_TEMP_LIMIT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_APU_SKIN_TEMP_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT) == ADJ_OK) {
            features.unite({PWTS::Feature::AMD_RY_DGPU_SKIN_TEMP_W, PWTS::Feature::AMD_CPU_RY_GROUP});

            if (ryzenadj_can_read(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_DGPU_SKIN_TEMP_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_VRM_CURRENT) == ADJ_OK) {
            features.unite({PWTS::Feature::AMD_RY_VRM_CURRENT_W, PWTS::Feature::AMD_CPU_RY_GROUP});

            if (ryzenadj_can_read(ADJ_OPT_VRM_CURRENT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_VRM_CURRENT_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_VRMSOC_CURRENT) == ADJ_OK) {
            features.unite({PWTS::Feature::AMD_RY_VRM_SOC_CURRENT_W, PWTS::Feature::AMD_CPU_RY_GROUP});

            if (ryzenadj_can_read(ADJ_OPT_VRMSOC_CURRENT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_VRM_SOC_CURRENT_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_VRMMAX_CURRENT) == ADJ_OK) {
            features.unite({PWTS::Feature::AMD_RY_VRM_MAX_CURRENT_W, PWTS::Feature::AMD_CPU_RY_GROUP});

            if (ryzenadj_can_read(ADJ_OPT_VRMMAX_CURRENT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_VRM_MAX_CURRENT_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_VRMSOCMAX_CURRENT) == ADJ_OK) {
            features.unite({PWTS::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_W, PWTS::Feature::AMD_CPU_RY_GROUP});

            if (ryzenadj_can_read(ADJ_OPT_VRMSOCMAX_CURRENT) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_R);
        }

        if (isWindows() && ryzenadj_can_write(ADJ_OPT_GFX_CLK) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_STATIC_GFX_CLK_W, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (isWindows() && ryzenadj_can_write(ADJ_OPT_MIN_GFXCLK_FREQ) == ADJ_OK) {
            features.unite({PWTS::Feature::AMD_RY_MIN_GFX_CLOCK_W, PWTS::Feature::AMD_CPU_RY_GROUP});

            if (ryzenadj_can_read(ADJ_OPT_MIN_GFXCLK_FREQ) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_MIN_GFX_CLOCK_R);
        }

        if (isWindows() && ryzenadj_can_write(ADJ_OPT_MAX_GFXCLK_FREQ) == ADJ_OK) {
            features.unite({PWTS::Feature::AMD_RY_MAX_GFX_CLOCK_W, PWTS::Feature::AMD_CPU_RY_GROUP});

            if (ryzenadj_can_read(ADJ_OPT_MAX_GFXCLK_FREQ) == ADJ_OK)
                features.insert(PWTS::Feature::AMD_RY_MAX_GFX_CLOCK_R);
        }

        if (ryzenadj_can_write(ADJ_OPT_CCLK_SETPOINT) == ADJ_OK && ryzenadj_can_write(ADJ_OPT_CCLK_BUSY) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_POWER_PROFILE_W, PWTS::Feature::AMD_CPU_RY_GROUP});

        if (ryzenadj_can_write(ADJ_OPT_COPER) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_CO_PER_W, PWTS::Feature::AMD_CPU_RY_GROUP});
        else if (ryzenadj_can_write(ADJ_OPT_COALL) == ADJ_OK)
            features.unite({PWTS::Feature::AMD_RY_CO_ALL_W, PWTS::Feature::AMD_CPU_RY_GROUP});

        return features;
    }

    void RyzenAdj::fillPackageData(const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::AMD_CPU_RY_GROUP))
            return;

        if (features.contains(PWTS::Feature::AMD_RY_STAPM_LIMIT_W))
            packet.amdData->stapmLimit = ryzenAdjGet(ADJ_OPT_STAPM_LIMIT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_FAST_LIMIT_W))
            packet.amdData->fastLimit = ryzenAdjGet(ADJ_OPT_FAST_LIMIT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_SLOW_LIMIT_W))
            packet.amdData->slowLimit = ryzenAdjGet(ADJ_OPT_SLOW_LIMIT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_TCTL_TEMP_W))
            packet.amdData->tctlTemp = ryzenAdjGet(ADJ_OPT_TCTL_TEMP, 1);

        if (features.contains(PWTS::Feature::AMD_RY_APU_SLOW_W))
            packet.amdData->apuSlow = ryzenAdjGet(ADJ_OPT_APU_SLOW_LIMIT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_APU_SKIN_TEMP_W))
            packet.amdData->apuSkinTemp = ryzenAdjGet(ADJ_OPT_APU_SKIN_TEMP_LIMIT, 1);

        if (features.contains(PWTS::Feature::AMD_RY_DGPU_SKIN_TEMP_W))
            packet.amdData->dgpuSkinTemp = ryzenAdjGet(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT, 1);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_CURRENT_W))
            packet.amdData->vrmCurrent = ryzenAdjGet(ADJ_OPT_VRM_CURRENT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_SOC_CURRENT_W))
            packet.amdData->vrmSocCurrent = ryzenAdjGet(ADJ_OPT_VRMSOC_CURRENT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_MAX_CURRENT_W))
            packet.amdData->vrmMaxCurrent = ryzenAdjGet(ADJ_OPT_VRMMAX_CURRENT, 1000);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_W))
            packet.amdData->vrmSocMaxCurrent = ryzenAdjGet(ADJ_OPT_VRMSOCMAX_CURRENT, 1000);

        if (isWindows() && features.contains(PWTS::Feature::AMD_RY_STATIC_GFX_CLK_W))
            packet.amdData->staticGfxClock = ryzenAdjGet(ADJ_OPT_GFX_CLK, 1);

        if (isWindows() && features.contains(PWTS::Feature::AMD_RY_MIN_GFX_CLOCK_W))
            packet.amdData->minGfxClock = ryzenAdjGet(ADJ_OPT_MIN_GFXCLK_FREQ, 1);

        if (isWindows() && features.contains(PWTS::Feature::AMD_RY_MAX_GFX_CLOCK_W))
            packet.amdData->maxGfxClock = ryzenAdjGet(ADJ_OPT_MAX_GFXCLK_FREQ, 1);

        if (features.contains(PWTS::Feature::AMD_RY_POWER_PROFILE_W))
            packet.amdData->powerProfile = PWTS::RWData<int>(ryTable[ADJ_OPT_CCLK_SETPOINT].toInt(), true);

        if (features.contains(PWTS::Feature::AMD_RY_CO_ALL_W))
            packet.amdData->curveOptimizer = ryzenAdjGet(ADJ_OPT_COALL, 1);
    }

    void RyzenAdj::fillCoreData(const int cpu, const QSet<PWTS::Feature> &features, const PWTS::DaemonPacket &packet) const {
        if (!features.contains(PWTS::Feature::AMD_CPU_RY_GROUP))
            return;

        PWTS::AMD::AMDCoreData &coreData = packet.amdData->coreData[cpu];

        if (features.contains(PWTS::Feature::AMD_RY_CO_PER_W))
            coreData.curveOptimizer = PWTS::RWData<int>((ryTable[ADJ_OPT_COPER].toList())[cpu].toInt(), true);
    }

    void RyzenAdj::fillPacketData(const QSet<PWTS::Feature> &features, PWTS::DaemonPacket &packet) const {
        if (!refreshRyzenAdjTable())
            packet.errors.insert(PWTS::DError::RY_REFRESH_TABLE);

        fillPackageData(features, packet);

        for (int i=0; i<cpuCoreCount; ++i)
            fillCoreData(i, features, packet);
    }

    void RyzenAdj::applyPackageSettings(const QSet<PWTS::Feature> &features, const PWTS::ClientPacket &packet, QSet<PWTS::DError> &errors) const {
        const QSharedPointer<PWTS::AMD::AMDData> data = packet.amdData;

        if (features.contains(PWTS::Feature::AMD_RY_APU_SLOW_W) && !ryzenAdjSet(ADJ_OPT_APU_SLOW_LIMIT, data->apuSlow))
            errors.insert(PWTS::DError::W_RY_APU_SLOW);

        if (features.contains(PWTS::Feature::AMD_RY_STAPM_LIMIT_W) && !ryzenAdjSet(ADJ_OPT_STAPM_LIMIT, data->stapmLimit))
            errors.insert(PWTS::DError::W_RY_STAPM_LIMIT);

        if (features.contains(PWTS::Feature::AMD_RY_SLOW_LIMIT_W) && !ryzenAdjSet(ADJ_OPT_SLOW_LIMIT, data->slowLimit))
            errors.insert(PWTS::DError::W_RY_SLOW_LIMIT);

        if (features.contains(PWTS::Feature::AMD_RY_FAST_LIMIT_W) && !ryzenAdjSet(ADJ_OPT_FAST_LIMIT, data->fastLimit))
            errors.insert(PWTS::DError::W_RY_FAST_LIMIT);

        if (features.contains(PWTS::Feature::AMD_RY_TCTL_TEMP_W) && !ryzenAdjSet(ADJ_OPT_TCTL_TEMP, data->tctlTemp))
            errors.insert(PWTS::DError::W_RY_TCTL_TEMP);

        if (features.contains(PWTS::Feature::AMD_RY_APU_SKIN_TEMP_W) && !ryzenAdjSet(ADJ_OPT_APU_SKIN_TEMP_LIMIT, data->apuSkinTemp))
            errors.insert(PWTS::DError::W_RY_APU_SKIN_TEMP);

        if (features.contains(PWTS::Feature::AMD_RY_DGPU_SKIN_TEMP_W) && !ryzenAdjSet(ADJ_OPT_DGPU_SKIN_TEMP_LIMIT, data->dgpuSkinTemp))
            errors.insert(PWTS::DError::W_RY_DGPU_SKIN_TEMP);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_CURRENT_W) && !ryzenAdjSet(ADJ_OPT_VRM_CURRENT, data->vrmCurrent))
            errors.insert(PWTS::DError::W_RY_VRM_CURRENT);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_SOC_CURRENT_W) && !ryzenAdjSet(ADJ_OPT_VRMSOC_CURRENT, data->vrmSocCurrent))
            errors.insert(PWTS::DError::W_RY_VRM_SOC_CURRENT);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_MAX_CURRENT_W) && !ryzenAdjSet(ADJ_OPT_VRMMAX_CURRENT, data->vrmMaxCurrent))
            errors.insert(PWTS::DError::W_RY_VRM_MAX_CURRENT);

        if (features.contains(PWTS::Feature::AMD_RY_VRM_SOC_MAX_CURRENT_W) && !ryzenAdjSet(ADJ_OPT_VRMSOCMAX_CURRENT, data->vrmSocMaxCurrent))
            errors.insert(PWTS::DError::W_RY_VRM_SOC_MAX_CURRENT);

        if (features.contains(PWTS::Feature::AMD_RY_STATIC_GFX_CLK_W) && !ryzenAdjSet(ADJ_OPT_GFX_CLK, data->staticGfxClock))
            errors.insert(PWTS::DError::W_RY_STATIC_GFX_CLOCK);

        if (features.contains(PWTS::Feature::AMD_RY_MIN_GFX_CLOCK_W) && !ryzenAdjSet(ADJ_OPT_MIN_GFXCLK_FREQ, data->minGfxClock))
            errors.insert(PWTS::DError::W_RY_MIN_GFX_CLOCK);

        if (features.contains(PWTS::Feature::AMD_RY_MAX_GFX_CLOCK_W) && !ryzenAdjSet(ADJ_OPT_MAX_GFXCLK_FREQ, data->maxGfxClock))
            errors.insert(PWTS::DError::W_RY_MAX_GFX_CLOCK);

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

        ryTable[ADJ_OPT_CCLK_SETPOINT] = data.getValue();

        return ryzenAdjSet(opt, data.getValue());
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

        ryTable[ADJ_OPT_COALL] = offt;

        return ryzenAdjSet(ADJ_OPT_COALL, co);
    }

    bool RyzenAdj::setCurveOptimizerCore(const int cpu, const PWTS::RWData<int> &data) const {
        if (!data.isValid() || data.isIgnored())
            return true;

        QList<QVariant> coList = ryTable[ADJ_OPT_COPER].toList();
        const int offt = data.getValue();
        const uint32_t co = (cpu << 20) | (offt & 0xffff);

        coList[cpu] = offt;
        ryTable[ADJ_OPT_COPER] = coList;

        return ryzenAdjSet(ADJ_OPT_COPER, co);
    }

    PWTS::ROData<int> RyzenAdj::getTemperature() const {
        if (!refreshRyzenAdjTable())
            return {};

        return ryzenAdjRead(ADJ_OPT_TCTL_TEMP, 1);
    }
}
