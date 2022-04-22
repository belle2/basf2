/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/top/dataobjects/TRGTOPTimeStamp.h"

#include "trg/top/dataobjects/TRGTOPTimeStampsSlot.h"
#include <top/dataobjects/TOPDigit.h>
//#include <framework/datastore/RelationVector.h>

//using namespace std;
using namespace Belle2;

void TRGTOPTimeStamp::setTimeStamp(int timeStamp) { m_timeStamp      = timeStamp     ; }
void TRGTOPTimeStamp::setSlot(int slot) { m_slot      = slot     ; }

//void TRGTOPTimeStamp::set(int ) { m_ = ; }

// no longer needed (in ROOT v6)
//ClassImp(TRGTOPTimeStamp)



