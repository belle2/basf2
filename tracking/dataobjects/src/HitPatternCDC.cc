/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/dataobjects/HitPatternCDC.h>
using namespace Belle2;


bool HitPatternCDC::getSLayer(short sLayer)
{
  // As any, count etc. don't work range-based, we use a masking of the relevant range and count then.

  return false;
}
