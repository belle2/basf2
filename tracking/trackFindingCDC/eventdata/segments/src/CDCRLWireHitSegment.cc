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

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <framework/datastore/StoreArray.h>
#include <genfit/TrackCand.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCRLWireHitSegment::CDCRLWireHitSegment(const genfit::TrackCand& gfTrackCand)
{
  // 1. Extract the hit content from the genfit track candidate.
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  StoreArray<CDCHit> storedHits;

  int nHits = gfTrackCand.getNHits();

  for (int iHit = 0 ; iHit < nHits; ++iHit) {
    // Extract the hit and detector id as output arguments
    int detId = 0;
    int hitId = 0;
    gfTrackCand.getHit(iHit, detId, hitId);

    if (detId == Const::CDC) {
      const genfit::TrackCandHit* ptrGFTrackCandHit = gfTrackCand.getHit(iHit);
      if (ptrGFTrackCandHit) {
        const genfit::TrackCandHit& gfTrackCandHit = *ptrGFTrackCandHit;
        const genfit::WireTrackCandHit& wireTrackCandHit = static_cast<const genfit::WireTrackCandHit&>(gfTrackCandHit);

        signed char gfLeftRight = wireTrackCandHit.getLeftRightResolution();
        B2INFO(gfLeftRight);

        ERightLeft rlInfo = (gfLeftRight > 0 ?
                             ERightLeft::c_Right :
                             (gfLeftRight < 0 ? ERightLeft::c_Left : ERightLeft::c_Invalid));

        if (rlInfo == ERightLeft::c_Invalid) {
          B2WARNING("Invalid right left passage information " << gfLeftRight << " received from genfit.");
          continue;
        }

        const CDCHit* ptrHit = storedHits[hitId];
        const CDCWireHit* ptrWireHit = wireHitTopology.getWireHit(ptrHit);
        if (ptrWireHit) {
          push_back(CDCRLWireHit(ptrWireHit, rlInfo));
        }

      }
    } // if (detId == Const::CDC)
  } // for iHit

  // 2. Extract the trajectory information
  CDCTrajectory3D trajectory3D(gfTrackCand);
  setTrajectory2D(trajectory3D.getTrajectory2D());
}

vector<const CDCWire*> CDCRLWireHitSegment::getWireSegment() const
{
  std::vector<const Belle2::TrackFindingCDC::CDCWire*> wireSegment;
  for (const CDCRLWireHit& rlWireHit : *this) {
    wireSegment.push_back(&(rlWireHit.getWire()));
  }
  return wireSegment;
}
