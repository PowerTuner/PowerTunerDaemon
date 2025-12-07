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

#include "Include/FanControls.h"
#include "Include/FanType.h"
#include "../OS/OS.h"

namespace PWTD {
    class FANDevice {
    private:
        QSharedPointer<FileLogger> logger;
        QSharedPointer<OS> os;
        QList<std::pair<int, int>> curve;
        QString id;

    protected:
        QString fanString;
        FanControls control;

    public:
        FANDevice(const QSharedPointer<OS> &os, const QString &id);
        virtual ~FANDevice();

        [[nodiscard]] virtual QSet<PWTS::Feature> getFeatures(const PWTS::Features &deviceFeatures) const = 0;
        [[nodiscard]] virtual FanType getFanType() const = 0;

        [[nodiscard]] QString getFanString() const { return fanString; }
        [[nodiscard]] QString getID() const { return id; }
        [[nodiscard]] FanControls getControls() const { return control; }
        [[nodiscard]] bool hasFanCurve() const { return curve.size() > 1; }

        [[nodiscard]] PWTS::FanData getFanData() const;
        void prepareForSleep() const;
        [[nodiscard]] QSet<PWTS::DError> applySettings(const PWTS::FanData &data);
        [[nodiscard]] bool applyCurve(const PWTS::ROData<int> &devTemp) const;
    };
}
