/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/dataobjects/Digit.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;


//------------------------------------------------------------------------------
// ProtoDigit class
//------------------------------------------------------------------------------
void Digit::print() const
{
  B2INFO(" Digit: charge = " << charge)
  for (StoreRelationMapItr iHit = sourceHits.begin(); iHit != sourceHits.end(); ++iHit)
    B2INFO("   hit: " << iHit->first << " charge: " << iHit->second);
}

