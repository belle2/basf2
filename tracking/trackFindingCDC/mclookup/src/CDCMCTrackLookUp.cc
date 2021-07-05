/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
