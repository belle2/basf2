/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/top/dataobjects/TRGTOPWaveFormTimeStampsSlot.h>

#include <top/dataobjects/TOPDigit.h>

using namespace Belle2;

void TRGTOPWaveFormTimeStampsSlot::setSlotId(int slotId) { m_slotId = slotId; }
void TRGTOPWaveFormTimeStampsSlot::setNumberOfTimeStamps(int numberOfTimeStamps) { m_numberOfTimeStamps = numberOfTimeStamps; }
void TRGTOPWaveFormTimeStampsSlot::setNumberOfActualTimeStamps(int numberOfActualTimeStamps) { m_numberOfActualTimeStamps = numberOfActualTimeStamps; }
void TRGTOPWaveFormTimeStampsSlot::setFirstActualTimeStampValue(int firstActualTimeStampValue) { m_firstActualTimeStampValue = firstActualTimeStampValue; }
void TRGTOPWaveFormTimeStampsSlot::setFirstActualTimeStampClockCycle(int firstActualTimeStampClockCycle) { m_firstActualTimeStampClockCycle = firstActualTimeStampClockCycle; }
