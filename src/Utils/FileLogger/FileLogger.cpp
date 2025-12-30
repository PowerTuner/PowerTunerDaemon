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
#include <QDir>

#include "FileLogger.h"
#include "../AppDataPath.h"

namespace PWTD {
    FileLogger::FileLogger() {
        basePath = AppDataPath::appDataLocation();

        if (basePath.isEmpty())
            qWarning("no log file path");
    }

    FileLogger::~FileLogger() {
        logFile.close();
    }

    void FileLogger::deleteOldestLogFile() const {
        const QDirListing dit(basePath, {"*.log"}, QDirListing::IteratorFlag::FilesOnly);
        QDateTime birth = QDateTime::currentDateTime();
        QString oldest;

        for (const QDirListing::DirEntry &entry: dit) {
            if (entry.fileInfo().birthTime() >= birth)
                continue;

            birth = entry.fileInfo().birthTime();
            oldest = entry.filePath();
        }

        if (!oldest.isEmpty() && !QFile::remove(oldest))
            qWarning("failed to delete oldest log");
    }

    int FileLogger::getLogFileCount() const {
        const QDirListing dit(basePath, {"*.log"}, QDirListing::IteratorFlag::FilesOnly);
        int i = 0;

        for ([[maybe_unused]] const QDirListing::DirEntry &entry: dit)
            ++i;

        return i;
    }

    QSharedPointer<FileLogger> FileLogger::getInstance() {
        if (instance.isNull())
            instance.reset(new FileLogger);

        return instance;
    }

    void FileLogger::init(const PWTS::LogLevel lvl, const int maxFiles) {
        if (basePath.isEmpty())
            return;

        logFile.close();

        level = lvl;
        maxLogFiles = maxFiles;

        if (level == PWTS::LogLevel::None)
            return;

        const QString logPath = QString("%1/powertunerd-%2.log").arg(basePath, QDateTime::currentDateTime().toString("ddd_MMMM_d_yyyy_hh_mm_ss"));

        logFile.setFileName(logPath);

        if (getLogFileCount() >= maxLogFiles)
            deleteOldestLogFile();

        if (!logFile.open(QFile::Text | QFile::WriteOnly)) {
            qWarning() << QString("cannot open log file %1 for write: %2").arg(logPath, logFile.errorString());
            level = PWTS::LogLevel::None;

        } else {
            ts.setDevice(&logFile);
        }
    }

    void FileLogger::setLevel(const PWTS::LogLevel lvl) {
        level = lvl;
    }

    bool FileLogger::isLevel(const PWTS::LogLevel lvl) const {
        return static_cast<int>(lvl) >= static_cast<int>(level);
    }

    void FileLogger::write(const QString &msg, const std::source_location source) {
        if (!logFile.isOpen() || level == PWTS::LogLevel::None)
            return;

        if (logFile.size() >= limit)
            init(level, maxLogFiles);

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
