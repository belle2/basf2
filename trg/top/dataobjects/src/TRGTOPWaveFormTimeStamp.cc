/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/top/dataobjects/TRGTOPWaveFormTimeStamp.h"

#include "trg/top/dataobjects/TRGTOPWaveFormTimeStampsSlot.h"
using namespace Belle2;

void TRGTOPWaveFormTimeStamp::setTimeStamp(int timeStamp) { m_timeStamp      = timeStamp     ; }
void TRGTOPWaveFormTimeStamp::setSlot(int slot) { m_slot      = slot     ; }

bool TRGTOPWaveFormTimeStamp::isEmptyClockCycle() const
{
  return (m_timeStamp == 65535);
}

bool TRGTOPWaveFormTimeStamp::isValid() const
{
  if (isEmptyClockCycle()) return false;
  if (m_timeStamp > 46080 || m_timeStamp < 0) return false;
  return true;
}
