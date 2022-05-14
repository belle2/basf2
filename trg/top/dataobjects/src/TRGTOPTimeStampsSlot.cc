/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/top/dataobjects/TRGTOPTimeStampsSlot.h"

#include "trg/top/dataobjects/TRGTOPTimeStamp.h"
#include <top/dataobjects/TOPDigit.h>
//#include <framework/datastore/RelationVector.h>

//using namespace std;
using namespace Belle2;

void TRGTOPTimeStampsSlot::setSlotId(int slotId) { m_slotId = slotId; }
void TRGTOPTimeStampsSlot::setNumberOfTimeStamps(int numberOfTimeStamps) { m_numberOfTimeStamps = numberOfTimeStamps; }
//void TRGTOPTimeStampsSlot::set(int ) { m_ = ; }

//void TRGTOPTimeStampsSlot::addTimeStamp(TRGTOPTimeStamp timeStamp) { m_timeStamps.push_back(timeStamp); }

// no longer needed (in ROOT v6)
//ClassImp(TRGTOPTimeStampsSlot)



