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

#include "FANFactory.h"
#ifdef _WIN32
#include "../OS/Windows/OSWindows.h"
#endif
#ifdef WITH_GPD_FAN
#include "GPD/GPDWin4FanBoard.h"
#include "GPD/GPDWinMiniFanBoard.h"
#include "GPD/GPDWinMax2FanBoard.h"
#include "GPD/GPDDUOFanBoard.h"
#include "GPD/GPDMPC2FanBoard.h"
#endif

namespace PWTD {
    void FANFactory::addCPUFan(const QSharedPointer<OS> &os, const int CPUExtModel, QList<QSharedPointer<FANDevice>> &fanList) {
        const QSharedPointer<PWTS::SystemInfo> sysInfo = os->getSystemInfo();
        QCryptographicHash crypto {QCryptographicHash::Sha256};
        QString fanID;

        crypto.addData(sysInfo->product.toUtf8());

        fanID = crypto.result().toHex().left(6);

#ifdef WITH_GPD_FAN
        if (sysInfo->manufacturer == "GPD") {
            if (sysInfo->product == "G1618-04") { // win 4
                if (CPUExtModel == 0x44) { // 6800u
                    const QSharedPointer<GPD::GPDWin4FanBoard> fan = QSharedPointer<GPD::GPDWin4FanBoard>::create(os, fanID);
#ifdef _WIN32
                    if (!qSharedPointerCast<WIN::OSWindows>(os)->gpdWin4ECInit(fan->getControls()))
                        return;
#endif
                    fanList.append(fan);

                } else { // 7xxx/8xxx/hx
                    fanList.append(QSharedPointer<GPD::GPDWinMax2FanBoard>::create(os, fanID));
                }
            } else if (sysInfo->product == "G1617-01" || sysInfo->product == "G1617-02" || sysInfo->product == "G1617-02-L") { // win mini
                fanList.append(QSharedPointer<GPD::GPDWinMiniFanBoard>::create(os, fanID));

            } else if (sysInfo->product == "G1619-04" || sysInfo->product == "G1619-05") { // max 2
                fanList.append(QSharedPointer<GPD::GPDWinMax2FanBoard>::create(os, fanID));

            } else if (sysInfo->product == "G1622-01" || sysInfo->product == "G1622-01-L") { // duo
                fanList.append(QSharedPointer<GPD::GPDDUOFanBoard>::create(os, fanID));

            } else if (sysInfo->product == "G1628-04" || sysInfo->product == "G1628-04-L") { // pocket 4
                fanList.append(QSharedPointer<GPD::GPDWinMiniFanBoard>::create(os, fanID));

            } else if (sysInfo->product == "G1618-05") { // win 5
                fanList.append(QSharedPointer<GPD::GPDDUOFanBoard>::create(os, fanID));

            } else if (sysInfo->product == "G1688-08") { // micro pc 2
                fanList.append(QSharedPointer<GPD::GPDMPC2FanBoard>::create(os, fanID));
            }
        }
#endif
    }

    QList<QSharedPointer<FANDevice>> FANFactory::getFans(const QSharedPointer<OS> &os, const int CPUExtModel) {
        QList<QSharedPointer<FANDevice>> fans;

        addCPUFan(os, CPUExtModel, fans);

        return fans;
    }
}
