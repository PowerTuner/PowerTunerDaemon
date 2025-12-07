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
#include "FANDevice.h"

namespace PWTD {
    FANDevice::FANDevice(const QSharedPointer<OS> &os, const QString &id) {
        logger = FileLogger::getInstance();
        this->os = os;
        this->id = id;
    }

    FANDevice::~FANDevice() {
        if (os->setupOSAccess()) {
            if (!os->setFanMode(control, PWTS::RWData<int>(0, true)) && logger->isLevel(PWTS::LogLevel::Error))
                logger->write(QString("%1: failed to reset to auto").arg(fanString));

            os->unsetOSAccess();

        } else if (logger->isLevel(PWTS::LogLevel::Error)) {
            logger->write("failed to setup os access");
        }
    }

    PWTS::FanData FANDevice::getFanData() const {
        return {
            .mode = os->getFanMode(control),
            .curve = curve
        };
    }

    void FANDevice::prepareForSleep() const {
        if (!os->setFanMode(control, PWTS::RWData<int>(0, true)) && logger->isLevel(PWTS::LogLevel::Error))
            logger->write(QString("%1: failed to reset to auto").arg(fanString));
    }

    QSet<PWTS::DError> FANDevice::applySettings(const PWTS::FanData &data) {
        QSet<PWTS::DError> errors;

        curve = data.curve;

        std::sort(curve.begin(), curve.end(), [](const std::pair<int, int> &a, const std::pair<int, int> &b)->bool {
            return a.first < b.first;
        });

        if (!os->setFanMode(control, data.mode))
            errors.insert(PWTS::DError::W_FAN_MODE);

        if (curve.size() == 1 && !os->setFanSpeed(control, curve[0].second))
            errors.insert(PWTS::DError::W_FAN_SPEED);

        return errors;
    }

    bool FANDevice::applyCurve(const PWTS::ROData<int> &devTemp) const {
        if (!devTemp.isValid() || curve.isEmpty())
            return false;

        const int dtemp = devTemp.getValue();

        for (int i=(curve.size()-1); i>=0; --i) {
            if (dtemp > curve[i].first)
                return os->setFanSpeed(control, curve[i].second);
        }

        return os->setFanSpeed(control, curve[0].second);
    }
}
