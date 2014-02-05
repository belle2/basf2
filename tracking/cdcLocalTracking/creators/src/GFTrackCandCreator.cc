/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include "../include/GFTrackCandCreator.h"
#include <boost/foreach.hpp>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


GFTrackCandCreator::GFTrackCandCreator() {;}



void
GFTrackCandCreator::create(
  const std::vector<CDCTrack>& cdcTracks,
  StoreArray<genfit::TrackCand>& gfTrackCands
) const
{

  BOOST_FOREACH(const CDCTrack & cdcTrack, cdcTracks) {

    genfit::TrackCand* gfTrack =  gfTrackCands.appendNew();
    create(cdcTrack, *gfTrack);

  }
}


void
GFTrackCandCreator::create(
  const CDCTrack& track,
  genfit::TrackCand& gfTrackCand
) const
{

  //if there are no segment triples in the track leave the gftarck empty
  if (not track.empty()) {

    Vector3D position;
    Vector3D momentum;

    if (track.getFBInfo() == BACKWARD) {

      position = track.getEndFitPos3D();

      momentum = track.getEndFitMom3D();

    } else {
      //must be set more appropriate with the fit in the track object
      position = track.getStartFitPos3D();

      momentum = track.getStartFitMom3D();

      //int pdg = track.getChargeSign() * (-13);
    }
    //The initial covariance matrix is calculated from these errors and it is important (!!) that it is not completely wrong - took them from Oksana
    TVector3 posError;
    posError.SetXYZ(2.0, 2.0, 2.0);
    TVector3 momError;
    momError.SetXYZ(0.1, 0.1, 0.5);

    //set the start parameters
    SignType q = track.getChargeSign();
    gfTrackCand.setPosMomSeed(position, momentum, q);

    //int pdg = track.getPID();
    //gfTrackCand.setPosMomSeedAndPdgCode(position, momentum, pdg);

    appendHits(track, gfTrackCand);

  }
}



inline void
GFTrackCandCreator::appendHits(
  const CDCTrack& track,
  genfit::TrackCand& gfTrackCand
) const
{

  //the detector id
  const unsigned int iDetector_CDC = Const::CDC;

  if (track.getFBInfo() == BACKWARD) {

    // perpS is relativ to the start
    FloatType endPrepS = track.getEndRecoHit3D().getPerpS();

    //assume backward
    BOOST_REVERSE_FOREACH(CDCRecoHit3D const & recohit , track) {


      //the hit id correspondes to the index in the TClonesArray
      //this is stored in the wirehit the recohit is based on
      unsigned int storeIHit = recohit.getWireHit().getStoreIHit();

      //the plain id serves to mark competition between two or more hits
      //use the wire id here which is unique for all the hits in the track
      //but it may also serve the fact that a track can only be responsable for a single hit on each wire
      //double hits should correspond to multiple tracks
      unsigned int planeId = recohit.getWire().getEWire();


      //the rho parameter is used to sort the cdchits accoring to the direction of flight
      //we assign the estimated flight distance to them
      double rho = endPrepS - recohit.getPerpS(); // translate it to a scale from the end of the track

      gfTrackCand.addHit(iDetector_CDC, storeIHit , planeId, rho);

    }

  } else {
    //assume forward
    BOOST_FOREACH(CDCRecoHit3D const & recohit , track) {


      //the hit id correspondes to the index in the TClonesArray
      //this is stored in the wirehit the recohit is based on
      unsigned int storeIHit = recohit.getWireHit().getStoreIHit();

      //the plain id serves to mark competition between two or more hits
      //use the wire id here which is unique for all the hits in the track
      //but it may also serve the fact that a track can only be responsable for a single hit on each wire
      //double hits should correspond to multiple tracks
      unsigned int planeId = recohit.getWire().getEWire();


      //the rho parameter is used to sort the cdchits accoring to the direction of flight
      //we assign the estimated flight distance to them
      double rho = recohit.getPerpS();

      gfTrackCand.addHit(iDetector_CDC, storeIHit , planeId, rho);
    }

  }
}


