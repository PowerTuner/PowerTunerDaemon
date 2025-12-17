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
    void serializeIntelData(QDataStream &ds, const PWTS::ClientPacket &packet);
    void serializeIntelCoreData(QDataStream &ds, const PWTS::Intel::IntelCoreData &coreData);
    void serializeIntelThreadData(QDataStream &ds, const PWTS::Intel::IntelThreadData &thdData);
    QSharedPointer<PWTS::Intel::IntelData> deserializeIntelData(QDataStream &ds);
    void deserializeIntelCoreData(QDataStream &ds, const QSharedPointer<PWTS::Intel::IntelData> &data, int version);
    void deserializeIntelThreadData(QDataStream &ds, const QSharedPointer<PWTS::Intel::IntelData> &profile, int version);
    void loadIntelProfileToDaemonPacket(const QSharedPointer<PWTS::Intel::IntelData> &profile, const QSharedPointer<PWTS::Intel::IntelData> &packet);
}
