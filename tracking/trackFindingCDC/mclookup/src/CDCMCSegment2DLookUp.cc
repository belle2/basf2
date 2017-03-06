/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

using namespace Belle2;
using namespace TrackFindingCDC;

const CDCMCSegment2DLookUp& CDCMCSegment2DLookUp::getInstance()
{
  return CDCMCManager::getMCSegment2DLookUp();
}
