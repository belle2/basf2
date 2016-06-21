/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/segments/CDCRLWireHitSegment.h>

#include <genfit/WireTrackCandHit.h>
#include <framework/gearbox/Const.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <framework/datastore/StoreArray.h>
#include <genfit/TrackCand.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

vector<const CDCWire*> CDCRLWireHitSegment::getWireSegment() const
{
  std::vector<const Belle2::TrackFindingCDC::CDCWire*> wireSegment;
  for (const CDCRLWireHit& rlWireHit : *this) {
    wireSegment.push_back(&(rlWireHit.getWire()));
  }
  return wireSegment;
}
