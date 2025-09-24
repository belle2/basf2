/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/top/dataobjects/TRGTOPSlotTiming.h>

using namespace Belle2;

void TRGTOPSlotTiming::setSlotId(int slotId) { m_slotId      = slotId     ; }
void TRGTOPSlotTiming::setSlotTiming(int slotTiming) { m_slotTiming  = slotTiming ; }
void TRGTOPSlotTiming::setSlotSegment(int slotSegment) { m_slotSegment = slotSegment; }
void TRGTOPSlotTiming::setSlotNHits(int slotNHits) { m_slotNHits   = slotNHits  ; }
void TRGTOPSlotTiming::setSlotLogL(int slotLogL) { m_slotLogL    = slotLogL   ; }
void TRGTOPSlotTiming::setSlotDecisionClockCycle(int slotDecisionClockCycle) { m_slotDecisionClockCycle = slotDecisionClockCycle; }
void TRGTOPSlotTiming::setSlotNErrors(int slotNErrors) { m_slotNErrors = slotNErrors; }
void TRGTOPSlotTiming::setSlotThisBoard(int slotThisBoard) { m_slotThisBoard = slotThisBoard; }
void TRGTOPSlotTiming::setSlotFirstTS(int slotFirstTS) { m_slotFirstTS = slotFirstTS; }

bool TRGTOPSlotTiming::isThisBoard() const
{
  return (m_slotThisBoard == 1);
}

bool TRGTOPSlotTiming::isFirstTSAvailable() const
{
  return (m_slotThisBoard != 1 or m_slotFirstTS == -1) ? false : true;
}
