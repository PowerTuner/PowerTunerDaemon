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
#include <QTimer>

#include "CPU/CPUDevice.h"
#include "GPU/GPUDevice.h"
#include "FAN/FANDevice.h"
#include "../Utils/FileLogger/FileLogger.h"

namespace PWTD {
    class Device final: public QObject {
        Q_OBJECT

    private:
        FileLogger &logger = FileLogger::get();
        std::unique_ptr<CPUDevice> cpu;
        std::shared_ptr<OS> os;
        std::vector<std::shared_ptr<GPUDevice>> gpus;
        std::vector<std::shared_ptr<FANDevice>> fans;
        PWTS::Features deviceFeatures;
        QList<int> coreIdxList;
        mutable QScopedPointer<QTimer> fanCurveTimer;

        Device();

        void setupFanCurveTimer(bool enable) const;

    public:
        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;

        [[nodiscard]] static Device &get();
        [[nodiscard]] bool isCPUSupported() const;
        [[nodiscard]] bool isOSSupported() const;
        [[nodiscard]] QByteArray getDeviceHash() const;
        [[nodiscard]] PWTS::Features getFeatures() const;
        [[nodiscard]] PWTS::CPUVendor getCPUVendor() const;
        [[nodiscard]] QSharedPointer<PWTS::SystemInfo> getSystemInfo() const;
        [[nodiscard]] PWTS::DynamicSystemInfo getDynamicSystemInfo() const;
        [[nodiscard]] QSharedPointer<PWTS::CpuInfo> getCPUInfo() const;
        [[nodiscard]] QMap<int, PWTS::GpuInfo> getGPUInfoMap() const;
        [[nodiscard]] QMap<QString, QString> getFanLabelsMap() const;
        void prepareForSleep() const;
        void fillPacketDeviceData(PWTS::DaemonPacket &packet) const;
        [[nodiscard]] QSet<PWTS::DError> applySettings(const PWTS::ClientPacket &packet) const;

    private slots:
        void onFanCurveTimerTimeout() const;
    };
}
