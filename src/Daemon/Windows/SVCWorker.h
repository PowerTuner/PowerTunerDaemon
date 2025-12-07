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

#include "pwtWin32/win.h"
#include "SVCSigHandler.h"
#include "../../Service/DaemonService.h"

namespace PWTD {
    class SVCWorker final: public QObject {
        Q_OBJECT

    private:
        static inline QScopedPointer<SVCSigHandler> svcSigHandler;
        static inline QScopedPointer<DaemonService> service;
        static inline SERVICE_STATUS_HANDLE svcStatusHandle = nullptr;
        static inline DWORD currentState = SERVICE_STOPPED;
        static inline DWORD checkpoint = 1;
        static inline bool srvNoClients;
        static inline QString srvAdr;
        static inline quint16 srvPort;

        static VOID svcEventLog(const TCHAR *msg, WORD type);
        static VOID reportSvcStatus(DWORD state, DWORD exitCode, DWORD waitHint);
        static DWORD WINAPI svcCtrlHandler(DWORD control, DWORD evtType, LPVOID evtData, LPVOID context);
        static VOID WINAPI svcMain(DWORD dwArgc, LPTSTR *lpszArgv);

    public:
        SVCWorker(bool cmdNoClients, const QString &cmdAdr, quint16 cmdPort);

        void start();

    private slots:
        void onSvcStopped();

    signals:
        void svcStopped();
    };
}
