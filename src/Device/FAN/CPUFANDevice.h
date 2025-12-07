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

#include "FANDevice.h"

namespace PWTD {
    class CPUFANDevice: public FANDevice {
    public:
        CPUFANDevice(const QSharedPointer<OS> &os, const QString &id): FANDevice(os, id) {}

        [[nodiscard]] FanType getFanType() const final { return FanType::CPU; }

        [[nodiscard]]
        QSet<PWTS::Feature> getFeatures(const PWTS::Features &deviceFeatures) const override {
            QSet<PWTS::Feature> features;
            const bool canReadTemp = deviceFeatures.cpu.contains(PWTS::Feature::AMD_RY_TCTL_TEMP_VAL) ||
                                    deviceFeatures.cpu.contains({PWTS::Feature::INTEL_PKG_THERM_STATUS, PWTS::Feature::INTEL_TEMPERATURE_TARGET});

            if (canReadTemp)
                features.insert(PWTS::Feature::FAN_CURVE);

            return features;
        }
    };
}
