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
  // FIXME : Update with new uncertain fits

  Vector3D position;
  Vector3D momentum;

  // Translate the trajectory information
  if (getFBInfo() == BACKWARD) {
    position = getEndFitPos3D();
    momentum = getEndFitMom3D();

  } else {
    position = getStartFitPos3D();
    momentum = getStartFitMom3D();

  }

  // The initial covariance matrix is calculated from these errors and it is important (!!) that it is not completely wrong - took them from Oksana
  // Currently unused
  TVector3 posError;
  posError.SetXYZ(2.0, 2.0, 2.0);
  TVector3 momError;
  momError.SetXYZ(0.1, 0.1, 0.5);

  // Set the start parameters
  SignType q = getChargeSign();
  trackCand.setPosMomSeed(position, momentum, q);

  // int pdg = track.getPID();
  // gfTrackCand.setPosMomSeedAndPdgCode(position, momentum, pdg);





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






