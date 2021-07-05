/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

using namespace Belle2;
using namespace TrackFindingCDC;

const CDCMCSegment2DLookUp& CDCMCSegmentLookUp::getInstance()
{
  return CDCMCManager::getMCSegment2DLookUp();
}
