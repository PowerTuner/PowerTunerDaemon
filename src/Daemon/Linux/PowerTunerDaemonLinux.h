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

#include "../PowerTunerDaemon.h"
#include "../SignalNotifier.h"

namespace PWTD {
    class PowerTunerDaemonLinux final: public PowerTunerDaemon {
        Q_OBJECT

    private:
        static inline QScopedPointer<SignalNotifier> sigNotifier {new SignalNotifier};
#ifdef SYSTEMD_NOTIFY
        bool cmdSystemdDaemon = false;
#endif

        static void sigterm([[maybe_unused]] int sig) { sigNotifier->signalSigTerm(); }
        static void sigHup([[maybe_unused]] int sig) { sigNotifier->signalSigHup(); }

    public:
        PowerTunerDaemonLinux();

        void setupCmdArgs() const override;
        void parseCmdArgs(const QCoreApplication &app) override;
        [[nodiscard]] int run() override;

    public slots:
        void onSigTerm();
        void onSigHup() const;
    };
}
