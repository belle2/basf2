/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/top/dataobjects/TRGTOPWaveFormTimeStamp.h"

#include "trg/top/dataobjects/TRGTOPWaveFormTimeStampsSlot.h"

//#include <framework/datastore/RelationVector.h>

//using namespace std;
using namespace Belle2;

void TRGTOPWaveFormTimeStamp::setTimeStamp(int timeStamp) { m_timeStamp      = timeStamp     ; }
void TRGTOPWaveFormTimeStamp::setSlot(int slot) { m_slot      = slot     ; }

bool TRGTOPWaveFormTimeStamp::isEmptyClockCycle() const
{
  if (m_timeStamp == 65535) return true;
  return false;
}

bool TRGTOPWaveFormTimeStamp::isValid() const
{
  if (isEmptyClockCycle()) return false;
  if (m_timeStamp > 46080 || m_timeStamp < 0) return false;
  return true;
}

//void TRGTOPWaveFormTimeStamp::set(int ) { m_ = ; }

// no longer needed (in ROOT v6)
//ClassImp(TRGTOPWaveFormTimeStamp)



