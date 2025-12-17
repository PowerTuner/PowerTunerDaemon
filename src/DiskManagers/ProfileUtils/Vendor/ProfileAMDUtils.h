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

#include "pwtShared/Include/Packets/ClientPacket.h"

namespace PWTD {
    void serializeAMDData(QDataStream &ds, const PWTS::ClientPacket &packet);
    void serializeAMDCoreData(QDataStream &ds, const PWTS::AMD::AMDCoreData &coreData);
    void serializeAMDThreadData(QDataStream &ds, const PWTS::AMD::AMDThreadData &thdData);
    QSharedPointer<PWTS::AMD::AMDData> deserializeAMDData(QDataStream &ds);
    void deserializeAMDCoreData(QDataStream &ds, const QSharedPointer<PWTS::AMD::AMDData> &data, int version);
    void deserializeAMDThreadData(QDataStream &ds, const QSharedPointer<PWTS::AMD::AMDData> &data, int version);
    void loadAMDProfileToDaemonPacket(const QSharedPointer<PWTS::AMD::AMDData> &profile, const QSharedPointer<PWTS::AMD::AMDData> &packet);
}
