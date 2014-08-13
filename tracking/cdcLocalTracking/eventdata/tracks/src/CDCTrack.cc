/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCTrack.h"

#include <genfit/WireTrackCandHit.h>
#include <framework/gearbox/Const.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;
using namespace genfit;

ClassImpInCDCLocalTracking(CDCTrack)




void CDCTrack::fillInto(genfit::TrackCand& trackCand) const
{
  // Translate the trajectory information
  if (getFBInfo() == BACKWARD) {
    getEndTrajectory3D().fillInto(trackCand);
  } else {
    getStartTrajectory3D().fillInto(trackCand);
  }



  // Add the hits
  if (getFBInfo() == BACKWARD) {

    // Calculate the travel distance relative to the end of the track
    FloatType endPrepS = getEndRecoHit3D().getPerpS();

    for (const CDCRecoHit3D & recoHit3D : reverseRange()) {
      // the hit id correspondes to the index in the TClonesArray
      // this is stored in the wirehit the recohit is based on
      unsigned int storeIHit = recoHit3D.getWireHit().getStoreIHit();

      // the plain id serves to mark competition between two or more hits
      // use the wire id here which is unique for all the hits in the track
      // but it may also serve the fact that a track can only be responsable for a single hit on each wire
      // double hits should correspond to multiple tracks
      unsigned int planeId = recoHit3D.getWire().getEWire();

      // the sorting parameter is used to sort the cdchits accoring to the direction of flight
      // we assign the estimated flight distance to them
      double sortingParameter = endPrepS - recoHit3D.getPerpS(); // translate it to a scale from the end of the track

      // Right left ambiguity resolution from the hypotheses stored in the reconstructed hit must be reversed to
      RightLeftInfo rlInfo = reversed(recoHit3D.getRLInfo());
      // Note:  rlInfo < 0 means LEFT,   rlInfo > 0 means RIGHT,  which is the same as in Genfit
      char genfitLeftRight = rlInfo;


      //do not delete! the genfit::TrackCand has ownership
      WireTrackCandHit* aCdcTrackCandHit = new WireTrackCandHit(Const::CDC,
                                                                storeIHit,
                                                                planeId,
                                                                sortingParameter,
                                                                genfitLeftRight);

      trackCand.addHit(aCdcTrackCandHit);

    }

  } else {
    for (const CDCRecoHit3D & recoHit3D : *this) {
      //the hit id correspondes to the index in the TClonesArray
      //this is stored in the wirehit the recohit is based on
      int storeIHit = recoHit3D.getWireHit().getStoreIHit();

      //the plain id serves to mark competition between two or more hits
      //use the wire id here which is unique for all the hits in the track
      //but it may also serve the fact that a track can only be responsable for a single hit on each wire
      //double hits should correspond to multiple tracks
      int planeId = recoHit3D.getWire().getEWire();

      // the rho parameter is used to sort the cdchits accoring to the direction of flight
      // we assign the estimated flight distance to them
      double sortingParameter = recoHit3D.getPerpS();

      // Right left ambiguity resolution from the hypotheses stored in the reconstructed hit not reversed in the normal case
      RightLeftInfo rlInfo = recoHit3D.getRLInfo();
      // Note:  rlInfo < 0 means LEFT,   rlInfo > 0 means RIGHT,  which is the same as in Genfit
      char genfitLeftRight = rlInfo;

      //do not delete! the genfit::TrackCand has ownership
      WireTrackCandHit* aCdcTrackCandHit = new WireTrackCandHit(Const::CDC,
                                                                storeIHit,
                                                                planeId,
                                                                sortingParameter,
                                                                genfitLeftRight);

      trackCand.addHit(aCdcTrackCandHit);

    }

  }

}






