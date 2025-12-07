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

#include "DaemonSettingDiskManager.h"
#include "../Utils/AppDataPath.h"
#include "pwtShared/Utils.h"

namespace PWTD {
    DaemonSettingDiskManager::DaemonSettingDiskManager() {
        path = AppDataPath::appDataLocation();

        if (!path.isEmpty())
            path.append("/powertuner.pwtd");
    }

    QSharedPointer<DaemonSettingDiskManager> DaemonSettingDiskManager::getInstance() {
        if (!instance.isNull())
            return instance;

        instance.reset(new DaemonSettingDiskManager);
        return instance;
    }

    QByteArray DaemonSettingDiskManager::load() const {
        if (path.isEmpty())
            return {};

        const QByteArray data = PWTS::loadFile(path);
        QDataStream ds(data);
        QString fsignature;
        QByteArray checksum;
        QByteArray settingsData;

        if (data.isEmpty())
            return {};

        ds >> fsignature;
        if (fsignature != signature) {
            qWarning("Failed to load daemon settings: invalid signature");
            return {};
        }

        ds >> checksum >> settingsData;
        if (checksum != QCryptographicHash::hash(settingsData, QCryptographicHash::Sha256)) {
            qWarning("Failed to load daemon settings: checksum mismatch");
            return {};
        }

        return settingsData;
    }

    bool DaemonSettingDiskManager::save(const QByteArray &data) const {
        if (path.isEmpty())
            return false;

        QByteArray fdata;
        QDataStream ds(&fdata, QIODevice::WriteOnly);

        ds << signature <<
            QCryptographicHash::hash(data, QCryptographicHash::Sha256) <<
            data;

        return PWTS::writeFile(path, fdata);
    }
}
