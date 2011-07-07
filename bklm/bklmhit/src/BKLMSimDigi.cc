/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/bklmhit/BKLMSimDigi.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(Belle2::BKLMSimDigi)

G4Allocator<BKLMSimDigi> BKLMSimDigiAllocator;

void BKLMSimDigi::Store(void) const
{
  B2INFO("BKLMSimDigi::Store called");
}



//} //end of Belle2 namespace
