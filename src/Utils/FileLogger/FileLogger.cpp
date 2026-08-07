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
#include <QFileInfo>

#include "FileLogger.h"

namespace PWTD {
    FileLogger::~FileLogger() {
        logFile.close();
    }

    FileLogger &FileLogger::get() {
        static FileLogger instance;

        return instance;
    }

    void FileLogger::init() {
        if (logDir.isEmpty())
            return;

        logFile.close();

        if (level == PWTS::LogLevel::None)
            return;

        const QString oldLog = QString("%1/powertunerd.log.old").arg(logDir);
        const QString newLog = QString("%1/powertunerd.log").arg(logDir);

        QFile::remove(oldLog);
        QFile::rename(newLog, oldLog);
        logFile.setFileName(newLog);

        if (!logFile.open(QFile::Text | QFile::WriteOnly)) {
            qWarning() << QString("cannot open log file %1 for write: %2").arg(newLog, logFile.errorString());
            level = PWTS::LogLevel::None;

        } else {
            ts.setDevice(&logFile);
        }
    }

    void FileLogger::setOutput(const QString &path) {
        logDir = path;
    }

    void FileLogger::setLevel(const PWTS::LogLevel lvl) {
        level = lvl;
    }

    bool FileLogger::isLevel(const PWTS::LogLevel lvl) const {
        return static_cast<int>(lvl) >= static_cast<int>(level);
    }

    void FileLogger::write(const QString &msg, const std::source_location source) {
        if (!logFile.isOpen())
            return;

        const QFileInfo srcFInfo {source.file_name()};

        ts << QDateTime::currentDateTime().toString("[ddd MMMM d yyyy hh:mm:ss]") << "\n" <<
            srcFInfo.fileName() << "[" << source.line() << ":" << source.column() << "]: " << source.function_name() << "\n" <<
            msg << "\n\n";

        ts.flush();

        if (ts.status() != QTextStream::Ok) {
            level = PWTS::LogLevel::None;

            qWarning("failed to write log, log will be disabled");
            logFile.close();
        }
    }
}
