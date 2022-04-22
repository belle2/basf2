/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/top/dataobjects/TRGTOPTimingISimSlot.h"

#include "trg/top/dataobjects/TRGTOPTimingISim.h"
#include <top/dataobjects/TOPDigit.h>
//#include <framework/datastore/RelationVector.h>

//using namespace std;
using namespace Belle2;

void TRGTOPTimingISimSlot::setSlotId(int slotId) { m_slotId = slotId; }
void TRGTOPTimingISimSlot::setNumberOfTimeStamps(int numberOfTimeStamps) { m_numberOfTimeStamps = numberOfTimeStamps; }
//void TRGTOPTimingISimSlot::set(int ) { m_ = ; }

//void TRGTOPTimingISimSlot::addTimingISim(TRGTOPTimingISim timingISim) { m_timingISims.push_back(timingISim); }

// no longer needed (in ROOT v6)
//ClassImp(TRGTOPTimingISimSlot)



