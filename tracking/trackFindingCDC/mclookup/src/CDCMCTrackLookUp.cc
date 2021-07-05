/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitCollectionLookUp.icc.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::CDCMCHitCollectionLookUp<CDCTrack>;

const CDCMCTrackLookUp& CDCMCTrackLookUp::getInstance()
{
  return CDCMCManager::getMCTrackLookUp();
}
