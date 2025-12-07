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

#include <QSharedPointer>
#include <QString>

namespace PWTD {
    class DaemonSettingDiskManager final {
    private:
        static inline QSharedPointer<DaemonSettingDiskManager> instance;
        const QString signature = "PWTDS";
        QString path;

        DaemonSettingDiskManager();

    public:
        DaemonSettingDiskManager(const DaemonSettingDiskManager &) = delete;
        DaemonSettingDiskManager &operator=(const DaemonSettingDiskManager &) = delete;

        [[nodiscard]] static QSharedPointer<DaemonSettingDiskManager> getInstance();
        [[nodiscard]] QByteArray load() const;
        [[nodiscard]] bool save(const QByteArray &data) const;
    };
}
