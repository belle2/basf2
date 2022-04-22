/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/top/dataobjects/TRGTOPCombinedT0Decision.h"

//#include <framework/datastore/RelationVector.h>

//using namespace std;
using namespace Belle2;

void TRGTOPCombinedT0Decision::setCombinedT0(int combinedT0) { m_combinedT0 = combinedT0 ; }
void TRGTOPCombinedT0Decision::setRVCNow(int rvcNow) { m_rvcNow = rvcNow ; }
void TRGTOPCombinedT0Decision::setCombinedT0ClockCycle(int combinedT0ClockCycle) { m_combinedT0ClockCycle = combinedT0ClockCycle ; }
void TRGTOPCombinedT0Decision::setCombinedT0RVC2GDL(int combinedT0RVC2GDL) { m_combinedT0RVC2GDL = combinedT0RVC2GDL ; }
void TRGTOPCombinedT0Decision::setEventNumberL1(int eventNumberL1) { m_eventNumberL1 = eventNumberL1 ; }
void TRGTOPCombinedT0Decision::setRVCL1(int rvcL1) { m_rvcL1 = rvcL1 ; }

//void TRGTOPCombinedT0Decision::set(int ) { m_ = ; }

// no longer needed (in ROOT v6)
//ClassImp(TRGTOPCombinedT0Decision)



