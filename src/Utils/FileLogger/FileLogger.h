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
#include <QTextStream>
#include <QFile>
#include <source_location>

#include "pwtShared/Include/LogLevel.h"

namespace PWTD {
    class FileLogger final {
    private:
        inline static QSharedPointer<FileLogger> instance;
        static constexpr qint64 limit = 50 * 1000 * 1000;
        PWTS::LogLevel level = PWTS::LogLevel::None;
        int maxLogFiles = 5;
        QString basePath;
        QTextStream ts;
        QFile logFile;

        FileLogger();

        void deleteOldestLogFile() const;
        [[nodiscard]] int getLogFileCount() const;

    public:
        FileLogger(const FileLogger &) = delete;
        FileLogger &operator=(const FileLogger &) = delete;

        ~FileLogger();

        [[nodiscard]] static QSharedPointer<FileLogger> getInstance();
        void init(PWTS::LogLevel lvl, int maxFiles);
        void setLevel(PWTS::LogLevel lvl);
        [[nodiscard]] bool isLevel(PWTS::LogLevel lvl) const;
        void write(const QString &msg, std::source_location source = std::source_location::current());
    };
}
