/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/top/dataobjects/TRGTOPSlotTiming.h"

//using namespace std;
using namespace Belle2;

void TRGTOPSlotTiming::setSlotId(int slotId) { m_slotId      = slotId     ; }
void TRGTOPSlotTiming::setSlotTiming(int slotTiming) { m_slotTiming  = slotTiming ; }
void TRGTOPSlotTiming::setSlotSegment(int slotSegment) { m_slotSegment = slotSegment; }
void TRGTOPSlotTiming::setSlotNHits(int slotNHits) { m_slotNHits   = slotNHits  ; }
void TRGTOPSlotTiming::setSlotLogL(int slotLogL) { m_slotLogL    = slotLogL   ; }
void TRGTOPSlotTiming::setSlotDecisionClockCycle(int slotDecisionClockCycle) { m_slotDecisionClockCycle = slotDecisionClockCycle; }
void TRGTOPSlotTiming::setSlotNErrors(int slotNErrors) { m_slotNErrors = slotNErrors; }
void TRGTOPSlotTiming::setSlotThisBoard(int slotThisBoard) { m_slotThisBoard = slotThisBoard; }
//void TRGTOPSlotTiming::set(int ) { m_ = ; }
void TRGTOPSlotTiming::setSlotFirstTS(int slotFirstTS) { m_slotFirstTS = slotFirstTS; }

bool TRGTOPSlotTiming::isThisBoard() const
{
  if (m_slotThisBoard == 1) return true;
  return false;
}

bool TRGTOPSlotTiming::isFirstTSAvailable() const
{
  if (m_slotThisBoard != 1) return false;
  if (m_slotFirstTS == -1) return false;
  return true;
}

// no longer needed (in ROOT v6)
//ClassImp(TRGTOPSlotTiming)



