/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/top/dataobjects/TRGTOPTimingISim.h"

//using namespace std;
using namespace Belle2;

void TRGTOPTimingISim::setSlotId(int slotId) { m_slotId      = slotId     ; }
void TRGTOPTimingISim::setSlotTiming(int slotTiming) { m_slotTiming  = slotTiming ; }
void TRGTOPTimingISim::setSlotSegment(int slotSegment) { m_slotSegment = slotSegment; }
void TRGTOPTimingISim::setSlotNHits(int slotNHits) { m_slotNHits   = slotNHits  ; }
void TRGTOPTimingISim::setSlotLogL(int slotLogL) { m_slotLogL    = slotLogL   ; }
void TRGTOPTimingISim::setSlotDecisionClockCycle(int slotDecisionClockCycle) { m_slotDecisionClockCycle = slotDecisionClockCycle; }
//void TRGTOPTimingISim::set(int ) { m_ = ; }

// no longer needed (in ROOT v6)
//ClassImp(TRGTOPTimingISim)



