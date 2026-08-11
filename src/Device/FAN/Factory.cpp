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

#include "Factory.h"
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

namespace PWTD::FAN {
#ifdef WITH_GPD_FAN
    static std::shared_ptr<FANDevice> GPDFactory(const std::shared_ptr<OS> &os, const int CPUExtModel, const QString &id) {
        const QSharedPointer<PWTS::SystemInfo> sysInfo = os->getSystemInfo();

        if (sysInfo->product == "G1618-04") { // win 4
            if (CPUExtModel == 0x44) { // 6800u
                const std::shared_ptr<GPD::GPDWin4FanBoard> fan = std::make_shared<GPD::GPDWin4FanBoard>(os, id);
#ifdef _WIN32
                if (!std::dynamic_pointer_cast<OSWindows>(os)->gpdWin4ECInit(fan->getControls()))
                    return {};
#endif
                return fan;
            }

            return std::make_shared<GPD::GPDWinMax2FanBoard>(os, id); // 7xxx/8xxx/hx

        } else if (sysInfo->product == "G1617-01" || sysInfo->product == "G1617-02" || sysInfo->product == "G1617-02-L" || // win mini
                    sysInfo->product == "G1628-04" || sysInfo->product == "G1628-04-L" // pocket 4
        ) {
            return std::make_shared<GPD::GPDWinMiniFanBoard>(os, id);

        } else if (sysInfo->product == "G1619-04" || sysInfo->product == "G1619-05") { // max 2
            return std::make_shared<GPD::GPDWinMax2FanBoard>(os, id);

        } else if (sysInfo->product == "G1622-01" || sysInfo->product == "G1622-01-L" || // duo
                    sysInfo->product == "G1618-05" // win 5
        ) {
            return std::make_shared<GPD::GPDDUOFanBoard>(os, id);

        } else if (sysInfo->product == "G1688-08") { // micro pc 2
            return std::make_shared<GPD::GPDMPC2FanBoard>(os, id);
        }

        return {};
    }
#endif

    std::shared_ptr<FANDevice> CPUFANfactory(const std::shared_ptr<OS> &os, const int CPUExtModel) {
        const QSharedPointer<PWTS::SystemInfo> sysInfo = os->getSystemInfo();
        QCryptographicHash crypto {QCryptographicHash::Sha256};
        QString id;

        crypto.addData(sysInfo->product.toUtf8());

        id = crypto.result().toHex().left(6);

#ifdef WITH_GPD_FAN
        if (sysInfo->manufacturer == "GPD")
            return GPDFactory(os, CPUExtModel, id);
#endif

        return {};
    }
}
