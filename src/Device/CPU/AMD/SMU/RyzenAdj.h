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
#include "libryzenadj/ryzenadj.h"
#include "../../Utils/FileLogger/FileLogger.h"
#include "../../../../external/libPowerTuner/src/DeviceData.h"

namespace PWTD::AMD {
    class RyzenAdj final {
    private:
        static constexpr uint32_t curveOptimizerBase = 0x100000;
        FileLogger &logger = FileLogger::get();
        bool initialized = false;

        [[nodiscard]] bool set(ADJ_OPT opt, int value) const;
        [[nodiscard]] std::optional<int> get(ADJ_OPT opt, float valueMult) const;
        [[nodiscard]] std::optional<int> getValue(ADJ_OPT opt, float valueMult) const;
        [[nodiscard]] bool refreshTable() const;
        void writeCore(int cpu, LIBPWT::DeviceData &devData, const LIBPWT::DeviceData &clientData, std::vector<LIBPWT::Error> &errors) const;
        [[nodiscard]] bool setCurveOptimizerAll(int value) const;

    public:
        explicit RyzenAdj(LIBPWT::DeviceData &devData);
        ~RyzenAdj();

        [[nodiscard]] bool isRunning() const { return initialized; }

        void read(LIBPWT::DeviceData &devData) const;
        [[nodiscard]] std::vector<LIBPWT::Error> write(LIBPWT::DeviceData &devData, const LIBPWT::DeviceData &clientData) const;
        [[nodiscard]] std::optional<int> getTemperature() const;
    };
}
