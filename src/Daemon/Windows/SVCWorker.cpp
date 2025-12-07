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
#include "SVCWorker.h"

#include <QEventLoop>

#define SVCNAME TEXT("PowerTunerDaemon")

namespace PWTD {
    SVCWorker::SVCWorker(const bool cmdNoClients, const QString &cmdAdr, const quint16 cmdPort) {
        srvNoClients = cmdNoClients;
        srvAdr = cmdAdr;
        srvPort = cmdPort;
    }

    VOID SVCWorker::svcEventLog(const TCHAR *msg, const WORD type) {
        HANDLE hEventSource = RegisterEventSource(nullptr, SVCNAME);
        std::wstring logMsg;
        LPCTSTR msgData[2];

        if (hEventSource == nullptr)
            return;

        logMsg.assign(msg);

        if (type == EVENTLOG_ERROR_TYPE)
            logMsg.append(std::format(L", error {}", GetLastError()));

        msgData[0] = SVCNAME;
        msgData[1] = logMsg.c_str();

        ReportEvent(hEventSource, type, 0, 63010, nullptr, 2, 0, msgData, nullptr);
        DeregisterEventSource(hEventSource);
    }

    VOID SVCWorker::reportSvcStatus(const DWORD state, const DWORD exitCode, const DWORD waitHint) {
        SERVICE_STATUS status {
            .dwServiceType = SERVICE_WIN32_OWN_PROCESS,
            .dwCurrentState = state,
            .dwControlsAccepted = 0,
            .dwWin32ExitCode = exitCode,
            .dwServiceSpecificExitCode = 0,
            .dwCheckPoint = 0,
            .dwWaitHint = waitHint
        };

        currentState = state;

        if (state != SERVICE_START_PENDING && state != SERVICE_STOPPED)
            status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PRESHUTDOWN;

        if (state != SERVICE_RUNNING && state != SERVICE_STOPPED)
            status.dwCheckPoint = checkpoint++;

        SetServiceStatus(svcStatusHandle, &status);
    }

    DWORD WINAPI SVCWorker::svcCtrlHandler(const DWORD control, const DWORD evtType, LPVOID evtData, LPVOID context) {
        switch (control) {
            case SERVICE_CONTROL_STOP: {
                svcEventLog(L"Service stop request received", EVENTLOG_INFORMATION_TYPE);
                reportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 3000);
                svcSigHandler->signalSvcStop();
            }
                break;
            case SERVICE_CONTROL_PRESHUTDOWN: {
                svcEventLog(L"System pre-shutdown event received", EVENTLOG_INFORMATION_TYPE);
                reportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 3000);
                svcSigHandler->signalSvcStop();
            }
                break;
            case SERVICE_CONTROL_INTERROGATE: {
                svcEventLog(L"Service interrogate request received", EVENTLOG_INFORMATION_TYPE);
                reportSvcStatus(currentState, NO_ERROR, 0);
            }
                break;
            default:
                svcEventLog(std::format(L"svcCtrlHandler: request {} not implemented", control).c_str(), EVENTLOG_WARNING_TYPE);
                return ERROR_CALL_NOT_IMPLEMENTED;
        }

        return NO_ERROR;
    }

    VOID WINAPI SVCWorker::svcMain([[maybe_unused]] const DWORD dwArgc, [[maybe_unused]] LPTSTR *lpszArgv) {
        svcStatusHandle = RegisterServiceCtrlHandlerEx(SVCNAME, svcCtrlHandler, nullptr);
        if (!svcStatusHandle) {
            svcEventLog(L"Failed to get status handle", EVENTLOG_ERROR_TYPE);
            reportSvcStatus(SERVICE_STOPPED, NO_ERROR, 500);
            return;
        }

        reportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 500);

        QEventLoop eloop;

        service.reset(new DaemonService);
        service->start(!srvNoClients, srvAdr, srvPort);

        QObject::connect(svcSigHandler.get(), &SVCSigHandler::svcStopped, &eloop, &QEventLoop::quit);

        reportSvcStatus(SERVICE_RUNNING, NO_ERROR, 500);
        svcEventLog(L"svc is running", EVENTLOG_INFORMATION_TYPE);
        eloop.exec();
    }

    void SVCWorker::start() {
        TCHAR name[] = L"";
        SERVICE_TABLE_ENTRY svcTable[] = {
            {name, svcMain},
            {nullptr, nullptr}
        };

        svcSigHandler.reset(new SVCSigHandler);

        QObject::connect(svcSigHandler.get(), &SVCSigHandler::svcStopped, this, &SVCWorker::onSvcStopped);

        if (!StartServiceCtrlDispatcher(svcTable)) {
            svcEventLog(std::format(L"Failed to start svc: error {}", GetLastError()).c_str(), EVENTLOG_ERROR_TYPE);
            emit svcStopped();
        }

        svcEventLog(L"svc exiting", EVENTLOG_INFORMATION_TYPE);
        service.reset();
        svcSigHandler.reset();
        emit svcStopped();
    }

    void SVCWorker::onSvcStopped() {
        service.reset();
        svcSigHandler.reset();
        svcEventLog(L"svc stopped", EVENTLOG_INFORMATION_TYPE);
        reportSvcStatus(SERVICE_STOPPED, NO_ERROR, 500);
        emit svcStopped();
    }
}
