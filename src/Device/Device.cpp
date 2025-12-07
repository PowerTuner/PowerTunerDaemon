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
#include <QCryptographicHash>

#include "Device.h"
#include "CPU/CPUDeviceFactory.h"
#include "GPU/GPUDeviceFactory.h"
#include "FAN/FANFactory.h"
#include "OS/OSFactory.h"

namespace PWTD {
    Device::Device() {
        QSharedPointer<PWTS::CpuInfo> cpuInfo;

        logger = FileLogger::getInstance();
        os = OSFactory::getOS();

        if (!os->setupOSAccess() && logger->isLevel(PWTS::LogLevel::Error))
            logger->write(QStringLiteral("failed to setup os access, some features may be disabled"));

        cpu = CPUDeviceFactory::getCpuDevice();

        if (cpu.isNull() || os.isNull())
            return;

        os->collectSystemInfo();

        cpuInfo = cpu->getCpuInfo();
        coreIdxList = os->getCPUCoreIndexList();
        deviceFeatures.cpu = cpu->getFeatures();

        deviceFeatures.cpu.unite(os->getCPUFeatures(cpuInfo->numLogicalCpus, cpuInfo->vendor));

        for (const int index: os->getGPUIndexList()) {
            const QSharedPointer<GPUDevice> gpu = GPUDeviceFactory::getGPUDevice(index, os);

            if (gpu.isNull())
                continue;

            gpus.append(gpu);
            deviceFeatures.gpus.insert(index, os->getGPUFeatures(index, gpu->getVendor()));
        }

        if (os->hasFanControls()) {
            fans = FANFactory::getFans(os, cpuInfo->extModel);

            for (const QSharedPointer<FANDevice> &fan: fans)
                deviceFeatures.fans.insert(fan->getID(), fan->getFeatures(deviceFeatures));
        }

        os->unsetOSAccess();
    }

    QSharedPointer<Device> Device::getDevice() {
        if (!instance.isNull())
            return instance;

        instance.reset(new Device);
        return instance;
    }

    bool Device::isCPUSupported() const {
        return !cpu.isNull();
    }

    bool Device::isOSSupported() const {
        return !os.isNull();
    }

    QByteArray Device::getDeviceHash() const {
        QCryptographicHash crypto {QCryptographicHash::Sha256};

        crypto.addData(cpu->getCpuInfo()->brand.toUtf8());
        crypto.addData(os->getSystemInfo()->product.toUtf8());
        crypto.addData(QString::number(static_cast<int>(os->getSystemInfo()->osType)).toUtf8());

        return crypto.result();
    }

    PWTS::Features Device::getFeatures() const {
        return deviceFeatures;
    }

    PWTS::CPUVendor Device::getCPUVendor() const {
        return cpu->getCpuInfo()->vendor;
    }

    QSharedPointer<PWTS::SystemInfo> Device::getSystemInfo() const {
        return os->getSystemInfo();
    }

    PWTS::DynamicSystemInfo Device::getDynamicSystemInfo() const {
        return os->getDynamicSystemInfo(cpu->getCpuInfo()->numLogicalCpus);
    }

    QSharedPointer<PWTS::CpuInfo> Device::getCPUInfo() const {
        return cpu->getCpuInfo();
    }

    QMap<int, PWTS::GpuInfo> Device::getGPUInfoMap() const {
        QMap<int, PWTS::GpuInfo> ginfo;

        for (const QSharedPointer<GPUDevice> &gpu: gpus)
            ginfo.insert(gpu->getGpuInfo()->index, *(gpu->getGpuInfo()));

        return ginfo;
    }

    QMap<QString, QString> Device::getFanLabelsMap() const {
        QMap<QString, QString> labelMap;

        for (const QSharedPointer<FANDevice> &fan: fans)
            labelMap.insert(fan->getID(), fan->getFanString());

        return labelMap;
    }

    void Device::setupFanCurveTimer(const bool enable) const {
        if (!enable) {
            fanCurveTimer.reset();

        } else if (fanCurveTimer.isNull()) {
            fanCurveTimer.reset(new QTimer);
            fanCurveTimer->setInterval(7500);
            fanCurveTimer->setSingleShot(true);
            QObject::connect(fanCurveTimer.get(), &QTimer::timeout, this, &Device::onFanCurveTimerTimeout);
        }
    }

    void Device::prepareForSleep() const {
        if (os->setupOSAccess()) {
            for (const QSharedPointer<FANDevice> &fan: fans)
                fan->prepareForSleep();

            os->unsetOSAccess();

        } else if (logger->isLevel(PWTS::LogLevel::Error)) {
            logger->write(QStringLiteral("failed to setup os access"));
        }
    }

    void Device::fillPacketDeviceData(PWTS::DaemonPacket &packet) const {
        if (!fanCurveTimer.isNull())
            fanCurveTimer->stop();

        if (!os->setupOSAccess())
            packet.errors.insert(PWTS::DError::OS_ACCESS_FAIL);

        cpu->fillDaemonPacket(deviceFeatures.cpu, coreIdxList, packet);
        os->fillDaemonPacket(deviceFeatures, cpu->getCpuInfo()->vendor, cpu->getCpuInfo()->numLogicalCpus, packet);

        for (const QSharedPointer<FANDevice> &fan: fans)
            packet.fanData.insert(fan->getID(), fan->getFanData());

        os->unsetOSAccess();

        if (!fanCurveTimer.isNull())
            fanCurveTimer->start();
    }

    QSet<PWTS::DError> Device::applySettings(const PWTS::ClientPacket &packet) const {
        if (!fanCurveTimer.isNull())
            fanCurveTimer->stop();

        QSet<PWTS::DError> errors;
        bool hasFanCurve = false;

        if (!os->setupOSAccess())
            errors.insert(PWTS::DError::OS_ACCESS_FAIL);

        errors.unite(cpu->applySettings(deviceFeatures.cpu, coreIdxList, packet));
        errors.unite(os->applySettings(deviceFeatures, cpu->getCpuInfo()->vendor, cpu->getCpuInfo()->numLogicalCpus, coreIdxList, packet));

        for (const QSharedPointer<FANDevice> &fan: fans) {
            const QString fanId = fan->getID();

            if (!packet.fanData.contains(fanId))
                continue;

            const PWTS::FanData &fanData = packet.fanData[fanId];

            if (fanData.mode.isValid() && fanData.mode.getValue() != 0 && fanData.curve.size() > 1)
                hasFanCurve = true;

            errors.unite(fan->applySettings(fanData));
        }

        os->unsetOSAccess();
        setupFanCurveTimer(hasFanCurve);

        if (!fanCurveTimer.isNull())
            onFanCurveTimerTimeout(); // apply curve immediately

        return errors;
    }

    void Device::onFanCurveTimerTimeout() const {
        const bool logErrorLev = logger->isLevel(PWTS::LogLevel::Error);

        if (os->setupOSAccess()) {
            for (const QSharedPointer<FANDevice> &fan: fans) {
                if (!fan->hasFanCurve())
                    continue;

                bool res = true;

                switch (fan->getFanType()) {
                    case FanType::CPU:
                        res = fan->applyCurve(cpu->getTemperature());
                        break;
                    default:
                        break;
                }

                if (!res && logErrorLev)
                    logger->write(QString("failed to apply fan curve to: %1").arg(fan->getID()));
            }

            os->unsetOSAccess();

        } else if (logErrorLev) {
            logger->write(QStringLiteral("failed to setup os access"));
        }

        fanCurveTimer->start();
    }
}
